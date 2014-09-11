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

#include "gfx_gasoline_parser.h"
#include "gfx_gasoline_type_system.h"

// Workaround for g++ not supporting moveable streams.
#define error(loc) (EXCEPT << "Shader error: " << loc << ": ")

std::ostream &operator<<(std::ostream &o, const GfxGslType *t_)
{   
    if (auto *t = dynamic_cast<const GfxGslFloatType*>(t_)) {
        o << "Float";
        if (t->dim > 1) o << t->dim;

    } else if (auto *t = dynamic_cast<const GfxGslFloatMatrixType*>(t_)) {
        o << "Float" << t->w << "x" << t->h;

    } else if (auto *t = dynamic_cast<const GfxGslFloatTextureType*>(t_)) {
        o << "FloatTexture";
        if (t->dim > 1) o << t->dim;

    } else if (auto *t = dynamic_cast<const GfxGslIntType*>(t_)) {
        o << "Int";
        if (t->dim > 1) o << t->dim;

    } else if (dynamic_cast<const GfxGslBoolType*>(t_)) {
        o << "Bool";

    } else if (dynamic_cast<const GfxGslVoidType*>(t_)) {
        o << "Void";

    } else if (dynamic_cast<const GfxGslGlobalType*>(t_)) {
        o << "Global";

    } else if (dynamic_cast<const GfxGslMatType*>(t_)) {
        o << "Mat";

    } else if (dynamic_cast<const GfxGslVertType*>(t_)) {
        o << "Vert";

    } else if (dynamic_cast<const GfxGslFragType*>(t_)) {
        o << "Frag";

    } else if (auto *t = dynamic_cast<const GfxGslFunctionType*>(t_)) {
        if (t->params.size() == 0) {
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


GfxGslType *GfxGslTypeSystem::cloneType (GfxGslType *t_)
{
    if (auto *t = dynamic_cast<GfxGslFloatType*>(t_)) {
        return alloc.makeType<GfxGslFloatType>(*t);
    } else if (auto *t = dynamic_cast<GfxGslFloatMatrixType*>(t_)) {
        return alloc.makeType<GfxGslFloatMatrixType>(*t);
    } else if (auto *t = dynamic_cast<GfxGslFloatTextureType*>(t_)) {
        return alloc.makeType<GfxGslFloatTextureType>(*t);
    } else if (auto *t = dynamic_cast<GfxGslIntType*>(t_)) {
        return alloc.makeType<GfxGslIntType>(*t);
    } else if (dynamic_cast<GfxGslBoolType*>(t_)) {
        return alloc.makeType<GfxGslBoolType>();
    } else if (dynamic_cast<GfxGslVoidType*>(t_)) {
        return alloc.makeType<GfxGslVoidType>();
    } else if (dynamic_cast<GfxGslGlobalType*>(t_)) {
        return alloc.makeType<GfxGslGlobalType>();
    } else if (dynamic_cast<GfxGslMatType*>(t_)) {
        return alloc.makeType<GfxGslMatType>();
    } else if (dynamic_cast<GfxGslVertType*>(t_)) {
        return alloc.makeType<GfxGslVertType>();
    } else if (dynamic_cast<GfxGslFragType*>(t_)) {
        return alloc.makeType<GfxGslFragType>();
    } else if (auto *t = dynamic_cast<GfxGslFunctionType*>(t_)) {
        return alloc.makeType<GfxGslFunctionType>(*t);
    } else {
        EXCEPTEX << "Internal error." << ENDL;
    }
}

void GfxGslTypeSystem::initObjectTypes (void)
{
    fragFields["colour"] = FieldType(alloc.makeType<GfxGslFloatType>(4), false, false, true, true);
    fragFields["position"] = FieldType(alloc.makeType<GfxGslFloatType>(4), true, true, true, false);

    vertFields["position"] = alloc.makeType<GfxGslFloatType>(4);
    vertFields["coord0"] = alloc.makeType<GfxGslFloatType>(4);

    globalFields["world"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    globalFields["worldViewProj"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    globalFields["viewProj"] = alloc.makeType<GfxGslFloatMatrixType>(4,4);
    globalFields["viewportSize"] = alloc.makeType<GfxGslFloatType>(2);
    globalFields["fovY"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["time"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["hellColour"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skyCloudColour"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skyCloudCoverage"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["skyGlareHorizonElevation"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["skyGlareSunDistance"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["skyDivider1"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["skyDivider2"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["skyDivider3"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["skyDivider4"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["skyColour0"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skyColour1"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skyColour2"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skyColour3"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skyColour4"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skyColour5"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skySunColour0"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skySunColour1"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skySunColour2"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skySunColour3"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["skySunColour4"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["sunColour"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["sunAlpha"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["sunDirection"] = alloc.makeType<GfxGslFloatType>(3);
    globalFields["sunFalloffDistance"] = alloc.makeType<GfxGslFloatType>(1);
    globalFields["sunSize"] = alloc.makeType<GfxGslFloatType>(1);
}

void GfxGslTypeSystem::initFuncTypes (void)
{
    auto is = [&] (int i) -> GfxGslIntType* { return alloc.makeType<GfxGslIntType>(i); };
    auto fs = [&] (int i) -> GfxGslFloatType* { return alloc.makeType<GfxGslFloatType>(i); };
    auto tex = [&] (int i) -> GfxGslFloatTextureType* { return alloc.makeType<GfxGslFloatTextureType>(i); };
    //GfxGslType *b = alloc.makeType<GfxGslBoolType>();
    // ts holds the set of all functions: float_n -> float_n
    std::vector<GfxGslFunctionType*> ts;
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i)}, fs(i)));
    funcTypes["tan"] = ts;
    funcTypes["atan"] = ts;
    funcTypes["sin"] = ts;
    funcTypes["asin"] = ts;
    funcTypes["cos"] = ts;
    funcTypes["acos"] = ts;
    funcTypes["ddx"] = ts;
    funcTypes["ddy"] = ts;
    funcTypes["sqrt"] = ts;

    funcTypes["pow"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(1)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4), fs(1)}, fs(4)),
    };

    funcTypes["atan2"] = { alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(1)) };

    funcTypes["dot"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(3)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(2)}, fs(1)),
    };
    funcTypes["normalize"] = { alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)) };

    // ts holds the set of all functions: (float_n, float_n, float_n) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(i), fs(i)}, fs(i)));
    funcTypes["clamp"] = ts;

    funcTypes["Float"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
    };
    funcTypes["Float2"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1)}, fs(2))
    };
    funcTypes["Float3"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(1)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(2)}, fs(3)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1), fs(1)}, fs(3)),
    };
    funcTypes["Float4"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{is(4)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(3)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(2)}, fs(4)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1), fs(2)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(2), fs(1)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2), fs(1), fs(1)}, fs(4)),

        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1), fs(1), fs(1), fs(1)}, fs(4))
    };

    // ts holds the set of all functions: (float_n, float_n, float1) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(i), fs(1)}, fs(i)));
    funcTypes["lerp"] = ts;

        
    // ts holds the set of all functions: (float_n, float_n) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(i)}, fs(i)));
    funcTypes["max"] = ts;
    funcTypes["min"] = ts;

    // ts holds the set of all functions: (float_n, float_1) -> float_n
    ts.clear();
    for (unsigned i=1 ; i<=4 ; ++i)
        ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(i), fs(1)}, fs(i)));
    funcTypes["mod"] = ts;

    ts.clear();
    for (unsigned w=1 ; w<=4 ; ++w) {
        for (unsigned h=1 ; h<=4 ; ++h) {
            GfxGslType *m = alloc.makeType<GfxGslFloatMatrixType>(w, h);
            ts.push_back(alloc.makeType<GfxGslFunctionType>(GfxGslTypes{m, fs(h)}, fs(h)));
        }
    }
    funcTypes["mul"] = ts;

    funcTypes["sample2D"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(2), fs(2)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(2), fs(2), fs(2), fs(2)}, fs(4)),
    };
    funcTypes["sample3D"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(3), fs(3)}, fs(4)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{tex(3), fs(3), fs(3), fs(3)}, fs(4)),
    };

    funcTypes["pma_decode"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
    };

    funcTypes["gamma_decode"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
    };

    funcTypes["gamma_encode"] = {
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(1)}, fs(1)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(2)}, fs(2)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(3)}, fs(3)),
        alloc.makeType<GfxGslFunctionType>(GfxGslTypes{fs(4)}, fs(4)),
    };

}

GfxGslFunctionType *GfxGslTypeSystem::lookupFunction(const GfxGslLocation &loc,
                                                     const std::string &name,
                                                     GfxGslAsts &asts)
{
    auto it = funcTypes.find(name);
    if (it == funcTypes.end()) {
        error(loc) << "Unrecognised function: " << name << ENDL;
    }
    const std::vector<GfxGslFunctionType*> &overrides = it->second;
    for (auto *o : overrides) {
        if (o->params.size() != asts.size()) continue;
        for (unsigned i=0 ; i<asts.size() ; ++i) {
            if (!equal(o->params[i], asts[i]->type)) goto nextfunc;
        }
        return o;
        nextfunc:;
    }

    GfxGslFunctionType *found = nullptr;
    for (auto *o : overrides) {
        //std::cout << o << std::endl;
        if (o->params.size() != asts.size()) continue;
        for (unsigned i=0 ; i<asts.size() ; ++i) {
            if (!conversionExists(asts[i]->type, o->params[i])) goto nextfunc2;
        }
        if (found != nullptr) {
            error(loc) << "Cannot unambiguously bind function: " << name << ENDL;
        }
        found = o;
        break;
        nextfunc2:;
    }

    if (found != nullptr) {
        for (unsigned i=0 ; i<asts.size() ; ++i) {
            doConversion(asts[i], found->params[i]);
        }
        return found;
    }

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

bool GfxGslTypeSystem::isFirstClass (GfxGslType *x)
{
    if (dynamic_cast<GfxGslFloatType*>(x)) {
        return true;
    } else if (dynamic_cast<GfxGslIntType*>(x)) {
        return true;
    } else if (dynamic_cast<GfxGslBoolType*>(x)) {
        return true;
    }
    return false;
}

bool GfxGslTypeSystem::equal (GfxGslType *a_, GfxGslType *b_)
{
    if (auto *a = dynamic_cast<GfxGslFloatType*>(a_)) {
        auto *b = dynamic_cast<GfxGslFloatType*>(b_);
        if (b == nullptr) return false;
        if (b->dim != a->dim) return false;

    } else if (auto *a = dynamic_cast<GfxGslIntType*>(a_)) {
        auto *b = dynamic_cast<GfxGslIntType*>(b_);
        if (b == nullptr) return false;
        if (b->dim != a->dim) return false;

    } else if (dynamic_cast<GfxGslBoolType*>(a_)) {
        auto *b = dynamic_cast<GfxGslBoolType*>(b_);
        if (b == nullptr) return false;

    } else if (dynamic_cast<GfxGslVoidType*>(a_)) {
        auto *b = dynamic_cast<GfxGslVoidType*>(b_);
        if (b == nullptr) return false;

    } else if (dynamic_cast<GfxGslGlobalType*>(a_)) {
        auto *b = dynamic_cast<GfxGslGlobalType*>(b_);
        if (b == nullptr) return false;

    } else if (dynamic_cast<GfxGslMatType*>(a_)) {
        auto *b = dynamic_cast<GfxGslMatType*>(b_);
        if (b == nullptr) return false;

    } else if (dynamic_cast<GfxGslVertType*>(a_)) {
        auto *b = dynamic_cast<GfxGslVertType*>(b_);
        if (b == nullptr) return false;

    } else if (dynamic_cast<GfxGslFragType*>(a_)) {
        auto *b = dynamic_cast<GfxGslFragType*>(b_);
        if (b == nullptr) return false;

    } else if (auto *a = dynamic_cast<GfxGslFunctionType*>(a_)) {
        auto *b = dynamic_cast<GfxGslFunctionType*>(b_);
        if (b == nullptr) return false;
        if (!equal(a->ret, b->ret)) return false;
        if (a->params.size() != b->params.size()) return false;
        for (unsigned i=0 ; i<a->params.size() ; ++i) {
            if (!equal(a->params[i], b->params[i])) return false;
        }
    }
    return true;
}


// Can the type be automatically converted
bool GfxGslTypeSystem::conversionExists (GfxGslType *from_, GfxGslType *to_)
{
    if (equal(from_, to_)) return true;
    
    if (dynamic_cast<GfxGslIntType*>(to_)) {
        if (auto *ft = dynamic_cast<GfxGslIntType*>(from_)) {
            if (ft->dim == 1) {
                // Int -> Int[n]
                return true;
            }
        }
    } else if (dynamic_cast<GfxGslFloatType*>(to_)) {
        if (auto *ft = dynamic_cast<GfxGslFloatType*>(from_)) {
            if (ft->dim == 1) {
                // Float -> Float[n]
                return true;
            }
        } else if (auto *ft = dynamic_cast<GfxGslIntType*>(from_)) {
            // Int -> Float[n]
            if (ft->dim == 1) {
                return true;
            }
        }
    }
    return false;
}

// Wrap from, if necessary, to convert type.
void GfxGslTypeSystem::doConversion (GfxGslAst *&from, GfxGslType *to_)
{
    if (equal(from->type, to_)) {
        // Nothing to do.
        return;
    }

    if (auto *tt = dynamic_cast<GfxGslIntType*>(to_)) {
        const char *names[] = { "Int1", "Int2", "Int3", "Int4" };
        if (auto *ft = dynamic_cast<GfxGslIntType*>(from->type)) {
            if (ft->dim == 1) {
                // Int -> Int[n]
                from = alloc.makeAst<GfxGslCall>(from->loc, names[tt->dim-1], GfxGslAsts{from});
                from->type = cloneType(tt);
                return;
            }
        }
    } else if (auto *tt = dynamic_cast<GfxGslFloatType*>(to_)) {
        const char *names[] = { "Float1", "Float2", "Float3", "Float4" };
        if (auto *ft = dynamic_cast<GfxGslFloatType*>(from->type)) {
            if (ft->dim == 1) {
                // Float -> Float[n]
                from = alloc.makeAst<GfxGslCall>(from->loc, names[tt->dim-1], GfxGslAsts{from});
                from->type = cloneType(tt);
                return;
            }
        } else if (auto *ft = dynamic_cast<GfxGslIntType*>(from->type)) {
            // Int1 -> Float[n]
            if (ft->dim == 1) {
                from = alloc.makeAst<GfxGslCall>(from->loc, names[tt->dim-1], GfxGslAsts{from});
                from->type = cloneType(tt);
                return;
            }
        }
    }
    EXCEPTEX << "No conversion possible." << ENDL;
}

void GfxGslTypeSystem::unify (const GfxGslLocation &loc, GfxGslAst *&a, GfxGslAst *&b)
{
    ASSERT(a != nullptr);
    ASSERT(b != nullptr);
    ASSERT(a->type != nullptr);
    ASSERT(b->type != nullptr);
    if (conversionExists(a->type, b->type)) {
        doConversion(a, b->type);
        return;
    }
    if (conversionExists(b->type, a->type)) {
        doConversion(b, a->type);
        return;
    }
    error(loc) << "Could not unify types " << a->type << " and " << b->type << ENDL;
}

void GfxGslTypeSystem::inferAndSet (GfxGslAst *ast_)
{
    ASSERT(ast_ != nullptr);
    const GfxGslLocation &loc = ast_->loc;

    if (auto *ast = dynamic_cast<GfxGslBlock*>(ast_)) {
        for (auto stmt : ast->stmts) {
            inferAndSet(stmt);
        }
        ast->type = alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslShader*>(ast_)) {
        for (auto stmt : ast->stmts) {
            inferAndSet(stmt);
        }
        ast->type = alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslDecl*>(ast_)) {
        inferAndSet(ast->init);
        if (!isFirstClass(ast->init->type)) {
            error(loc) << "Type is not first class: " << ast->init->type << ENDL;
        }
        if (!ast->immutable) ast->init->type->writeable = true;
        vars[ast->id] = ast->init->type;
        ast->type = alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslIf*>(ast_)) {
        inferAndSet(ast->cond);
        inferAndSet(ast->yes);
        if (ast->no) {
            inferAndSet(ast->no);
            unify(loc, ast->yes, ast->no);
        }
        ast->type = alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslAssign*>(ast_)) {
        inferAndSet(ast->target);
        if (!ast->target->type->writeable) {
            error(loc) << "Cannot assign to this object." <<  ENDL;
        }
        inferAndSet(ast->expr);
        
        ast->type = alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslDiscard*>(ast_)) {
        ast->type = alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslReturn*>(ast_)) {
        ast->type = alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslCall*>(ast_)) {
        for (unsigned i=0 ; i<ast->args.size() ; ++i)
            inferAndSet(ast->args[i]);
        GfxGslFunctionType *t = lookupFunction(loc, ast->func, ast->args);
        ast->type = t->ret;

    } else if (auto *ast = dynamic_cast<GfxGslField*>(ast_)) {
        inferAndSet(ast->target);
        if (dynamic_cast<GfxGslFragType*>(ast->target->type)) {
            if (fragFields.find(ast->id) == fragFields.end()) {
                error(loc) << "Frag field does not exist:  " << ast->id << ENDL;
            }
            const FieldType &f = fragFields[ast->id];
            ast->type = cloneType(f.t);
            if (kind == GFX_GSL_FRAG) {
                if (!f.fr) {
                    error(loc) << "frag." << ast->id << " not accessible in fragment shader." << ENDL;
                }
                ast->type->writeable = f.fw;
            } else {
                if (!f.vr) {
                    error(loc) << "frag." << ast->id << " not accessible in vertex shader." << ENDL;
                }
                ast->type->writeable = f.vw;
            }
        } else if (dynamic_cast<GfxGslVertType*>(ast->target->type)) {
            if (vertFields.find(ast->id) == vertFields.end()) {
                error(loc) << "vert." << ast->id << " does not exist." << ENDL;
            }
            GfxGslType *t = vertFields[ast->id];
            ast->type = cloneType(t);
            ast->type->writeable = false;
        } else if (dynamic_cast<GfxGslGlobalType*>(ast->target->type)) {
            if (globalFields.find(ast->id) == globalFields.end()) {
                error(loc) << "global." << ast->id << " does not exist." << ENDL;
            }
            GfxGslType *t = globalFields[ast->id];
            ast->type = cloneType(t);
            ast->type->writeable = false;
        } else if (dynamic_cast<GfxGslMatType*>(ast->target->type)) {
            if (matFields.find(ast->id) == matFields.end()) {
                error(loc) << "mat." << ast->id << " does not exist." << ENDL;
            }
            GfxGslType *t = matFields[ast->id];
            ast->type = cloneType(t);
            if (dynamic_cast<GfxGslFloatTextureType*>(ast->type)) {
                if (kind == GFX_GSL_VERT) {
                    error(loc) << "Cannot use textures in vertex shader: " << ast->id << ENDL;
                }
            }
            ast->type->writeable = false;
        } else if (auto *ft = dynamic_cast<GfxGslFloatType*>(ast->target->type)) {
            unsigned dim = ft->dim;
            const std::string &id = ast->id;
            if (id.length() > 4) {
                error(loc) << "Invalid field of " << ft << ": " << ast->id << ENDL;
            }
            for (unsigned i=0 ; i<id.length() ; ++i) {
                unsigned offset;
                switch (id[i]) {
                    case 'r': case 'x': offset = 0; break;
                    case 'g': case 'y': offset = 1; break;
                    case 'b': case 'z': offset = 2; break;
                    case 'a': case 'w': offset = 3; break;
                    default:
                    error(loc) << "Invalid field of " << ft << ": " << ast->id << ENDL;
                }
                if (offset >= dim) {
                    error(loc) << "Invalid field of " << ft << ": " << ast->id << ENDL;
                }
            }
            ast->type = alloc.makeType<GfxGslFloatType>(id.length());
            ast->type->writeable = ft->writeable;
            
        } else {
            error(loc) << "Cannot access " << ft << " of type: " << ast->target->type << ENDL;
        }

    } else if (auto *ast = dynamic_cast<GfxGslLiteralInt*>(ast_)) {
        ast->type = alloc.makeType<GfxGslIntType>(1);

    } else if (auto *ast = dynamic_cast<GfxGslLiteralFloat*>(ast_)) {
        ast->type = alloc.makeType<GfxGslFloatType>(1);

    } else if (auto *ast = dynamic_cast<GfxGslVar*>(ast_)) {
        auto it = vars.find(ast->id);
        if (it == vars.end()) {
            error(loc) << "Unknown variable: " << ast->id << ENDL;
        }
        ast->type = vars[ast->id];

    } else if (auto *ast = dynamic_cast<GfxGslBinary*>(ast_)) {
        inferAndSet(ast->a);
        inferAndSet(ast->b);
        unify(loc, ast->a, ast->b);
        GfxGslType *t = ast->a->type;
        switch (ast->op) {
            case GFX_GSL_OP_ADD: 
            case GFX_GSL_OP_SUB: 
            case GFX_GSL_OP_MUL: 
            case GFX_GSL_OP_DIV: 
            case GFX_GSL_OP_MOD: {
                if (auto *ct = dynamic_cast<GfxGslCoordType*>(t)) {
                    ast->type = alloc.makeType<GfxGslFloatType>(ct->dim);
                } else {
                    error(loc) << "Type error at operator " << ast->op << ENDL;
                }
            } break;

            case GFX_GSL_OP_EQ:
            case GFX_GSL_OP_NE:
            case GFX_GSL_OP_LT:
            case GFX_GSL_OP_LTE:
            case GFX_GSL_OP_GT:
            case GFX_GSL_OP_GTE: {
                if (dynamic_cast<GfxGslCoordType*>(t)) {
                    ast->type = alloc.makeType<GfxGslBoolType>();
                } else {
                    error(loc) << "Type error at operator " << ast->op << ENDL;
                }
            } break;

            case GFX_GSL_OP_AND:
            case GFX_GSL_OP_OR: {
                if (dynamic_cast<GfxGslBoolType*>(t)) {
                    ast->type = alloc.makeType<GfxGslBoolType>();
                } else {
                    error(loc) << "Type error at operator " << ast->op << ENDL;
                }
            } break;
        }

    } else if (auto *ast = dynamic_cast<GfxGslGlobal*>(ast_)) {
        ast->type = alloc.makeType<GfxGslGlobalType>();

    } else if (auto *ast = dynamic_cast<GfxGslMat*>(ast_)) {
        ast->type = alloc.makeType<GfxGslMatType>();

    } else if (auto *ast = dynamic_cast<GfxGslVert*>(ast_)) {
        ast->type = alloc.makeType<GfxGslVertType>();

    } else if (auto *ast = dynamic_cast<GfxGslFrag*>(ast_)) {
        ast->type = alloc.makeType<GfxGslFragType>();

    } else {
        EXCEPTEX << "INTERNAL ERROR: Unknown Ast." << ENDL;

    }
}

