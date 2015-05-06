/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

#include "../centralised_log.h"

#include "gfx_gasoline_parser.h"
#include "gfx_gasoline_type_system.h"

// Workaround for g++ not supporting moveable streams.
#define error(loc) (EXCEPT << "Type error: " << loc << ": ")

static bool frag(GfxGslKind k)
{
    return k != GFX_GSL_VERTEX;
}

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

    } else if (dynamic_cast<const GfxGslFloatTextureCubeType*>(t_)) {
        o << "FloatTextureCube";

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

    } else if (dynamic_cast<const GfxGslOutType*>(t_)) {
        o << "Out";

    } else if (dynamic_cast<const GfxGslBodyType*>(t_)) {
        o << "Body";

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


GfxGslType *GfxGslTypeSystem::cloneType (const GfxGslType *t_)
{
    if (auto *t = dynamic_cast<const GfxGslFloatType*>(t_)) {
        return ctx.alloc.makeType<GfxGslFloatType>(*t);
    } else if (auto *t = dynamic_cast<const GfxGslFloatMatrixType*>(t_)) {
        return ctx.alloc.makeType<GfxGslFloatMatrixType>(*t);
    } else if (auto *t = dynamic_cast<const GfxGslFloatTextureType*>(t_)) {
        return ctx.alloc.makeType<GfxGslFloatTextureType>(*t);
    } else if (dynamic_cast<const GfxGslFloatTextureCubeType*>(t_)) {
        return ctx.alloc.makeType<GfxGslFloatTextureCubeType>();
    } else if (auto *t = dynamic_cast<const GfxGslIntType*>(t_)) {
        return ctx.alloc.makeType<GfxGslIntType>(*t);
    } else if (dynamic_cast<const GfxGslBoolType*>(t_)) {
        return ctx.alloc.makeType<GfxGslBoolType>();
    } else if (dynamic_cast<const GfxGslVoidType*>(t_)) {
        return ctx.alloc.makeType<GfxGslVoidType>();
    } else if (dynamic_cast<const GfxGslGlobalType*>(t_)) {
        return ctx.alloc.makeType<GfxGslGlobalType>();
    } else if (dynamic_cast<const GfxGslMatType*>(t_)) {
        return ctx.alloc.makeType<GfxGslMatType>();
    } else if (dynamic_cast<const GfxGslVertType*>(t_)) {
        return ctx.alloc.makeType<GfxGslVertType>();
    } else if (dynamic_cast<const GfxGslOutType*>(t_)) {
        return ctx.alloc.makeType<GfxGslOutType>();
    } else if (dynamic_cast<const GfxGslBodyType*>(t_)) {
        return ctx.alloc.makeType<GfxGslBodyType>();
    } else if (dynamic_cast<const GfxGslFragType*>(t_)) {
        return ctx.alloc.makeType<GfxGslFragType>();
    } else if (auto *t = dynamic_cast<const GfxGslFunctionType*>(t_)) {
        return ctx.alloc.makeType<GfxGslFunctionType>(*t);
    } else {
        EXCEPTEX << "Internal error." << ENDL;
    }
}

void GfxGslTypeSystem::initObjectTypes (GfxGslKind k)
{
    switch (k) {
        case GFX_GSL_COLOUR_ALPHA:
        outFields["colour"] = ctx.alloc.makeType<GfxGslFloatType>(3);
        outFields["alpha"] = ctx.alloc.makeType<GfxGslFloatType>(1);
        break;

        case GFX_GSL_DANGS:
        outFields["diffuse"] = ctx.alloc.makeType<GfxGslFloatType>(3);
        outFields["alpha"] = ctx.alloc.makeType<GfxGslFloatType>(1);
        outFields["normal"] = ctx.alloc.makeType<GfxGslFloatType>(3);
        outFields["gloss"] = ctx.alloc.makeType<GfxGslFloatType>(1);
        outFields["specular"] = ctx.alloc.makeType<GfxGslFloatType>(1);
        break;

        case GFX_GSL_VERTEX:
        outFields["position"] = ctx.alloc.makeType<GfxGslFloatType>(3);
        break;
    }

    if (frag(k))
        fragFields["screen"] = ctx.alloc.makeType<GfxGslFloatType>(2);

    vertFields["coord0"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["coord1"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["coord2"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["coord3"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["coord4"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["coord5"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["coord6"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["coord7"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["normal"] = ctx.alloc.makeType<GfxGslFloatType>(3);
    vertFields["colour"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["tangent"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["position"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["boneWeights"] = ctx.alloc.makeType<GfxGslFloatType>(4);
    vertFields["boneAssignments"] = ctx.alloc.makeType<GfxGslFloatType>(4);

}

GfxGslFunctionType *GfxGslTypeSystem::lookupFunction(const GfxGslLocation &loc,
                                                     const std::string &name,
                                                     GfxGslAsts &asts)
{
    auto it = ctx.funcTypes.find(name);
    if (it == ctx.funcTypes.end()) {
        error(loc) << "Unrecognised function: " << name << ENDL;
    }

    // Bind if it's an exact match
    const std::vector<GfxGslFunctionType*> &overloads = it->second;
    for (auto *o : overloads) {
        if (o->params.size() != asts.size()) continue;
        for (unsigned i=0 ; i<asts.size() ; ++i) {
            if (!equal(o->params[i], asts[i]->type)) goto nextfunc;
        }
        //std::cout << "Exact match: " << name << ": " << o << std::endl;
        return o;
        nextfunc:;
    }

    // Otherwise search for one that matches with conversions.
    // If there is just 1, use it, otherwise error.
    GfxGslFunctionType *found = nullptr;
    for (auto *o : overloads) {
        //std::cout << "Trying with conversions: " << name << ": " << o << std::endl;
        if (o->params.size() != asts.size()) continue;
        for (unsigned i=0 ; i<asts.size() ; ++i) {
            if (!conversionExists(asts[i]->type, o->params[i])) goto nextfunc2;
        }
        if (found != nullptr) {
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
            error(loc) << "Cannot unambiguously bind function: " << name << ss.str() << ENDL;
        }
        found = o;
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

    } else if (auto *a = dynamic_cast<GfxGslFloatMatrixType*>(a_)) {
        auto *b = dynamic_cast<GfxGslFloatMatrixType*>(b_);
        if (b == nullptr) return false;
        if (b->w != a->w) return false;
        if (b->h != a->h) return false;

    } else if (auto *a = dynamic_cast<GfxGslFloatTextureType*>(a_)) {
        auto *b = dynamic_cast<GfxGslFloatTextureType*>(b_);
        if (b == nullptr) return false;
        if (b->dim != a->dim) return false;

    } else if (dynamic_cast<GfxGslFloatTextureCubeType*>(a_)) {
        auto *b = dynamic_cast<GfxGslFloatTextureCubeType*>(b_);
        if (b == nullptr) return false;

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

    } else if (dynamic_cast<GfxGslOutType*>(a_)) {
        auto *b = dynamic_cast<GfxGslOutType*>(b_);
        if (b == nullptr) return false;

    } else if (dynamic_cast<GfxGslBodyType*>(a_)) {
        auto *b = dynamic_cast<GfxGslBodyType*>(b_);
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
    } else {
        EXCEPTEX << "Internal error: " << a_ << ENDL;
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
        const char *names[] = { "Int", "Int2", "Int3", "Int4" };
        if (auto *ft = dynamic_cast<GfxGslIntType*>(from->type)) {
            if (ft->dim == 1) {
                // Int -> Int[n]
                from = ctx.alloc.makeAst<GfxGslCall>(from->loc, names[tt->dim-1], GfxGslAsts{from});
                from->type = cloneType(tt);
                return;
            }
        }
    } else if (auto *tt = dynamic_cast<GfxGslFloatType*>(to_)) {
        const char *names[] = { "Float", "Float2", "Float3", "Float4" };
        if (auto *ft = dynamic_cast<GfxGslFloatType*>(from->type)) {
            if (ft->dim == 1) {
                // Float -> Float[n]
                from = ctx.alloc.makeAst<GfxGslCall>(from->loc, names[tt->dim-1], GfxGslAsts{from});
                from->type = cloneType(tt);
                return;
            }
        } else if (auto *ft = dynamic_cast<GfxGslIntType*>(from->type)) {
            // Int1 -> Float[n]
            if (ft->dim == 1) {
                from = ctx.alloc.makeAst<GfxGslCall>(from->loc, names[tt->dim-1], GfxGslAsts{from});
                from->type = cloneType(tt);
                return;
            }
        }
    }
    error(from->loc) << "Could not convert type " << from->type << " to " << to_ << ENDL;
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

static bool get_offset (char c, unsigned &offset)
{
    switch (c) {
        case 'r': case 'x': offset = 0; return true;
        case 'g': case 'y': offset = 1; return true;
        case 'b': case 'z': offset = 2; return true;
        case 'a': case 'w': offset = 3; return true;
        default:
        return false;
    }
}

void GfxGslTypeSystem::addTrans (const GfxGslLocation &loc, const std::vector<std::string> &path,
                                 const GfxGslType *type, bool vert)
{
    auto *ft = dynamic_cast<const GfxGslFloatType *>(type);
    if (ft == nullptr)
        error(loc) << "Can only capture floating point vertex attributes." << ENDL;
    std::set<unsigned> parts;
    if (path.size() == 1) {
        for (unsigned i=0 ; i<ft->dim ; ++i) {
            parts.insert(i);
        }
    } else if (path.size() == 2) {
        std::string last = path[1];
        for (unsigned i=0 ; i<last.length() ; ++i) {
            unsigned offset;
            if (!get_offset(last[i], offset))
                EXCEPTEX << "Internal error." << ENDL;
            ASSERT(offset < ft->dim);
            parts.insert(offset);
        }
    } else {
        EXCEPTEX << "Internal error." << ENDL;
    }
    static const char *fields = "xyzw";
    for (unsigned p : parts) {
        GfxGslTrans t;
        t.kind = vert ? GfxGslTrans::VERT : GfxGslTrans::USER;
        t.path.push_back(path[0]);
        if (ft->dim > 1) {
            t.path.emplace_back(1, fields[p]);
        }
        trans.insert(t);
    }
}

void GfxGslTypeSystem::inferAndSet (GfxGslAst *ast_, const Ctx &c)
{
    ASSERT(ast_ != nullptr);
    const GfxGslLocation &loc = ast_->loc;

    if (auto *ast = dynamic_cast<GfxGslBlock*>(ast_)) {
        for (auto stmt : ast->stmts) {
            inferAndSet(stmt, c);
        }
        ast->type = ctx.alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslShader*>(ast_)) {
        for (auto stmt : ast->stmts) {
            inferAndSet(stmt, c);
        }
        ast->type = ctx.alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslDecl*>(ast_)) {
        inferAndSet(ast->init, Ctx().setRead(true));
        if (vars.find(ast->id) != vars.end())
            error(loc) << "Variable already defined: " << ast->id << ENDL;
        if (!isFirstClass(ast->init->type))
            error(loc) << "Variable cannot have type: " << ast->init->type << ENDL;
        GfxGslType *type = cloneType(ast->init->type);
        type->writeable = true;
        vars[ast->id] = type;
        ast->type = ctx.alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslIf*>(ast_)) {
        inferAndSet(ast->cond, Ctx().setRead(true));
        inferAndSet(ast->yes, c);
        if (ast->no) {
            inferAndSet(ast->no, c);
            unify(loc, ast->yes, ast->no);
        }
        ast->type = ctx.alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslAssign*>(ast_)) {
        inferAndSet(ast->target, Ctx().setWrite(true));
        if (!ast->target->type->writeable)
            error(loc) << "Cannot assign to this object." <<  ENDL;
        inferAndSet(ast->expr, c.setRead(true));
        doConversion(ast->expr, ast->target->type);
        ast->type = ctx.alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslDiscard*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslReturn*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslVoidType>();

    } else if (auto *ast = dynamic_cast<GfxGslCall*>(ast_)) {
        if (c.write)
            error(loc) << "Cannot assign to result of function call." <<  ENDL;
        for (unsigned i=0 ; i<ast->args.size() ; ++i)
            inferAndSet(ast->args[i], Ctx().setRead(true));
        GfxGslFunctionType *t = lookupFunction(loc, ast->func, ast->args);
        ast->type = t->ret;

    } else if (auto *ast = dynamic_cast<GfxGslField*>(ast_)) {
        inferAndSet(ast->target, c.appendPath(ast->id));
        if (dynamic_cast<GfxGslFragType*>(ast->target->type)) {
            if (fragFields.find(ast->id) == fragFields.end())
                error(loc) << "Frag field does not exist:  " << ast->id << ENDL;
            const GfxGslType *type = fragFields[ast->id];
            ast->type = cloneType(type);
            ast->type->writeable = false;
            if (c.read) {
                fragFieldsRead.insert(ast->id);
            }
            if (c.write) {
                error(loc) << "frag." << ast->id << " cannot be written." << ENDL;
            }
        } else if (dynamic_cast<GfxGslVertType*>(ast->target->type)) {
            if (vertFields.find(ast->id) == vertFields.end())
                error(loc) << "vert." << ast->id << " does not exist." << ENDL;
            const GfxGslType *type = vertFields[ast->id];
            ast->type = cloneType(type);
            ast->type->writeable = false;
            if (c.read) {
                if (kind == GFX_GSL_VERTEX) {
                    vertFieldsRead.insert(ast->id);
                } else {
                    addTrans(loc, c.appendPath(ast->id).path, type, true);
                }
            }
            if (c.write)
                error(loc) << "vert." << ast->id << " cannot be written." << ENDL;
        } else if (dynamic_cast<GfxGslOutType*>(ast->target->type)) {
            if (outFields.find(ast->id) == outFields.end())
                error(loc) << "out." << ast->id << " does not exist." << ENDL;
            const GfxGslType *type = outFields[ast->id];
            ast->type = cloneType(type);
            ast->type->writeable = true;
            if (c.write)
                outFieldsWritten.insert(ast->id);
        } else if (dynamic_cast<GfxGslBodyType*>(ast->target->type)) {
            if (bodyFields.find(ast->id) == outFields.end())
                error(loc) << "body." << ast->id << " does not exist." << ENDL;
            const GfxGslType *type = bodyFields[ast->id];
            ast->type = cloneType(type);
            ast->type->writeable = true;
            if (c.write)
                outFieldsWritten.insert(ast->id);
        } else if (dynamic_cast<GfxGslGlobalType*>(ast->target->type)) {
            if (ctx.globalFields.find(ast->id) == ctx.globalFields.end())
                error(loc) << "global." << ast->id << " does not exist." << ENDL;
            ast->type = cloneType(ctx.globalFields[ast->id]);
            ast->type->writeable = false;
            if (c.read)
                globalFieldsRead.insert(ast->id);
            if (c.write)
                error(loc) << "global." << ast->id << " cannot be written." << ENDL;
        } else if (dynamic_cast<GfxGslMatType*>(ast->target->type)) {
            if (ctx.matFields.find(ast->id) == ctx.matFields.end())
                error(loc) << "mat." << ast->id << " does not exist." << ENDL;
            ast->type = cloneType(ctx.matFields[ast->id]);
            if (dynamic_cast<GfxGslTextureType*>(ast->type)) {
                if (!frag(kind)) {
                    error(loc) << "Cannot use textures in vertex shader: " << ast->id << ENDL;
                }
            }
            ast->type->writeable = false;
            if (c.read)
                matFieldsRead.insert(ast->id);
            if (c.write)
                error(loc) << "mat." << ast->id << " cannot be written." << ENDL;
        } else if (auto *ft = dynamic_cast<GfxGslFloatType*>(ast->target->type)) {
            unsigned dim = ft->dim;
            const std::string &id = ast->id;
            if (id.length() > 4) {
                error(loc) << "Invalid field of " << ft << ": " << ast->id << ENDL;
            }
            for (unsigned i=0 ; i<id.length() ; ++i) {
                unsigned offset;
                if (!get_offset(id[i], offset))
                    error(loc) << "Invalid field of " << ft << ": " << ast->id << ENDL;
                if (offset >= dim) {
                    error(loc) << "Invalid field of " << ft << ": " << ast->id << ENDL;
                }
            }
            ast->type = ctx.alloc.makeType<GfxGslFloatType>(id.length());
            ast->type->writeable = ft->writeable;
            
        } else {
            error(loc) << "Cannot access " << ft << " of type: " << ast->target->type << ENDL;
        }

    } else if (auto *ast = dynamic_cast<GfxGslLiteralInt*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslIntType>(1);

    } else if (auto *ast = dynamic_cast<GfxGslLiteralFloat*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslFloatType>(1);

    } else if (auto *ast = dynamic_cast<GfxGslVar*>(ast_)) {
        auto it = vars.find(ast->id);
        if (it == vars.end()) {
            error(loc) << "Unknown variable: " << ast->id << ENDL;
        }
        ast->type = cloneType(vars[ast->id]);
        if (c.write && !ast->type->writeable)
            error(loc) << "Cannot write to variable: " << ast->id << ENDL;

        if (outerVars.find(ast->id) != outerVars.end()) {
            addTrans(loc, c.appendPath(ast->id).path, ast->type, false);
        }

    } else if (auto *ast = dynamic_cast<GfxGslBinary*>(ast_)) {
        inferAndSet(ast->a, c.setRead(true));
        inferAndSet(ast->b, c.setRead(true));
        unify(loc, ast->a, ast->b);
        GfxGslType *t = ast->a->type;
        switch (ast->op) {
            case GFX_GSL_OP_ADD: 
            case GFX_GSL_OP_SUB: 
            case GFX_GSL_OP_MUL: 
            case GFX_GSL_OP_DIV: 
            case GFX_GSL_OP_MOD: {
                if (auto *ct = dynamic_cast<GfxGslCoordType*>(t)) {
                    ast->type = ctx.alloc.makeType<GfxGslFloatType>(ct->dim);
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
                    ast->type = ctx.alloc.makeType<GfxGslBoolType>();
                } else {
                    error(loc) << "Type error at operator " << ast->op << ENDL;
                }
            } break;

            case GFX_GSL_OP_AND:
            case GFX_GSL_OP_OR: {
                if (dynamic_cast<GfxGslBoolType*>(t)) {
                    ast->type = ctx.alloc.makeType<GfxGslBoolType>();
                } else {
                    error(loc) << "Type error at operator " << ast->op << ENDL;
                }
            } break;
        }

    } else if (auto *ast = dynamic_cast<GfxGslGlobal*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslGlobalType>();

    } else if (auto *ast = dynamic_cast<GfxGslMat*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslMatType>();

    } else if (auto *ast = dynamic_cast<GfxGslVert*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslVertType>();

    } else if (auto *ast = dynamic_cast<GfxGslFrag*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslFragType>();

    } else if (auto *ast = dynamic_cast<GfxGslOut*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslOutType>();

    } else if (auto *ast = dynamic_cast<GfxGslBody*>(ast_)) {
        ast->type = ctx.alloc.makeType<GfxGslBodyType>();

    } else {
        EXCEPTEX << "INTERNAL ERROR: Unknown Ast." << ENDL;

    }
}
