/* Copyright (c) David Cunningham and the Grit Game Engine project 2013
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <cstdlib>
#include <cfloat>

#include <list>
#include <map>
#include <sstream>

#include "gfx_gasoline_parser.h"
#include "centralised_log.h"
#include <exception.h>

// Don't use soft keywords.  They will make it harder to read code.  Use proper keyword tokens.  It
// makes it easier to disallow keywords being identifiers.

enum TokenKind {
    IDENTIFIER,  // Has val.

    DISCARD,
    ELSE,
    FRAG,
    GLOBAL,
    IF,
    MAT,
    RETURN,
    VAL,
    VAR,
    VERT,

    SYMBOL,  // Has val.
    LITERAL_NUMBER,  // Has val.
    END_OF_FILE  // Holds location info if EOF error.
};

static std::string to_string (TokenKind k)
{
    switch (k) {
        case IDENTIFIER: return "identifier";
        case DISCARD: return "discard";
        case ELSE: return "else";
        case FRAG: return "frag";
        case GLOBAL: return "global";
        case IF: return "if";
        case MAT: return "mat";
        case RETURN: return "return";
        case VAL: return "val";
        case VAR: return "var";
        case VERT: return "vert";
        case SYMBOL: return "symbol";
        case LITERAL_NUMBER: return "number";
        case END_OF_FILE: return "EOF";
        default: EXCEPTEX << "Internal error: Unknown token kind: " << k << ENDL;
    }
}

struct Token {
    TokenKind kind;
    std::string val;
    GfxGslLocation loc;
    Token (TokenKind kind, const GfxGslLocation &loc)
      : kind(kind), val(to_string(kind)), loc(loc)
    { }
    Token (TokenKind kind, const std::string &val, const GfxGslLocation &loc)
      : kind(kind), val(val), loc(loc)
    { }
};

static inline bool operator==(const Token &a, const Token &b)
{
    return a.val == b.val;
}

static inline std::ostream &operator<<(std::ostream &o, const Token &v)
{
    o << v.val;
    return o;
}

// Workaround for g++ not supporting moveable streams.
#define error(loc) (EXCEPT << "Shader error: " << loc << ": ")

// Symbol tokens are arbitrary-length uninterrupted sequences of these characters
bool is_symbol (char c)
{
    switch (c) {
        case '=': case '!': case '+': case '-': case '*': case '/': case '%': case '^':
        case '<': case '>':
        case '&': case '|':
        return true;
    }
    return false;
}

static bool is_upper (char c) { return c >= 'A' && c <= 'Z'; }
static bool is_lower (char c) { return c >= 'a' && c <= 'z'; }
static bool is_number (char c) { return c >= '0' && c <= '9'; }
static bool is_identifier1 (char c) { return c == '_' || is_upper(c) || is_lower(c); }
static bool is_identifier (char c) { return is_identifier1(c) || is_number(c); }

static Token lex_number (const char *&c, const GfxGslLocation &here)
{
    enum State {
        BEGIN,
        AFTER_ZERO,
        AFTER_ONE_TO_NINE,
        AFTER_DOT,
        AFTER_DIGIT,
        AFTER_EXP,
        AFTER_EXP_SIGN,
        AFTER_EXP_DIGIT
    } state;

    std::stringstream ss;

    state = BEGIN;
    while (true) {
        switch (state) {
            case BEGIN:
            switch (*c) {
                case '0':
                state = AFTER_ZERO;
                break;

                case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                state = AFTER_ONE_TO_NINE;
                break;

                default:
                error(here) << "Couldn't lex number" << ENDL;
            }
            break;

            case AFTER_ZERO:
            switch (*c) {
                case '.':
                state = AFTER_DOT;
                break;

                case 'E': case 'e':
                state = AFTER_EXP;
                break;

                default:
                goto end;
            }
            break;

            case AFTER_ONE_TO_NINE:
            switch (*c) {
                case '.':
                state = AFTER_DOT;
                break;

                case 'E': case 'e': 
                state = AFTER_EXP;
                break;

                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                state = AFTER_ONE_TO_NINE;
                break;

                default:
                goto end;
            }
            break;

            case AFTER_DOT:
            switch (*c) {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                state = AFTER_DIGIT;
                break;

                default:
                error(here) << "Couldn't lex number, junk after decimal point: " << *c << ENDL;
            }
            break;

            case AFTER_DIGIT:
            switch (*c) {
                case 'E': case 'e': 
                state = AFTER_EXP;
                break;

                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                state = AFTER_DIGIT;
                break;

                default:
                goto end;
            }
            break;

            case AFTER_EXP:
            switch (*c) {
                case '+': case '-':
                state = AFTER_EXP_SIGN;
                break;

                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                state = AFTER_EXP_DIGIT;
                break;

                default:
                error(here) << "Couldn't lex number, junk after E: " << *c << ENDL;
            }
            break;

            case AFTER_EXP_SIGN:
            switch (*c) {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                state = AFTER_EXP_DIGIT;
                break;

                default:
                error(here) << "Couldn't lex number, junk after exponent sign: " << *c << ENDL;
            }
            break;

            case AFTER_EXP_DIGIT:
            switch (*c) {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                state = AFTER_EXP_DIGIT;
                break;

                default:
                goto end;
            }
            break;
        }
        ss << *c;
        c++;
    }
    end:
    c--;
    return Token(LITERAL_NUMBER, ss.str(), here);
}

std::list<Token> lex (const std::string &shader)
{
    unsigned long this_line_number = 1;
    const char *this_line = &shader[0];
    std::list<Token> r;
    const char *c = shader.c_str();
    for ( ; *c != '\0' ; ++c) {
        // Columns start from 1, hence the + 1.
        GfxGslLocation here(this_line_number, c - this_line + 1);
        switch (*c) {
            // White space
            case '\n':
            this_line_number++;
            this_line = c + 1;
            case ' ': case '\t':
            break;

            // Symbols that cannot be combined with a =
            case '{': case '}': case '(': case ')': case '.': case ',': case ';':
            r.emplace_back(SYMBOL, std::string({*c}), here);
            break;

            default:
            if (is_symbol(*c)) {
                /* Old-style C comments */
                if (*c == '/' && *(c+1) == '*') {
                    c += 2;
                    while (*c != '\0' && !(*c == '*' && *(c+1) == '/')) {
                        if (*c == '\n') {
                            // Maintain line and column vars.
                            this_line_number++;
                            this_line = c+1;
                        }
                        ++c;
                    }
                    if (*c == '\0') {
                        error(here) << "Multi-line comment has no terminating */.";
                    }
                    c++; // Set counter to the end /.
                    break;
                }

                // Single-line C++ style comments
                if (*c == '/' && *(c+1) == '/') {
                    while (*c != '\0' && *c != '\n') ++c;
                    this_line_number++;
                    this_line = c + 1;
                    break;
                }

                // Accumulate all symbol chars.
                std::stringstream ss;
                while (is_symbol(*c)) {
                    ss << *c;
                    ++c;
                }
                c--;  // Leave it on the last symbol.
                r.emplace_back(SYMBOL, ss.str(), here);

            } else if (is_identifier1(*c)) {
                std::string id;
                for (; is_identifier(*c) ; ++c) {
                    id += *c;
                }
                --c;
                if (id == "discard") {
                    r.emplace_back(DISCARD, "discard", here);
                } else if (id == "else") {
                    r.emplace_back(ELSE, "else", here);
                } else if (id == "frag") {
                    r.emplace_back(FRAG, "frag", here);
                } else if (id == "global") {
                    r.emplace_back(GLOBAL, "global", here);
                } else if (id == "if") {
                    r.emplace_back(IF, "if", here);
                } else if (id == "mat") {
                    r.emplace_back(MAT, "mat", here);
                } else if (id == "return") {
                    r.emplace_back(RETURN, "return", here);
                } else if (id == "val") {
                    r.emplace_back(VAL, "val", here);
                } else if (id == "var") {
                    r.emplace_back(VAR, "var", here);
                } else if (id == "vert") {
                    r.emplace_back(VERT, "vert", here);
                } else {
                    r.emplace_back(IDENTIFIER, id, here);
                }
            } else if (is_number(*c)) {
                r.push_back(lex_number(c, here));
            } else {
                error(here) << "Did not understand character \"" << std::string({*c}) << "\""
                            << ENDL;
            }
        }
    }
    r.emplace_back(END_OF_FILE, "EOF", GfxGslLocation(this_line_number, c - this_line + 1));
    //for (auto t : r) std::cout << t << std::endl;
    return r;
}


/////////////
// PARSING //
/////////////

static const int precedence_apply = 1;
static const int precedence_uop = 2;
static const int precedence_max = 7;

// Can't make this const, as [] is not a const operator.
static std::map<GfxGslOp, int> precedence_op = {
    {GFX_GSL_OP_MUL, 3},
    {GFX_GSL_OP_DIV, 3},
    {GFX_GSL_OP_MOD, 3},

    {GFX_GSL_OP_ADD, 4},
    {GFX_GSL_OP_SUB, 4},

    {GFX_GSL_OP_LT, 5},
    {GFX_GSL_OP_LTE, 5},
    {GFX_GSL_OP_GT, 5},
    {GFX_GSL_OP_GTE, 5},

    {GFX_GSL_OP_EQ, 6},
    {GFX_GSL_OP_NE, 6},

    {GFX_GSL_OP_AND, 7},
    {GFX_GSL_OP_OR, 7},
};

static inline bool is_op(const std::string &symbol, GfxGslOp &op)
{
    auto it = op_map.find(symbol);
    if (it == op_map.end()) return false;
    op = it->second;
    return true;
}



namespace {
    class Parser {
        GfxGslAllocator &alloc;
        std::list<Token> tokens;

        Token peek (void) { return tokens.front(); }

        Token pop (void)
        {
            auto tok = peek();
            tokens.pop_front();
            return tok;
        }

        bool peekKind (TokenKind k, const char *val=nullptr)
        {
            auto tok = peek();
            if (tok.kind != k) return false;
            if (val != nullptr && tok.val != val) return false;
            return true;
        }

        bool maybePopKind (TokenKind k, const char *val=nullptr)
        {
            auto r = peekKind(k, val);
            if (r) pop();
            return r;
        }

        Token popKind (TokenKind k, const char *val=nullptr)
        {
            auto tok = pop();
            if (tok.kind != k || (val!=nullptr && tok.val != val)) {
                if (val == nullptr) {
                    error(tok.loc) << "Expected " << to_string(k) << ", got " << tok.val << ENDL;
                } else {
                    error(tok.loc) << "Expected " << val << ", got " << tok.val << ENDL;
                }
            }
            return tok;
        }

        public:

        Parser (GfxGslAllocator &alloc, std::list<Token> tokens)
          : alloc(alloc), tokens(tokens)
        { }

        GfxGslAst *parseExpr (int precedence)
        {
            auto tok = peek();
            if (precedence == 0) {
                switch (tok.kind) {
                    case MAT: return alloc.makeAst<GfxGslMat>(pop().loc);
                    case GLOBAL: return alloc.makeAst<GfxGslGlobal>(pop().loc);
                    case VERT: return alloc.makeAst<GfxGslVert>(pop().loc);
                    case FRAG: return alloc.makeAst<GfxGslFrag>(pop().loc);
                    case IDENTIFIER: {
                        auto tok = pop();
                        return alloc.makeAst<GfxGslVar>(tok.loc, tok.val);
                    }
                    case LITERAL_NUMBER: {
                        auto tok = pop();
                        // If contains only [0-9].
                        bool is_float = false;
                        for (auto c : tok.val) {
                            if (c < '0' || c > '9') is_float = true;
                        }
                        if (is_float)
                            return alloc.makeAst<GfxGslLiteralFloat>(tok.loc, strtod(tok.val.c_str(), nullptr));
                        else
                            return alloc.makeAst<GfxGslLiteralInt>(tok.loc, strtol(tok.val.c_str(), nullptr, 10));
                    }
                    case SYMBOL:
                    if (tok.val == "(") {
                        pop();
                        auto *expr = parseExpr(precedence_max);
                        popKind(SYMBOL, ")");
                        return expr;
                    }
                    // else follow into error
                    default: 
                    error(tok.loc) << "Unexpected: " << tok << ENDL;
                }
            } else if (precedence == precedence_uop) {
                if (tok.kind == SYMBOL) {
                    if (tok.val == "!") {
                        pop();
                        auto *expr = parseExpr(precedence_uop);
                        auto *_false = alloc.makeAst<GfxGslLiteralBoolean>(tok.loc, false);
                        return alloc.makeAst<GfxGslBinary>(tok.loc, expr, GFX_GSL_OP_EQ, _false);
                    } else if (tok.val == "-") {
                        pop();
                        auto *expr = parseExpr(precedence_uop);
                        auto *zero = alloc.makeAst<GfxGslLiteralInt>(tok.loc, 0);
                        return alloc.makeAst<GfxGslBinary>(tok.loc, zero, GFX_GSL_OP_SUB, expr);
                    }
                }
            }

            // Must be binary
            auto *a = parseExpr(precedence - 1);
            
            while (true) {

                if (peek().kind == END_OF_FILE) return a;

                if (peek().kind != SYMBOL) {
                    error(tok.loc) << "Not a symbol: " << peek() << ENDL;
                }
                auto sym = peek().val;

                // special cases for things that arent binary operators
                if (precedence == precedence_apply) {
                    if (sym == "(") {
                        Token lparen = pop();
                        GfxGslAsts args;
                        bool comma = true;
                        while (true) {
                            if (!comma && maybePopKind(SYMBOL, ","))
                                comma = true;
                            if (maybePopKind(SYMBOL, ")")) break;
                            args.push_back(parseExpr(precedence_max));
                            comma = false;
                        }
                        if (auto *var = dynamic_cast<GfxGslVar*>(a)) {
                            a = alloc.makeAst<GfxGslCall>(lparen.loc, var->id, args);
                        } else {
                            error(tok.loc) << "Invalid call syntax: " << lparen << ENDL;
                        }
                    } else if (sym == ".") {
                        auto dot = pop();
                        const auto &id = popKind(IDENTIFIER).val;
                        a = alloc.makeAst<GfxGslField>(dot.loc, a, id);
                    } else {
                        break;  // Process this token at higher precedence.
                    }
                } else {
                    GfxGslOp op;
                    if (is_op(sym, op) && precedence==precedence_op[op]) {
                        auto op_tok = pop();
                        auto *b = parseExpr(precedence-1);
                        a = alloc.makeAst<GfxGslBinary>(op_tok.loc, a, op, b);
                    } else {
                        break;  // Process this token at higher precedence.
                    }
                }
            }

            return a;
        }

        GfxGslAst *parseStmt (void)
        {
            auto loc = peek().loc;
            if (maybePopKind(VAL)) {
                const auto &id = popKind(IDENTIFIER).val;
                popKind(SYMBOL, "=");
                auto *init = parseExpr(precedence_max);
                popKind(SYMBOL, ";");
                return alloc.makeAst<GfxGslDecl>(loc, true, id, init);
            } else if (maybePopKind(VAR)) {
                const auto &id = popKind(IDENTIFIER).val;
                popKind(SYMBOL, "=");
                auto *init = parseExpr(precedence_max);
                popKind(SYMBOL, ";");
                return alloc.makeAst<GfxGslDecl>(loc, false, id, init);
            } else if (maybePopKind(IF)) {
                popKind(SYMBOL, "(");
                auto *cond = parseExpr(precedence_max);
                popKind(SYMBOL, ")");
                auto *yes = parseStmt();
                if (maybePopKind(ELSE)) {
                    auto *no = parseStmt();
                    return alloc.makeAst<GfxGslIf>(loc, cond, yes, no);
                }
                return alloc.makeAst<GfxGslIf>(loc, cond, yes, nullptr);
            } else if (maybePopKind(SYMBOL, "{")) {
                GfxGslAsts stmts;
                while (!maybePopKind(SYMBOL, "}")) {
                    stmts.push_back(parseStmt());
                }
                return alloc.makeAst<GfxGslBlock>(loc, stmts);
            } else if (maybePopKind(DISCARD)) {
                popKind(SYMBOL, ";");
                return alloc.makeAst<GfxGslDiscard>(loc);
            } else if (maybePopKind(RETURN)) {
                popKind(SYMBOL, ";");
                return alloc.makeAst<GfxGslReturn>(loc);
            } else {
                auto *lhs = parseExpr(precedence_max);
                popKind(SYMBOL, "=");
                auto *rhs = parseExpr(precedence_max);
                popKind(SYMBOL, ";");
                return alloc.makeAst<GfxGslAssign>(loc, lhs, rhs);
            }
        }

        GfxGslAst *parseShader (void)
        {
            GfxGslAsts stmts;
            auto loc = peek().loc;
            while (!peekKind(END_OF_FILE)) {
                stmts.push_back(parseStmt());
            }
            return alloc.makeAst<GfxGslShader>(loc, stmts);
        }
    };
}

GfxGslAst *gfx_gasoline_parse (GfxGslAllocator &alloc, const std::string &shader)
{
    auto tokens = lex(shader);
    Parser parser(alloc, tokens);
    return parser.parseShader();
}

// vim: shiftwidth=4:tabstop=4:expandtab
