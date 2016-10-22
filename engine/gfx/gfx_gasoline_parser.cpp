/* Copyright (c) The Grit Game Engine authors 2016
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

#include <list>
#include <map>
#include <sstream>

#include <exception.h>

#include <centralised_log.h>

#include "gfx_gasoline.h"
#include "gfx_gasoline_parser.h"
#include "gfx_gasoline_type_system.h"

// Thankyou, Windows...
#ifdef OUT
#undef OUT
#endif

// Workaround for g++ not supporting moveable streams.
#define error(loc) (EXCEPT << "Parse error: " << loc << ": ")

enum TokenKind {
    BODY,
    DISCARD,
    ELSE,
    END_OF_FILE,  // Holds location info if EOF error.
    FOR,
    FRAG,
    GLOBAL,
    IDENTIFIER,  // Has val.
    IF,
    LITERAL_NUMBER,  // Has val.
    MAT,
    OUT,
    RETURN,
    SYMBOL,  // Has val.
    VAR,
    VERT,

    // Type tokens are distinguished by having capital letters.
    TYPE_FLOAT,
    TYPE_FLOAT2,
    TYPE_FLOAT3,
    TYPE_FLOAT4,
    TYPE_FLOAT1x1,
    TYPE_FLOAT2x1,
    TYPE_FLOAT3x1,
    TYPE_FLOAT4x1,
    TYPE_FLOAT1x2,
    TYPE_FLOAT2x2,
    TYPE_FLOAT3x2,
    TYPE_FLOAT4x2,
    TYPE_FLOAT1x3,
    TYPE_FLOAT2x3,
    TYPE_FLOAT3x3,
    TYPE_FLOAT4x3,
    TYPE_FLOAT1x4,
    TYPE_FLOAT2x4,
    TYPE_FLOAT3x4,
    TYPE_FLOAT4x4,
    TYPE_FLOAT_TEXTURE1,
    TYPE_FLOAT_TEXTURE2,
    TYPE_FLOAT_TEXTURE3,
    TYPE_FLOAT_TEXTURE4,
    TYPE_FLOAT_TEXTURE_CUBE,
    TYPE_INT,
    TYPE_INT2,
    TYPE_INT3,
    TYPE_INT4,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_GLOBAL,
    TYPE_MAT,
    TYPE_VERT,
    TYPE_OUT,
    TYPE_BODY,
    TYPE_FRAG,
};

static std::string to_string (TokenKind k)
{
    switch (k) {
        case BODY: return "body";
        case DISCARD: return "discard";
        case ELSE: return "else";
        case END_OF_FILE: return "EOF";
        case FOR: return "for";
        case FRAG: return "frag";
        case GLOBAL: return "global";
        case IDENTIFIER: return "identifier";
        case IF: return "if";
        case LITERAL_NUMBER: return "number";
        case MAT: return "mat";
        case OUT: return "out";
        case RETURN: return "return";
        case SYMBOL: return "symbol";
        case VAR: return "var";
        case VERT: return "vert";

        case TYPE_FLOAT: return "Float";
        case TYPE_FLOAT2: return "Float2";
        case TYPE_FLOAT3: return "Float3";
        case TYPE_FLOAT4: return "Float4";
        case TYPE_FLOAT1x1: return "Float1x1";
        case TYPE_FLOAT2x1: return "Float2x1";
        case TYPE_FLOAT3x1: return "Float3x1";
        case TYPE_FLOAT4x1: return "Float4x1";
        case TYPE_FLOAT1x2: return "Float1x2";
        case TYPE_FLOAT2x2: return "Float2x2";
        case TYPE_FLOAT3x2: return "Float3x2";
        case TYPE_FLOAT4x2: return "Float4x2";
        case TYPE_FLOAT1x3: return "Float1x3";
        case TYPE_FLOAT2x3: return "Float2x3";
        case TYPE_FLOAT3x3: return "Float3x3";
        case TYPE_FLOAT4x3: return "Float4x3";
        case TYPE_FLOAT1x4: return "Float1x4";
        case TYPE_FLOAT2x4: return "Float2x4";
        case TYPE_FLOAT3x4: return "Float3x4";
        case TYPE_FLOAT4x4: return "Float4x4";
        case TYPE_FLOAT_TEXTURE1: return "FloatTexture1";
        case TYPE_FLOAT_TEXTURE2: return "FloatTexture2";
        case TYPE_FLOAT_TEXTURE3: return "FloatTexture3";
        case TYPE_FLOAT_TEXTURE4: return "FloatTexture4";
        case TYPE_FLOAT_TEXTURE_CUBE: return "FloatTextureCube";
        case TYPE_INT: return "Int";
        case TYPE_INT2: return "Int2";
        case TYPE_INT3: return "Int3";
        case TYPE_INT4: return "Int4";
        case TYPE_BOOL: return "Bool";
        case TYPE_VOID: return "Void";
        case TYPE_GLOBAL: return "Global";
        case TYPE_MAT: return "Mat";
        case TYPE_VERT: return "Vert";
        case TYPE_OUT: return "Out";
        case TYPE_BODY: return "Body";
        case TYPE_FRAG: return "Frag";
    }
    EXCEPT << "Internal error." << ENDL;
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
            case ' ': case '\t': case '\r':
            break;

            // Symbols that cannot be combined with a =
            case '{': case '}': case '(': case ')': case '.': case ',': case ';':
            case '[': case ']': case ':':
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

                // Lex operator -- sequence of symbols chars that ends at the first /* // and
                // cannot end with a - ! ~ or + (unless the operator is that single char).
                const char *operator_begin = c;
                std::stringstream ss;
                while (is_symbol(*c)) {
                    // Not allowed // in operators
                    if (*c == '/' && *(c+1) == '/') break;
                    // Not allowed /* in operators
                    if (*c == '/' && *(c+1) == '*') break;
                    ++c;
                }
                while (c > operator_begin + 1) {
                    switch (*(c - 1)) {
                        case '+': case '-': case '~': case '!':
                        c--;
                        break;
                        default:
                        goto operator_done;
                    }
                }
                operator_done:
                r.emplace_back(SYMBOL, std::string(operator_begin, c), here);
                c--;  // Leave it on the last symbol.

            } else if (is_identifier1(*c)) {
                std::string id;
                for (; is_identifier(*c) ; ++c) {
                    id += *c;
                }
                --c;
                if (id == "discard") {
                    r.emplace_back(DISCARD, id, here);
                } else if (id == "else") {
                    r.emplace_back(ELSE, id, here);
                } else if (id == "for") {
                    r.emplace_back(FOR, id, here);
                } else if (id == "frag") {
                    r.emplace_back(FRAG, id, here);
                } else if (id == "global") {
                    r.emplace_back(GLOBAL, id, here);
                } else if (id == "if") {
                    r.emplace_back(IF, id, here);
                } else if (id == "mat") {
                    r.emplace_back(MAT, id, here);
                } else if (id == "return") {
                    r.emplace_back(RETURN, id, here);
                } else if (id == "var") {
                    r.emplace_back(VAR, id, here);
                } else if (id == "vert") {
                    r.emplace_back(VERT, id, here);
                } else if (id == "out") {
                    r.emplace_back(OUT, id, here);
                } else if (id == "body") {
                    r.emplace_back(BODY, id, here);

                } else if (id == "Float") {
                    r.emplace_back(TYPE_FLOAT, id, here);
                } else if (id == "Float2") {
                    r.emplace_back(TYPE_FLOAT2, id, here);
                } else if (id == "Float3") {
                    r.emplace_back(TYPE_FLOAT3, id, here);
                } else if (id == "Float4") {
                    r.emplace_back(TYPE_FLOAT4, id, here);
                } else if (id == "Float1x1") {
                    r.emplace_back(TYPE_FLOAT1x1, id, here);
                } else if (id == "Float2x1") {
                    r.emplace_back(TYPE_FLOAT2x1, id, here);
                } else if (id == "Float3x1") {
                    r.emplace_back(TYPE_FLOAT3x1, id, here);
                } else if (id == "Float4x1") {
                    r.emplace_back(TYPE_FLOAT4x1, id, here);
                } else if (id == "Float1x2") {
                    r.emplace_back(TYPE_FLOAT1x2, id, here);
                } else if (id == "Float2x2") {
                    r.emplace_back(TYPE_FLOAT2x2, id, here);
                } else if (id == "Float3x2") {
                    r.emplace_back(TYPE_FLOAT3x2, id, here);
                } else if (id == "Float4x2") {
                    r.emplace_back(TYPE_FLOAT4x2, id, here);
                } else if (id == "Float1x3") {
                    r.emplace_back(TYPE_FLOAT1x3, id, here);
                } else if (id == "Float2x3") {
                    r.emplace_back(TYPE_FLOAT2x3, id, here);
                } else if (id == "Float3x3") {
                    r.emplace_back(TYPE_FLOAT3x3, id, here);
                } else if (id == "Float4x3") {
                    r.emplace_back(TYPE_FLOAT4x3, id, here);
                } else if (id == "Float1x4") {
                    r.emplace_back(TYPE_FLOAT1x4, id, here);
                } else if (id == "Float2x4") {
                    r.emplace_back(TYPE_FLOAT2x4, id, here);
                } else if (id == "Float3x4") {
                    r.emplace_back(TYPE_FLOAT3x4, id, here);
                } else if (id == "Float4x4") {
                    r.emplace_back(TYPE_FLOAT4x4, id, here);
                } else if (id == "FloatTexture1") {
                    r.emplace_back(TYPE_FLOAT_TEXTURE1, id, here);
                } else if (id == "FloatTexture2") {
                    r.emplace_back(TYPE_FLOAT_TEXTURE2, id, here);
                } else if (id == "FloatTexture3") {
                    r.emplace_back(TYPE_FLOAT_TEXTURE3, id, here);
                } else if (id == "FloatTexture4") {
                    r.emplace_back(TYPE_FLOAT_TEXTURE4, id, here);
                } else if (id == "FloatTextureCube") {
                    r.emplace_back(TYPE_FLOAT_TEXTURE_CUBE, id, here);
                } else if (id == "Int") {
                    r.emplace_back(TYPE_INT, id, here);
                } else if (id == "Int2") {
                    r.emplace_back(TYPE_INT2, id, here);
                } else if (id == "Int3") {
                    r.emplace_back(TYPE_INT3, id, here);
                } else if (id == "Int4") {
                    r.emplace_back(TYPE_INT4, id, here);
                } else if (id == "Bool") {
                    r.emplace_back(TYPE_BOOL, id, here);
                } else if (id == "Void") {
                    r.emplace_back(TYPE_VOID, id, here);
                } else if (id == "Global") {
                    r.emplace_back(TYPE_GLOBAL, id, here);
                } else if (id == "Mat") {
                    r.emplace_back(TYPE_MAT, id, here);
                } else if (id == "Vert") {
                    r.emplace_back(TYPE_VERT, id, here);
                } else if (id == "Out") {
                    r.emplace_back(TYPE_OUT, id, here);
                } else if (id == "Body") {
                    r.emplace_back(TYPE_BODY, id, here);
                } else if (id == "Frag") {
                    r.emplace_back(TYPE_FRAG, id, here);

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

        GfxGslType *parseType (void)
        {
            GfxGslFloatVec black(0);  // TODO(dcunnin): This is a hack.
            auto tok = pop();
            switch (tok.kind) {
                case TYPE_FLOAT: return alloc.makeType<GfxGslFloatType>(1);
                case TYPE_FLOAT2: return alloc.makeType<GfxGslFloatType>(2);
                case TYPE_FLOAT3: return alloc.makeType<GfxGslFloatType>(3);
                case TYPE_FLOAT4: return alloc.makeType<GfxGslFloatType>(4);
                case TYPE_FLOAT1x1: return alloc.makeType<GfxGslFloatMatrixType>(1, 1);
                case TYPE_FLOAT2x1: return alloc.makeType<GfxGslFloatMatrixType>(2, 1);
                case TYPE_FLOAT3x1: return alloc.makeType<GfxGslFloatMatrixType>(3, 1);
                case TYPE_FLOAT4x1: return alloc.makeType<GfxGslFloatMatrixType>(4, 1);
                case TYPE_FLOAT1x2: return alloc.makeType<GfxGslFloatMatrixType>(1, 2);
                case TYPE_FLOAT2x2: return alloc.makeType<GfxGslFloatMatrixType>(2, 2);
                case TYPE_FLOAT3x2: return alloc.makeType<GfxGslFloatMatrixType>(3, 2);
                case TYPE_FLOAT4x2: return alloc.makeType<GfxGslFloatMatrixType>(4, 2);
                case TYPE_FLOAT1x3: return alloc.makeType<GfxGslFloatMatrixType>(1, 3);
                case TYPE_FLOAT2x3: return alloc.makeType<GfxGslFloatMatrixType>(2, 3);
                case TYPE_FLOAT3x3: return alloc.makeType<GfxGslFloatMatrixType>(3, 3);
                case TYPE_FLOAT4x3: return alloc.makeType<GfxGslFloatMatrixType>(4, 3);
                case TYPE_FLOAT1x4: return alloc.makeType<GfxGslFloatMatrixType>(1, 4);
                case TYPE_FLOAT2x4: return alloc.makeType<GfxGslFloatMatrixType>(2, 4);
                case TYPE_FLOAT3x4: return alloc.makeType<GfxGslFloatMatrixType>(3, 4);
                case TYPE_FLOAT4x4: return alloc.makeType<GfxGslFloatMatrixType>(4, 4);
                case TYPE_FLOAT_TEXTURE1: return alloc.makeType<GfxGslFloatTextureType>(black, 1);
                case TYPE_FLOAT_TEXTURE2: return alloc.makeType<GfxGslFloatTextureType>(black, 2);
                case TYPE_FLOAT_TEXTURE3: return alloc.makeType<GfxGslFloatTextureType>(black, 3);
                case TYPE_FLOAT_TEXTURE4: return alloc.makeType<GfxGslFloatTextureType>(black, 4);
                case TYPE_FLOAT_TEXTURE_CUBE:
                    return alloc.makeType<GfxGslFloatTextureCubeType>(black);
                case TYPE_INT: return alloc.makeType<GfxGslIntType>(1);
                case TYPE_INT2: return alloc.makeType<GfxGslIntType>(2);
                case TYPE_INT3: return alloc.makeType<GfxGslIntType>(3);
                case TYPE_INT4: return alloc.makeType<GfxGslIntType>(4);
                case TYPE_BOOL: return alloc.makeType<GfxGslBoolType>();
                case TYPE_VOID: return alloc.makeType<GfxGslVoidType>();
                case TYPE_GLOBAL: return alloc.makeType<GfxGslGlobalType>();
                case TYPE_MAT: return alloc.makeType<GfxGslMatType>();
                case TYPE_VERT: return alloc.makeType<GfxGslVertType>();
                case TYPE_OUT: return alloc.makeType<GfxGslOutType>();
                case TYPE_BODY: return alloc.makeType<GfxGslBodyType>();
                case TYPE_FRAG: return alloc.makeType<GfxGslFragType>();
                case SYMBOL: {
                    if (tok.val == "[") {
                        GfxGslAst *sz_ast = parseExpr(precedence_max);
                        auto *sz_ast2 = dynamic_cast<GfxGslLiteralInt*>(sz_ast);
                        if (sz_ast2 == nullptr || sz_ast2->val <= 0) {
                            error(tok.loc) << "Array size must be a positive integer" << ENDL;
                        }
                        unsigned sz = unsigned(sz_ast2->val);
                        popKind(SYMBOL, "]");
                        GfxGslType *el = parseType();
                        return alloc.makeType<GfxGslArrayType>(sz, el);
                    }
                    // Would put function type parsing here but we don't actually need it.
                } // Carry on to default case for bad symbol.
                default:
                    error(tok.loc) << "Expected a type, got " << tok << ENDL;
            }
        }

        GfxGslAst *parseExpr (int precedence)
        {
            auto tok = peek();
            if (precedence == 0) {
                switch (tok.kind) {
                    case MAT: return alloc.makeAst<GfxGslMat>(pop().loc);
                    case GLOBAL: return alloc.makeAst<GfxGslGlobal>(pop().loc);
                    case VERT: return alloc.makeAst<GfxGslVert>(pop().loc);
                    case OUT: return alloc.makeAst<GfxGslOut>(pop().loc);
                    case BODY: return alloc.makeAst<GfxGslBody>(pop().loc);
                    case FRAG: return alloc.makeAst<GfxGslFrag>(pop().loc);
                    case IDENTIFIER:
                    case TYPE_FLOAT: case TYPE_FLOAT2: case TYPE_FLOAT3: case TYPE_FLOAT4: {
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
                        // Parentheses
                        pop();
                        auto *expr = parseExpr(precedence_max);
                        popKind(SYMBOL, ")");
                        return expr;
                    } else if (tok.val == "[") {
                        // Array Literal
                        pop();
                        popKind(SYMBOL, "]");
                        GfxGslType *element_type = parseType();
                        popKind(SYMBOL, "{");
                        GfxGslAsts elements;
                        bool comma = true;
                        while (true) {
                            if (!comma && maybePopKind(SYMBOL, ","))
                                comma = true;
                            if (maybePopKind(SYMBOL, "}")) break;
                            elements.push_back(parseExpr(precedence_max));
                            comma = false;
                        }
                        return alloc.makeAst<GfxGslLiteralArray>(tok.loc, element_type, elements);
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
                        // Call
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
                    } else if (sym == "[") {
                        Token lbracket = pop();
                        GfxGslAst *index = parseExpr(precedence_max);
                        popKind(SYMBOL, "]");
                        a = alloc.makeAst<GfxGslArrayLookup>(lbracket.loc, a, index);
                    } else if (sym == ".") {
                        Token dot = pop();
                        const std::string &id = popKind(IDENTIFIER).val;
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

        // Currently this does not allow an expression on a line as expressions
        // do not have side-effects.  However, if we were to add user-defined functions
        // that had side-effects then we would want to at least allow calls as well.
        GfxGslAst *parseAssignment (void)
        {
            auto loc = peek().loc;
            auto *lhs = parseExpr(precedence_max);
            popKind(SYMBOL, "=");
            auto *rhs = parseExpr(precedence_max);
            return alloc.makeAst<GfxGslAssign>(loc, lhs, rhs);
        }

        GfxGslAst *parseStmt (void)
        {
            auto loc = peek().loc;
            if (maybePopKind(IF)) {
                popKind(SYMBOL, "(");
                auto *cond = parseExpr(precedence_max);
                popKind(SYMBOL, ")");
                auto *yes = parseStmt();
                if (maybePopKind(ELSE)) {
                    auto *no = parseStmt();
                    return alloc.makeAst<GfxGslIf>(loc, cond, yes, no);
                }
                return alloc.makeAst<GfxGslIf>(loc, cond, yes, nullptr);
            } else if (maybePopKind(FOR)) {
                popKind(SYMBOL, "(");
                GfxGslAst *init = nullptr;
                GfxGslType *annot = nullptr;
                std::string id;
                if (maybePopKind(VAR)) {
                    id = popKind(IDENTIFIER).val;
                    if (maybePopKind(SYMBOL, ":")) {
                        annot = parseType();
                    }
                    if (maybePopKind(SYMBOL, "=")) {
                        init = parseExpr(precedence_max);
                    }
                } else {
                    init = parseAssignment();
                }
                popKind(SYMBOL, ";");
                auto *cond = parseExpr(precedence_max);
                popKind(SYMBOL, ";");
                auto *inc = parseAssignment();
                popKind(SYMBOL, ")");
                auto *body = parseStmt();
                return alloc.makeAst<GfxGslFor>(loc, id, annot, init, cond, inc, body);
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
            } else if (maybePopKind(VAR)) {
                const auto &id = popKind(IDENTIFIER).val;
                GfxGslType *annot = nullptr;
                if (maybePopKind(SYMBOL, ":")) {
                    annot = parseType();
                }
                GfxGslAst *init = nullptr;
                if (maybePopKind(SYMBOL, "=")) {
                    init = parseExpr(precedence_max);
                }
                popKind(SYMBOL, ";");
                return alloc.makeAst<GfxGslDecl>(loc, id, annot, init);
            } else {
                GfxGslAst *ast = parseAssignment();
                popKind(SYMBOL, ";");
                return ast;
            }
        }

        GfxGslShader *parseShader (void)
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

GfxGslShader *gfx_gasoline_parse (GfxGslAllocator &alloc, const std::string &shader)
{
    auto tokens = lex(shader);
    Parser parser(alloc, tokens);
    return parser.parseShader();
}

// vim: shiftwidth=4:tabstop=4:expandtab
