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

#include <typeinfo>
#include <list>
#include <map>
#include <sstream>

#include "gfx_gasoline.h"
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
                error(here) << "Couldn't lex number, junk after decimal point: " << c << ENDL;
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
                error(here) << "Couldn't lex number, junk after E: " << c << ENDL;
            }
            break;

            case AFTER_EXP_SIGN:
            switch (*c) {
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                state = AFTER_EXP_DIGIT;
                break;

                default:
                error(here) << "Couldn't lex number, junk after exponent sign: " << c << ENDL;
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

typedef std::vector<Ast*> Asts;

struct Shader : public Ast {
    Asts stmts;
    Shader (const GfxGslLocation &loc, const Asts &stmts)
      : Ast(loc), stmts(stmts)
    { }
};

struct Block : public Ast {
    Asts stmts;
    Block (const GfxGslLocation &loc, const Asts &stmts)
      : Ast(loc), stmts(stmts)
    { }
};

struct Decl : public Ast {
    bool immutable;
    const std::string id;
    Ast *init;
    Decl (const GfxGslLocation &loc, bool immutable, const std::string id, Ast *init)
      : Ast(loc), immutable(immutable), id(id), init(init)
    { }
};

struct If : public Ast {
    Ast *cond;
    Ast *yes;
    Ast *no;
    If (const GfxGslLocation &loc, Ast *cond, Ast *yes, Ast *no)
      : Ast(loc), cond(cond), yes(yes), no(no)
    { }
};

struct Assign : public Ast {
    Ast *target;
    Ast *expr;
    Assign (const GfxGslLocation &loc, Ast *target, Ast *expr)
      : Ast(loc), target(target), expr(expr)
    { }
};

struct Call : public Ast {
    const std::string func;
    Asts args;
    Call (const GfxGslLocation &loc, const std::string &func, Asts args)
      : Ast(loc), func(func), args(args)
    { }
};

struct Field : public Ast {
    Ast *target;
    const std::string id;
    Field (const GfxGslLocation &loc, Ast *target, const std::string &id)
      : Ast(loc), target(target), id(id)
    { }
};

template<class T> struct Literal : public Ast {
    T val;
    Literal (const GfxGslLocation &loc, T val)
      : Ast(loc), val(val)
    { }
};

typedef Literal<int32_t> LiteralInt;
typedef Literal<float> LiteralFloat;

struct Var : public Ast {
    const std::string id;
    Var (const GfxGslLocation &loc, const std::string &id)
      : Ast(loc), id(id)
    { }
};

enum Op {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_EQ,
    OP_NE,
    OP_LT,
    OP_LTE,
    OP_GT,
    OP_GTE,
    OP_AND,
    OP_OR
};

std::string to_string (Op op)
{
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_MOD: return "%";
        case OP_EQ: return "==";
        case OP_LT: return "<";
        case OP_LTE: return "<=";
        case OP_GT: return ">";
        case OP_GTE: return ">=";
        case OP_AND: return "&&";
        case OP_OR: return "||";
        default: EXCEPTEX << "INTERNAL ERROR: Unknown binary operator: " << op << ENDL;
    }
}

const std::map<std::string, Op> op_map = {
    {"*", OP_MUL},
    {"/", OP_DIV},
    {"%", OP_MOD},

    {"+", OP_ADD},
    {"-", OP_SUB},

    {"<", OP_LT},
    {"<=", OP_LTE},
    {">", OP_GT},
    {">=", OP_GTE},

    {"==", OP_EQ},
    {"!=", OP_NE},

    {"&&", OP_AND},
    {"||", OP_OR},
};

bool is_op(const std::string &symbol, Op &op)
{
    auto it = op_map.find(symbol);
    if (it == op_map.end()) return false;
    op = it->second;
    return true;
}


struct Binary : public Ast {
    Ast *a;
    Op op;
    Ast *b;
    Binary (const GfxGslLocation &loc, Ast *a, Op op, Ast *b)
      : Ast(loc), a(a), op(op), b(b)
    { }
};

enum UnaryOp {
    UOP_NEG,
    UOP_NOT
};

std::string to_string (UnaryOp op)
{
    switch (op) {
        case UOP_NEG: return "-";
        case UOP_NOT: return "!";
        default: EXCEPTEX << "INTERNAL ERROR: Unknown unary operator: " << op << ENDL;
    }
}

const std::map<std::string, UnaryOp> unary_map = {
    {"-", UOP_NEG},
    {"!", UOP_NOT}
};

bool is_uop(const std::string &symbol, UnaryOp &uop)
{
    auto it = unary_map.find(symbol);
    if (it == unary_map.end()) return false;
    uop = it->second;
    return true;
}

struct Unary : public Ast {
    UnaryOp op;
    Ast *expr;
    Unary (const GfxGslLocation &loc, UnaryOp op, Ast *expr)
      : Ast(loc), op(op), expr(expr)
    { }
};


// Keywords

struct Global : public Ast { Global (const GfxGslLocation &loc) : Ast(loc) { } };
struct Mat : public Ast { Mat (const GfxGslLocation &loc) : Ast(loc) { } };
struct Vert : public Ast { Vert (const GfxGslLocation &loc) : Ast(loc) { } };
struct Frag : public Ast { Frag (const GfxGslLocation &loc) : Ast(loc) { } };
struct Discard : public Ast { Discard (const GfxGslLocation &loc) : Ast(loc) { } };
struct Return : public Ast { Return (const GfxGslLocation &loc) : Ast(loc) { } };

const GfxGslLocation NO_LOC;

const int precedence_apply = 1;
const int precedence_uop = 2;
const int precedence_max = 7;

// Can't make this const, as [] is not a const operator.
std::map<Op, int> precedence_op = {
    {OP_MUL, 3},
    {OP_DIV, 3},
    {OP_MOD, 3},

    {OP_ADD, 4},
    {OP_SUB, 4},

    {OP_LT, 5},
    {OP_LTE, 5},
    {OP_GT, 5},
    {OP_GTE, 5},

    {OP_EQ, 6},
    {OP_NE, 6},

    {OP_AND, 7},
    {OP_OR, 7},
};




class Parser {
    Allocator &alloc;
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

    Parser (Allocator &alloc, std::list<Token> tokens)
      : alloc(alloc), tokens(tokens)
    { }

    Ast *parseExpr (int precedence)
    {
        auto tok = peek();
        if (precedence == 0) {
            switch (tok.kind) {
                case MAT: return alloc.makeAst<Mat>(pop().loc);
                case GLOBAL: return alloc.makeAst<Global>(pop().loc);
                case VERT: return alloc.makeAst<Vert>(pop().loc);
                case FRAG: return alloc.makeAst<Frag>(pop().loc);
                case IDENTIFIER: {
                    auto tok = pop();
                    return alloc.makeAst<Var>(tok.loc, tok.val);
                }
                case LITERAL_NUMBER: {
                    auto tok = pop();
                    // If contains only [0-9].
                    bool is_float = false;
                    for (auto c : tok.val) {
                        if (c < '0' || c > '9') is_float = true;
                    }
                    if (is_float)
                        return alloc.makeAst<LiteralFloat>(tok.loc, strtod(tok.val.c_str(), nullptr));
                    else
                        return alloc.makeAst<LiteralInt>(tok.loc, strtol(tok.val.c_str(), nullptr, 10));
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
                UnaryOp op;
                if (is_uop(tok.val, op)) {
                    pop();
                    auto *expr = parseExpr(precedence_uop);
                    return alloc.makeAst<Unary>(tok.loc, op, expr);
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
                    Asts args;
                    bool comma = true;
                    while (true) {
                        if (!comma && maybePopKind(SYMBOL, ","))
                            comma = true;
                        if (maybePopKind(SYMBOL, ")")) break;
                        args.push_back(parseExpr(precedence_max));
                        comma = false;
                    }
                    if (auto *var = dynamic_cast<Var*>(a)) {
                        a = alloc.makeAst<Call>(lparen.loc, var->id, args);
                    } else {
                        error(tok.loc) << "Invalid call syntax: " << lparen << ENDL;
                    }
                } else if (sym == ".") {
                    auto dot = pop();
                    const auto &id = popKind(IDENTIFIER).val;
                    a = alloc.makeAst<Field>(dot.loc, a, id);
                } else {
                    break;  // Process this token at higher precedence.
                }
            } else {
                Op op;
                if (is_op(sym, op) && precedence==precedence_op[op]) {
                    auto op_tok = pop();
                    auto *b = parseExpr(precedence-1);
                    a = alloc.makeAst<Binary>(op_tok.loc, a, op, b);
                } else {
                    break;  // Process this token at higher precedence.
                }
            }
        }

        return a;
    }

    Ast *parseStmt (void)
    {
        auto loc = peek().loc;
        if (maybePopKind(VAL)) {
            const auto &id = popKind(IDENTIFIER).val;
            popKind(SYMBOL, "=");
            auto *init = parseExpr(precedence_max);
            popKind(SYMBOL, ";");
            return alloc.makeAst<Decl>(loc, true, id, init);
        } else if (maybePopKind(VAR)) {
            const auto &id = popKind(IDENTIFIER).val;
            popKind(SYMBOL, "=");
            auto *init = parseExpr(precedence_max);
            popKind(SYMBOL, ";");
            return alloc.makeAst<Decl>(loc, false, id, init);
        } else if (maybePopKind(IF)) {
            popKind(SYMBOL, "(");
            auto *cond = parseExpr(precedence_max);
            popKind(SYMBOL, ")");
            auto *yes = parseStmt();
            if (maybePopKind(ELSE)) {
                auto *no = parseStmt();
                return alloc.makeAst<If>(loc, cond, yes, no);
            }
            return alloc.makeAst<If>(loc, cond, yes, nullptr);
        } else if (maybePopKind(SYMBOL, "{")) {
            Asts stmts;
            while (!maybePopKind(SYMBOL, "}")) {
                stmts.push_back(parseStmt());
            }
            return alloc.makeAst<Block>(loc, stmts);
        } else if (maybePopKind(DISCARD)) {
            popKind(SYMBOL, ";");
            return alloc.makeAst<Discard>(loc);
        } else if (maybePopKind(RETURN)) {
            popKind(SYMBOL, ";");
            return alloc.makeAst<Return>(loc);
        } else {
            auto *lhs = parseExpr(precedence_max);
            popKind(SYMBOL, "=");
            auto *rhs = parseExpr(precedence_max);
            popKind(SYMBOL, ";");
            return alloc.makeAst<Assign>(loc, lhs, rhs);
        }
    }

    Ast *parseShader (void)
    {
        Asts stmts;
        auto loc = peek().loc;
        while (!peekKind(END_OF_FILE)) {
            stmts.push_back(parseStmt());
        }
        return alloc.makeAst<Shader>(loc, stmts);
    }
};

Ast *gfx_gasoline_parse (Allocator &alloc, const std::string &shader)
{
    auto tokens = lex(shader);
    Parser parser(alloc, tokens);
    return parser.parseShader();
}

static void unparse (std::stringstream &ss, const Ast *ast_, int indent)
{
    std::string space(4 * indent, ' ');
    if (auto ast = dynamic_cast<const Block*>(ast_)) {
        ss << space << "{\n";
        for (auto stmt : ast->stmts) {
            unparse(ss, stmt, indent+1);
        }
        ss << space << "}\n";
    } else if (auto ast = dynamic_cast<const Shader*>(ast_)) {
        for (auto stmt : ast->stmts) {
            unparse(ss, stmt, indent);
        }
    } else if (auto ast = dynamic_cast<const Decl*>(ast_)) {
        ss << space << (ast->immutable ? "val" : "var");
        ss << " " << ast->id << " = ";
        unparse(ss, ast->init, indent);
        ss << ";\n";
    } else if (auto ast = dynamic_cast<const If*>(ast_)) {
        ss << space << "if (";
        unparse(ss, ast->cond, indent);
        ss << ") {\n";
        unparse(ss, ast->yes, indent+1);
        if (ast->no) {
            ss << space << "} else {\n";
            unparse(ss, ast->no, indent+1);
        }
        ss << space << "}\n";
    } else if (auto ast = dynamic_cast<const Assign*>(ast_)) {
        ss << space;
        unparse(ss, ast->target, indent);
        ss << " = ";
        unparse(ss, ast->expr, indent);
        ss << ";\n";
    } else if (dynamic_cast<const Discard*>(ast_)) {
        ss << space << "discard;\n";
    } else if (dynamic_cast<const Return*>(ast_)) {
        ss << space << "return;\n";

    } else if (auto ast = dynamic_cast<const Call*>(ast_)) {
        ss << ast->func;
        if (ast->args.size() == 0) {
            ss << "()";
        } else {
            const char *sep = "(";
            for (auto arg : ast->args) {
                ss << sep;
                unparse(ss, arg, indent);
                sep = ", ";
            }
            ss << ")";
        }
    } else if (auto ast = dynamic_cast<const Field*>(ast_)) {
        ss << "(";
        unparse(ss, ast->target, indent);
        ss << ").";
        ss << ast->id;
    } else if (auto ast = dynamic_cast<const LiteralInt*>(ast_)) {
        ss << ast->val;
    } else if (auto ast = dynamic_cast<const LiteralFloat*>(ast_)) {
        ss << ast->val;
    } else if (auto ast = dynamic_cast<const Var*>(ast_)) {
        ss << ast->id;
    } else if (auto ast = dynamic_cast<const Binary*>(ast_)) {
        ss << "(";
        unparse(ss, ast->a, indent);
        ss << " " << to_string(ast->op) << " ";
        unparse(ss, ast->b, indent);
        ss << ")";
    } else if (auto ast = dynamic_cast<const Unary*>(ast_)) {
        ss << "(";
        ss << " " << to_string(ast->op) << " ";
        unparse(ss, ast->expr, indent);
        ss << ")";

    } else if (dynamic_cast<const Global*>(ast_)) {
        ss << "global";
    } else if (dynamic_cast<const Mat*>(ast_)) {
        ss << "mat";
    } else if (dynamic_cast<const Vert*>(ast_)) {
        ss << "vert";
    } else if (dynamic_cast<const Frag*>(ast_)) {
        ss << "frag";

    } else {
        EXCEPTEX << "INTERNAL ERROR: Unknown Ast." << ENDL;
    }
}

std::string gfx_gasoline_unparse (const Ast *ast)
{
    std::stringstream ss;
    unparse(ss, ast, 0);
    return ss.str();
}


// Type system:
//
// float, float2, float3, float4, bool
// (T*) -> T
//
typedef std::vector<Type*> Types;

struct FloatType : public Type {
    unsigned dim;
    FloatType (unsigned dim) : dim(dim) { }
};

struct FloatMatrixType : public Type {
    unsigned w;
    unsigned h;
    FloatMatrixType (unsigned w, unsigned h) : w(w), h(h) { }
};

struct IntType : public Type {
    unsigned dim;
    IntType (unsigned dim) : dim(dim) { }
};

struct BoolType : public Type {
    BoolType (void) { }
};

struct VoidType : public Type {
    VoidType (void) { }
};

struct GlobalType : public Type {
    GlobalType (void) { }
};

struct MatType : public Type {
    MatType (void) { }
};

struct VertType : public Type {
    VertType (void) { }
};

struct FragType : public Type {
    FragType (void) { }
};

struct FunctionType : public Type {
    Types params;
    Type *ret;
    FunctionType (const Types &params, Type *ret) : params(params), ret(ret) { }
};

static inline std::ostream &operator<<(std::ostream &o, Type *t_)
{   
    if (auto *t = dynamic_cast<FloatType*>(t_)) {
        o << "float";
        if (t->dim > 1) o << t->dim;

    } else if (auto *t = dynamic_cast<FloatMatrixType*>(t_)) {
        o << "float" << t->w << "x" << t->h;

    } else if (auto *t = dynamic_cast<IntType*>(t_)) {
        o << "int";
        if (t->dim > 1) o << t->dim;

    } else if (dynamic_cast<BoolType*>(t_)) {
        o << "bool";

    } else if (dynamic_cast<VoidType*>(t_)) {
        o << "void";

    } else if (dynamic_cast<GlobalType*>(t_)) {
        o << "Global";

    } else if (dynamic_cast<MatType*>(t_)) {
        o << "Mat";

    } else if (dynamic_cast<VertType*>(t_)) {
        o << "Vert";

    } else if (dynamic_cast<FragType*>(t_)) {
        o << "Frag";

    } else if (auto *t = dynamic_cast<FunctionType*>(t_)) {
        if (t->params.size() > 0) {
            o << "( )";
        } else {
            const char *prefix = "(";
            for (unsigned i=0 ; i<t->params.size() ; ++i) {
                o << prefix << t->params[i];
                prefix = ", ";
            }
            o << ")";
        }

        o << " -> ";
        o << t->ret;
    }

    return o;
}

class TypeSystem {
    Allocator &alloc;

    std::map<std::string, Type*> symbols;

    std::map<std::string, std::vector<FunctionType*>> funcTypes;

    void initFuncTypes (void)
    {
        Type *fs[] = {
            nullptr,  // Easier to read code below if array starts from 1
            alloc.makeType<FloatType>(1),
            alloc.makeType<FloatType>(2),
            alloc.makeType<FloatType>(3),
            alloc.makeType<FloatType>(4)
        };
        //Type *b = alloc.makeType<BoolType>();
        // ts holds the set of all functions: float_n -> float_n
        std::vector<FunctionType*> ts;
        for (unsigned i=1 ; i<=4 ; ++i)
            ts.push_back(alloc.makeType<FunctionType>(Types{fs[i]}, fs[i]));
        funcTypes["atan"] = ts;
        funcTypes["ddx"] = ts;
        funcTypes["ddy"] = ts;
        funcTypes["pow"] = ts;

        funcTypes["dot"] = { alloc.makeType<FunctionType>(Types{fs[3], fs[3]}, fs[1]) };
        funcTypes["normalize"] = { alloc.makeType<FunctionType>(Types{fs[3]}, fs[3]) };

        // ts holds the set of all functions: (float_n, float_n, float_n) -> float_n
        ts.clear();
        for (unsigned i=1 ; i<=4 ; ++i)
            ts.push_back(alloc.makeType<FunctionType>(Types{fs[i], fs[i], fs[i]}, fs[i]));
        funcTypes["clamp"] = ts;

        funcTypes["float"] = { alloc.makeType<FunctionType>(Types{fs[1]}, fs[1]) };
        funcTypes["float2"] = {
            alloc.makeType<FunctionType>(Types{fs[2]}, fs[2]),
            alloc.makeType<FunctionType>(Types{fs[1], fs[1]}, fs[2])
        };
        funcTypes["float3"] = {
            alloc.makeType<FunctionType>(Types{fs[3]}, fs[3]),
            alloc.makeType<FunctionType>(Types{fs[2], fs[1]}, fs[3]),
            alloc.makeType<FunctionType>(Types{fs[1], fs[2]}, fs[3])
        };
        funcTypes["float4"] = {
            alloc.makeType<FunctionType>(Types{fs[4]}, fs[4]),

            alloc.makeType<FunctionType>(Types{fs[3], fs[1]}, fs[4]),
            alloc.makeType<FunctionType>(Types{fs[1], fs[3]}, fs[4]),
            alloc.makeType<FunctionType>(Types{fs[2], fs[2]}, fs[4]),

            alloc.makeType<FunctionType>(Types{fs[1], fs[1], fs[2]}, fs[4]),
            alloc.makeType<FunctionType>(Types{fs[1], fs[2], fs[1]}, fs[4]),
            alloc.makeType<FunctionType>(Types{fs[2], fs[1], fs[1]}, fs[4]),

            alloc.makeType<FunctionType>(Types{fs[1], fs[1], fs[1], fs[1]}, fs[4])
        };

        // ts holds the set of all functions: (float_n, float_n, float1) -> float_n
        ts.clear();
        for (unsigned i=1 ; i<=4 ; ++i)
            ts.push_back(alloc.makeType<FunctionType>(Types{fs[i], fs[i], fs[1]}, fs[i]));
        funcTypes["lerp"] = ts;

            
        // ts holds the set of all functions: (float_n, float_n) -> float_n
        ts.clear();
        for (unsigned i=1 ; i<=4 ; ++i)
            ts.push_back(alloc.makeType<FunctionType>(Types{fs[i], fs[i]}, fs[i]));
        funcTypes["max"] = ts;
        funcTypes["min"] = ts;

        // ts holds the set of all functions: (float_n, float_1) -> float_n
        ts.clear();
        for (unsigned i=1 ; i<=4 ; ++i)
            ts.push_back(alloc.makeType<FunctionType>(Types{fs[i], fs[1]}, fs[i]));
        funcTypes["mod"] = ts;

        ts.clear();
        for (unsigned w=1 ; w<=4 ; ++w) {
            for (unsigned h=1 ; h<=4 ; ++h) {
                Type *m = alloc.makeType<FloatMatrixType>(w, h);
                ts.push_back(alloc.makeType<FunctionType>(Types{m, fs[h]}, fs[h]));
            }
        }
        funcTypes["mul"] = ts;
    }

    FunctionType *lookupFunction(const GfxGslLocation &loc, const std::string &name,
                                 const std::vector<Ast*> &asts)
    {
        auto it = funcTypes.find(name);
        if (it == funcTypes.end()) {
            error(loc) << "Unrecognised function: " << name << ENDL;
        }
        const std::vector<FunctionType*> &overrides = it->second;
        for (auto *o : overrides) {
            if (o->params.size() != asts.size()) continue;
            for (unsigned i=0 ; i<asts.size() ; ++i) {
                if (!equal(o->params[i], asts[i]->type)) goto nextfunc;
            }
            return o;
            nextfunc:;
        }

        // TODO: search again using implicit conversions

        std::stringstream ss;
        if (asts.size() == 0) {
            ss << "()";
        } else {
            const char *prefix = "(";
            for (unsigned i=0 ; i<asts.size() ; ++i) {
                ss << prefix << asts[i]->type;
                prefix = ", ";
            }
            ss << ")";
        }
        error(loc) << "No override found for " << name << ss.str() << ENDL;
    }

    bool isVoid (Type *x)
    {
        return dynamic_cast<Type*>(x) != nullptr;
    }

    bool isFirstClass (Type *x)
    {
        if (dynamic_cast<FloatType*>(x)) {
            return true;
        } else if (dynamic_cast<IntType*>(x)) {
            return true;
        } else if (dynamic_cast<BoolType*>(x)) {
            return true;
        }
        return false;
    }

    bool equal (Type *a_, Type *b_)
    {
        if (auto *a = dynamic_cast<FloatType*>(a_)) {
            auto *b = dynamic_cast<FloatType*>(b_);
            if (b == nullptr) return false;
            if (b->dim != a->dim) return false;

        } else if (auto *a = dynamic_cast<IntType*>(a_)) {
            auto *b = dynamic_cast<IntType*>(b_);
            if (b == nullptr) return false;
            if (b->dim != a->dim) return false;

        } else if (dynamic_cast<BoolType*>(a_)) {
            auto *b = dynamic_cast<BoolType*>(b_);
            if (b == nullptr) return false;

        } else if (dynamic_cast<VoidType*>(a_)) {
            auto *b = dynamic_cast<VoidType*>(b_);
            if (b == nullptr) return false;

        } else if (dynamic_cast<GlobalType*>(a_)) {
            auto *b = dynamic_cast<GlobalType*>(b_);
            if (b == nullptr) return false;

        } else if (dynamic_cast<MatType*>(a_)) {
            auto *b = dynamic_cast<MatType*>(b_);
            if (b == nullptr) return false;

        } else if (dynamic_cast<VertType*>(a_)) {
            auto *b = dynamic_cast<VertType*>(b_);
            if (b == nullptr) return false;

        } else if (dynamic_cast<FragType*>(a_)) {
            auto *b = dynamic_cast<FragType*>(b_);
            if (b == nullptr) return false;

        } else if (auto *a = dynamic_cast<FunctionType*>(a_)) {
            auto *b = dynamic_cast<FunctionType*>(b_);
            if (b == nullptr) return false;
            if (!equal(a->ret, b->ret)) return false;
            if (a->params.size() != b->params.size()) return false;
            for (unsigned i=0 ; i<a->params.size() ; ++i) {
                if (!equal(a->params[i], b->params[i])) return false;
            }
        }
        return true;
    }

    // Wrap from to do conversion
    bool conversionExists (Ast *&from, Type *to)
    {
        // TODO(dcunnin): Support conversions.
        if (equal(from->type, to)) return true;
        return false;
    }

    void unify (const GfxGslLocation &loc, Ast *&a, Ast *&b)
    {
        ASSERT(a != nullptr);
        ASSERT(b != nullptr);
        ASSERT(a->type != nullptr);
        ASSERT(b->type != nullptr);
        if ((conversionExists(a, b->type))) {
            return;
        }
        if ((conversionExists(b, a->type))) {
            return;
        }
        error(loc) << "Could not unify types " << a->type << " and " << b->type << ENDL;
    }

    public:

    TypeSystem (Allocator &alloc)
      : alloc(alloc)
    {
        initFuncTypes();
    }

    void inferAndSet (Ast *ast_)
    {
        ASSERT(ast_ != nullptr);
        const GfxGslLocation &loc = ast_->loc;

        if (auto *ast = dynamic_cast<Block*>(ast_)) {
            for (auto stmt : ast->stmts) {
                inferAndSet(stmt);
            }
            ast->type = alloc.makeType<VoidType>();

        } else if (auto *ast = dynamic_cast<Shader*>(ast_)) {
            for (auto stmt : ast->stmts) {
                inferAndSet(stmt);
            }
            ast->type = alloc.makeType<VoidType>();

        } else if (auto *ast = dynamic_cast<Decl*>(ast_)) {
            inferAndSet(ast->init);
            if (!isFirstClass(ast->init->type)) {
                error(loc) << "Type is not first class: " << ast->init->type << ENDL;
            }
            if (!ast->immutable) ast->init->type->writeable = true;
            symbols[ast->id] = ast->init->type;
            ast->type = alloc.makeType<VoidType>();

        } else if (auto *ast = dynamic_cast<If*>(ast_)) {
            inferAndSet(ast->cond);
            inferAndSet(ast->yes);
            if (ast->no) {
                inferAndSet(ast->no);
                unify(loc, ast->yes, ast->no);
            }
            ast->type = alloc.makeType<VoidType>();

        } else if (auto *ast = dynamic_cast<Assign*>(ast_)) {
            inferAndSet(ast->target);
            if (!ast->target->type->writeable) {
                error(loc) << "Cannot assign to this object." << ENDL;
            }
            inferAndSet(ast->expr);
            
            ast->type = alloc.makeType<VoidType>();

        } else if (auto *ast = dynamic_cast<Discard*>(ast_)) {
            ast->type = alloc.makeType<VoidType>();

        } else if (auto *ast = dynamic_cast<Return*>(ast_)) {
            ast->type = alloc.makeType<VoidType>();

        } else if (auto *ast = dynamic_cast<Call*>(ast_)) {
            // TODO(dcunnin): Implement this.
            for (unsigned i=0 ; i<ast->args.size() ; ++i)
                inferAndSet(ast->args[i]);
            FunctionType *t = lookupFunction(loc, ast->func, ast->args);
            ast->type = t->ret;

        } else if (auto *ast = dynamic_cast<Field*>(ast_)) {
            // TODO(dcunnin): Implement this.
            //ast->target
            //ast->id
            std::cout << "field: " << ast->id << std::endl;;
            ast->type = alloc.makeType<FloatType>(1);
            ast->type->writeable = true;

        } else if (auto *ast = dynamic_cast<LiteralInt*>(ast_)) {
            ast->type = alloc.makeType<IntType>(1);

        } else if (auto *ast = dynamic_cast<LiteralFloat*>(ast_)) {
            ast->type = alloc.makeType<FloatType>(1);

        } else if (auto *ast = dynamic_cast<Var*>(ast_)) {
            auto it = symbols.find(ast->id);
            if (it == symbols.end()) {
                error(loc) << "Unknown variable: " << ast->id << ENDL;
            }
            ast->type = symbols[ast->id];

        } else if (auto *ast = dynamic_cast<Binary*>(ast_)) {
            // TODO(dcunnin): Implement this.
            // Varies depending on the op, e.g. + vs <=
            //ast->op
            //ast->a
            //ast->b
            ast->type = alloc.makeType<FloatType>(1);

        } else if (auto *ast = dynamic_cast<Unary*>(ast_)) {
            // TODO(dcunnin): Implement this.
            // Varies depending on the op, e.g. ! vs -
            //ast->op
            //ast->expr
            ast->type = alloc.makeType<FloatType>(1);

        } else if (auto *ast = dynamic_cast<Global*>(ast_)) {
            ast->type = alloc.makeType<GlobalType>();

        } else if (auto *ast = dynamic_cast<Mat*>(ast_)) {
            ast->type = alloc.makeType<MatType>();

        } else if (auto *ast = dynamic_cast<Vert*>(ast_)) {
            ast->type = alloc.makeType<VertType>();

        } else if (auto *ast = dynamic_cast<Frag*>(ast_)) {
            ast->type = alloc.makeType<FragType>();

        } else {
            EXCEPTEX << "INTERNAL ERROR: Unknown Ast." << ENDL;

        }
    }
};

void gfx_gasoline_type (Allocator &alloc, Ast *ast)
{
    TypeSystem ts(alloc);
    ts.inferAndSet(ast);
}
// vim: shiftwidth=4:tabstop=4:expandtab
