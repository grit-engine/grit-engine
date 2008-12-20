#include <iostream>
#include <cstdlib>

#include <OgreException.h>

#include <LinearMath/btGeometryUtil.h>

#include "TColLexer"

#include "TColParser.h"

static inline bool fnear(const float x, const float y)
{
        return fabs(x-y) < 1E-6;
}

static inline bool ffar(const float x, const float y)
{
        return !fnear(x,y);
}

static std::string str (const quex::Token &t)
{
        std::string tmp;
        typedef std::basic_string<QUEX_CHARACTER_TYPE> S;
        S tmp2 = t.text();
        for (S::iterator i=tmp2.begin(),i_=tmp2.end() ; i!=i_ ; ++i) {
                uint8_t utf8[7];
                int utf8_length = quex::Quex_unicode_to_utf8(*i, utf8);
                if (utf8_length<0 || utf8_length>6) continue;
                utf8[utf8_length] = '\0';
                tmp += std::string((const char*)utf8);
        }
        return tmp;
}

const char *utf8 (const quex::Token &t)
{
        // the lexer is ascii so a direct translation is possible
        return (char*) &t.text()[0];
}

static std::string where (quex::TColLexer* qlex)
{
        std::stringstream ss;
        ss << qlex->line_number() << "," << qlex->column_number();
        return ss.str();
}

static const char * what2 (QUEX_TOKEN_ID_TYPE tid)
{
        switch (tid) {
                case QUEX_TKN_TERMINATION: return "end of file";
                case QUEX_TKN_TCOL: return "TCOL header";

                case QUEX_TKN_ATTRIBUTES: return "attributes";
                case QUEX_TKN_STATIC: return "static";
                case QUEX_TKN_MASS: return "mass";
                case QUEX_TKN_INERTIA: return "inertia";
                case QUEX_TKN_FRICTION: return "friction";
                case QUEX_TKN_RESTITUTION: return "restitution";
                case QUEX_TKN_LINEAR_DAMPING: return "linear_damping";
                case QUEX_TKN_ANGULAR_DAMPING: return "angular_damping";
                case QUEX_TKN_LINEAR_SLEEP_THRESHOLD:
                        return "linear_sleep_threshold";
                case QUEX_TKN_ANGULAR_SLEEP_THRESHOLD:
                        return "angular_sleep_threshold";
                case QUEX_TKN_CCD_MOTION_THRESHOLD:
                        return "ccd_motion_threshold";
                case QUEX_TKN_CCD_SWEPT_SPHERE_RADIUS:
                        return "ccd_swept_sphere_radius";

                case QUEX_TKN_MARGIN: return "margin";
                case QUEX_TKN_SHRINK: return "shrink";
                case QUEX_TKN_CENTRE: return "centre";
                case QUEX_TKN_NORMAL: return "normal";
                case QUEX_TKN_ORIENTATION: return "orientation";
                case QUEX_TKN_DIMENSIONS: return "dimensions";
                case QUEX_TKN_RADIUS: return "radius";
                case QUEX_TKN_HEIGHT: return "height";
                case QUEX_TKN_DISTANCE: return "distance";
                case QUEX_TKN_VERTEXES: return "vertexes";
                case QUEX_TKN_FACES: return "faces";

                case QUEX_TKN_COMPOUND: return "compound";
                case QUEX_TKN_HULL: return "hull";
                case QUEX_TKN_BOX: return "box";
                case QUEX_TKN_CYLINDER: return "cylinder";
                case QUEX_TKN_CONE: return "cone";
                case QUEX_TKN_SPHERE: return "sphere";
                case QUEX_TKN_PLANE: return "plane";
                case QUEX_TKN_CAPSULE: return "capsule";
                case QUEX_TKN_MULTISPHERE: return "multisphere";
                case QUEX_TKN_TRIMESH: return "trimesh";

                case QUEX_TKN_SEMI: return ";";
                case QUEX_TKN_LBRACE: return "{";
                case QUEX_TKN_RBRACE: return "}";
                case QUEX_TKN_NATURAL: return "positive integer";
                case QUEX_TKN_FLOAT: return "float";
                case QUEX_TKN_HEX: return "hex flag";
                case QUEX_TKN_UNKNOWN: return "bad token";
                default: return "unknown token (probably a bug?)";
        }
}

static std::string what (const quex::Token &t)
{
        switch (t.type_id()) {
                case QUEX_TKN_NATURAL: return "positive integer "+str(t);
                case QUEX_TKN_FLOAT: return "float "+str(t);
                case QUEX_TKN_HEX: return "hex flag "+str(t);
                case QUEX_TKN_UNKNOWN: return "bad token "+str(t);
                default: return what2(t.type_id());
        }
}


#ifdef __GNUC__
#define NORETURN __attribute__ ((noreturn))
#else
#define NORETURN
#endif


static void err (const Ogre::String &, quex::TColLexer *,
                 const quex::Token &, const std::string &)
NORETURN;

static void err (const Ogre::String &name, quex::TColLexer *qlex,
                 const quex::Token &t, const std::string &expected)
{
        std::stringstream msg;
        msg << "Parse error at " << where(qlex)
            << " - got \"" << what(t) << "\" "
            << "but expected " << expected << ".";
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE, msg.str(), name);
}


static void err (const Ogre::String &name, quex::TColLexer *qlex,
                 const std::string &msg)
{
        std::stringstream ss;
        ss << "Error at " << where(qlex) << ":  " << msg;
        OGRE_EXCEPT(Ogre::Exception::ERR_INVALID_STATE, ss.str(), name);
}


static void ensure_token (const Ogre::String &name, quex::TColLexer* qlex,
                          QUEX_TOKEN_ID_TYPE tid)
{
        quex::Token t;
        qlex->get_token(&t);
        if (t.type_id() != tid) {
                err(name,qlex,t,what2(tid));
        }
}


//quex::Token is not const because number() is not const (a bug in quex)
static int get_int_from_token (const Ogre::String &name,
                               quex::TColLexer* qlex,
                               quex::Token &t,
                               int num_vertexes)
{
        int v = t.number();
        if (v>=num_vertexes)
                err(name,qlex,t,"index of a vertex");
        return v;
}

static int parse_int (const Ogre::String &name,
                      quex::TColLexer* qlex,
                      int num_vertexes)
{
        quex::Token t; qlex->get_token(&t);
        if (t.type_id()!=QUEX_TKN_NATURAL) {
                err(name,qlex,t,"positive integer");
        }
        return get_int_from_token(name,qlex,t,num_vertexes);
}


static unsigned long get_ulong_from_hex_token (const quex::Token &t)
{
        const char *text = utf8(t);
        return strtoul(text,NULL,16);
}

static unsigned long parse_hex (const Ogre::String &name,
                                quex::TColLexer* qlex)
{
        quex::Token t; qlex->get_token(&t);
        if (t.type_id()!=QUEX_TKN_HEX) {
                err(name,qlex,t,"hexadecimal flags");
        }
        return get_ulong_from_hex_token(t);
}


static Ogre::Real get_real_from_token (const quex::Token &t)
{
        const char *text = utf8(t);
        return (Ogre::Real) strtod(text,NULL);
}

static Ogre::Real parse_real (const Ogre::String &name, quex::TColLexer* qlex)
{
        quex::Token t; qlex->get_token(&t);
        if (t.type_id()==QUEX_TKN_FLOAT) {
                return get_real_from_token(t);
        } else if (t.type_id()==QUEX_TKN_NATURAL) {
                return t.number();
        } else {
                err(name,qlex,t,"float");
        }
}

static Ogre::Real parse_positive_real (const Ogre::String &name,
                                       quex::TColLexer* qlex)
{
        Ogre::Real v;
        quex::Token t; qlex->get_token(&t);
        if (t.type_id()==QUEX_TKN_FLOAT) {
                v = get_real_from_token(t);
        } else if (t.type_id()==QUEX_TKN_NATURAL) {
                v = t.number();
        } else {
                err(name,qlex,t,"float");
        }
        if (v<0)
                err(name,qlex,t,"positive float");
        return v;
}

// pops ; or }, returning true or false respectively
bool more_to_come (const Ogre::String &name, quex::TColLexer* qlex)
{
        quex::Token t; qlex->get_token(&t);
        switch (t.type_id()) {
                case QUEX_TKN_SEMI: return true;
                case QUEX_TKN_RBRACE: return false;
                default:
                        err(name,qlex,t,"; or }");
        }
        return false; // never happens
}




static void parse_vertexes (const Ogre::String &name,
                            quex::TColLexer* qlex,
                            Vertexes &vertexes)
{
        ensure_token(name,qlex,QUEX_TKN_LBRACE);

        while (true) { 
                float x,y,z;
                quex::Token t; qlex->get_token(&t);
                switch (t.type_id()) {
                        case QUEX_TKN_FLOAT:
                        case QUEX_TKN_NATURAL:
                        x=get_real_from_token(t);
                        y=parse_real(name,qlex);
                        z=parse_real(name,qlex);
                        vertexes.push_back(btVector3(x,y,z));
                        if (!more_to_come(name,qlex)) break;
                        continue;

                        case QUEX_TKN_RBRACE:
                        break;

                        default:
                        err(name,qlex,t,"3 floats or }");
                }

                break;
        }
}

static void shrink_vertexes (Vertexes &vertexes, Ogre::Real distance)
{
        btAlignedObjectArray<btVector3> planes;
        btGeometryUtil::getPlaneEquationsFromVertices(vertexes, planes);
        int sz = planes.size();
        for (int i=0 ; i<sz ; ++i) {
                planes[i][3] += distance;
        }
        vertexes.clear();
        btGeometryUtil::getVerticesFromPlaneEquations(planes, vertexes);
}

template <typename T>
static inline T &vecnext (std::vector<T> &vec)
{
        size_t sz = vec.size();
        vec.resize(sz+1); // push a blank element
        return vec[sz]; // and return reference to it
}

static void parse_hull (const Ogre::String &name,
                         quex::TColLexer* qlex,
                         Hull &hull)
{
        ensure_token(name,qlex,QUEX_TKN_LBRACE);

        Ogre::Real shrink = 0;
        bool has_vertexes = false;
        hull.margin = 0.04;

        quex::Token t; 
        while (true) {
                qlex->get_token(&t);
                switch (t.type_id()) {
                        case QUEX_TKN_MARGIN:
                        hull.margin = parse_positive_real(name,qlex);
                        if (more_to_come(name, qlex)) continue;
                        break;

                        case QUEX_TKN_SHRINK:
                        if (has_vertexes) {
                                err(name,qlex,"Give shrink before vertexes!");
                        }
                        shrink = parse_positive_real(name,qlex);
                        if (more_to_come(name, qlex)) continue;
                        break;

                        case QUEX_TKN_VERTEXES:
                        if (has_vertexes) {
                                err(name,qlex,"Only one vertex list allowed.");
                        }
                        has_vertexes = true;
                        parse_vertexes(name, qlex, hull.vertexes);
                        if (shrink > 0) {
                                shrink_vertexes(hull.vertexes, shrink);
                        }
                        if (more_to_come(name, qlex)) continue;
                        break;

                        case QUEX_TKN_RBRACE:
                        break;

                        default:
                        err(name,qlex,t,"margin, shrink, vertexes or }");
                }
                break;
        }       

        if (!has_vertexes) {
                err(name,qlex,"No vertexes provided for hull.");
        }
}


static void parse_box (const Ogre::String &name,
                        quex::TColLexer* qlex,
                        Box &box)
{
        ensure_token(name,qlex,QUEX_TKN_LBRACE);
        box.margin = 0.04;
        box.qx = 0;
        box.qy = 0;
        box.qz = 0;
        box.qw = 1;
        quex::Token t; qlex->get_token(&t);
        switch (t.type_id()) {
                case QUEX_TKN_MARGIN:
                box.margin = parse_positive_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                ensure_token(name,qlex,QUEX_TKN_CENTRE);
                case QUEX_TKN_CENTRE:
                box.px=parse_real(name,qlex);
                box.py=parse_real(name,qlex);
                box.pz=parse_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                break;
                default:
                err(name,qlex,t,"margin or centre");
        }
        qlex->get_token(&t);
        switch (t.type_id()) {
                case QUEX_TKN_ORIENTATION:
                box.qw=parse_real(name,qlex);
                box.qx=parse_real(name,qlex);
                box.qy=parse_real(name,qlex);
                box.qz=parse_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                ensure_token(name,qlex,QUEX_TKN_DIMENSIONS);
                case QUEX_TKN_DIMENSIONS:
                box.dx=parse_real(name,qlex);
                box.dy=parse_real(name,qlex);
                box.dz=parse_real(name,qlex);
                break;
                default:
                err(name,qlex,t,"orientation or dimensions");
        }
        if (more_to_come(name,qlex))
                ensure_token(name,qlex,QUEX_TKN_RBRACE);
}


static void parse_cylinder (const Ogre::String &name,
                             quex::TColLexer* qlex,
                             Cylinder &cylinder)
{
        ensure_token(name,qlex,QUEX_TKN_LBRACE);
        cylinder.margin = 0.04;
        cylinder.qx = 0;
        cylinder.qy = 0;
        cylinder.qz = 0;
        cylinder.qw = 1;
        quex::Token t; qlex->get_token(&t);
        switch (t.type_id()) {
                case QUEX_TKN_MARGIN:
                cylinder.margin = parse_positive_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                ensure_token(name,qlex,QUEX_TKN_CENTRE);
                case QUEX_TKN_CENTRE:
                cylinder.px=parse_real(name,qlex);
                cylinder.py=parse_real(name,qlex);
                cylinder.pz=parse_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                break;
                default:
                err(name,qlex,t,"margin or centre");
        }
        qlex->get_token(&t);
        switch (t.type_id()) {
                case QUEX_TKN_ORIENTATION:
                cylinder.qw=parse_real(name,qlex);
                cylinder.qx=parse_real(name,qlex);
                cylinder.qy=parse_real(name,qlex);
                cylinder.qz=parse_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                ensure_token(name,qlex,QUEX_TKN_DIMENSIONS);
                case QUEX_TKN_DIMENSIONS:
                cylinder.dx=parse_real(name,qlex);
                cylinder.dy=parse_real(name,qlex);
                cylinder.dz=parse_real(name,qlex);
                break;
                default:
                err(name,qlex,t,"orientation or dimensions");
        }
        if (more_to_come(name,qlex))
                ensure_token(name,qlex,QUEX_TKN_RBRACE);
}


static void parse_cone (const Ogre::String &name,
                         quex::TColLexer* qlex,
                         Cone &cone)
{
        ensure_token(name,qlex,QUEX_TKN_LBRACE);
        cone.margin = 0.04;
        cone.qx = 0;
        cone.qy = 0;
        cone.qz = 0;
        cone.qw = 1;
        quex::Token t; qlex->get_token(&t);
        switch (t.type_id()) {
                case QUEX_TKN_MARGIN:
                cone.margin = parse_positive_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                ensure_token(name,qlex,QUEX_TKN_CENTRE);
                case QUEX_TKN_CENTRE:
                cone.px=parse_real(name,qlex);
                cone.py=parse_real(name,qlex);
                cone.pz=parse_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                break;
                default:
                err(name,qlex,t,"margin or centre");
        }
        qlex->get_token(&t);
        switch (t.type_id()) {
                case QUEX_TKN_ORIENTATION:
                cone.qw=parse_real(name,qlex);
                cone.qx=parse_real(name,qlex);
                cone.qy=parse_real(name,qlex);
                cone.qz=parse_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                ensure_token(name,qlex,QUEX_TKN_RADIUS);
                case QUEX_TKN_RADIUS:
                cone.radius=parse_real(name,qlex);
                ensure_token(name,qlex,QUEX_TKN_SEMI);
                ensure_token(name,qlex,QUEX_TKN_HEIGHT);
                cone.height=parse_real(name,qlex);
                break;
                default:
                err(name,qlex,t,"orientation or radius");
        }
        if (more_to_come(name,qlex))
                ensure_token(name,qlex,QUEX_TKN_RBRACE);
}


static void parse_plane (const Ogre::String &name,
                         quex::TColLexer* qlex,
                         Plane &plane)
{
        ensure_token(name,qlex,QUEX_TKN_LBRACE);
        ensure_token(name,qlex,QUEX_TKN_NORMAL);
        plane.nx = parse_real(name,qlex);
        plane.ny = parse_real(name,qlex);
        plane.nz = parse_real(name,qlex);
        ensure_token(name,qlex,QUEX_TKN_SEMI);
        ensure_token(name,qlex,QUEX_TKN_DISTANCE);
        plane.d = parse_real(name,qlex);
        if (more_to_come(name,qlex))
                ensure_token(name,qlex,QUEX_TKN_RBRACE);
}


static void parse_sphere (const Ogre::String &name,
                          quex::TColLexer* qlex,
                          Sphere &sphere)
{
        ensure_token(name,qlex,QUEX_TKN_LBRACE);
        ensure_token(name,qlex,QUEX_TKN_CENTRE);
        sphere.px = parse_real(name,qlex);
        sphere.py = parse_real(name,qlex);
        sphere.pz = parse_real(name,qlex);
        ensure_token(name,qlex,QUEX_TKN_SEMI);
        ensure_token(name,qlex,QUEX_TKN_RADIUS);
        sphere.radius = parse_real(name,qlex);
        if (more_to_come(name,qlex))
                ensure_token(name,qlex,QUEX_TKN_RBRACE);
}


static void parse_compound_shape (const Ogre::String &name,
                           quex::TColLexer* qlex,
                           Compound &compound)
{

        ensure_token(name,qlex,QUEX_TKN_LBRACE);

        while (true) {
                // define all these upfront since we're using a switch
                quex::Token t; qlex->get_token(&t);
                switch (t.type_id()) {
                        case QUEX_TKN_COMPOUND: ///////////////////////////////
                        parse_compound_shape(name,
                                              qlex,
                                              vecnext(compound.compounds)); 
                        continue;

                        case QUEX_TKN_HULL: ///////////////////////////////////
                        parse_hull(name,
                                    qlex,
                                    vecnext(compound.hulls)); 
                        continue;

                        case QUEX_TKN_BOX: ////////////////////////////////////
                        parse_box(name,qlex,vecnext(compound.boxes));
                        continue;

                        case QUEX_TKN_CYLINDER: //////////////////////////////
                        parse_cylinder(name,qlex,vecnext(compound.cylinders));
                        continue;

                        case QUEX_TKN_CONE: ///////////////////////////////////
                        parse_cone(name,qlex,vecnext(compound.cones));
                        continue;

                        case QUEX_TKN_PLANE: //////////////////////////////////
                        parse_plane(name,qlex,vecnext(compound.planes));
                        continue;

                        case QUEX_TKN_SPHERE: /////////////////////////////////
                        parse_sphere(name,qlex,vecnext(compound.spheres));
                        continue;

                        case QUEX_TKN_RBRACE: /////////////////////////////////
                        break;

                        default:
                                err(name,qlex,t,"compound, box, sphere, hull");
                }

                break;
        }
}


static void parse_faces (const Ogre::String &name,
                         quex::TColLexer* qlex,
                         size_t num_vertexes,
                         Faces &faces)
{
        ensure_token(name,qlex,QUEX_TKN_LBRACE);
        int v1, v2, v3;
        unsigned int flags;

        while (true) {
                quex::Token t; qlex->get_token(&t);
                switch (t.type_id()) {
                        case QUEX_TKN_NATURAL:
                        v1 = get_int_from_token(name,qlex,t,num_vertexes);
                        v2 = parse_int(name,qlex,num_vertexes);
                        v3 = parse_int(name,qlex,num_vertexes);
                        flags = parse_hex(name,qlex);
                        faces.push_back(Face(v1,v2,v3,flags));
                        if (!more_to_come(name,qlex)) break;
                        continue;

                        case QUEX_TKN_RBRACE:
                        break;

                        default:
                        err(name,qlex,t,"positive integer or }");
                }

                break;
        }

}

static void parse_static_trimesh_shape (const Ogre::String &name,
                                        quex::TColLexer* qlex,
                                        TriMesh &triMesh)
{
        ensure_token(name,qlex,QUEX_TKN_LBRACE);

        ensure_token(name,qlex,QUEX_TKN_VERTEXES);

        parse_vertexes(name,qlex,triMesh.vertexes);

        ensure_token(name,qlex,QUEX_TKN_FACES);

        parse_faces(name,qlex,triMesh.vertexes.size(),triMesh.faces);

        ensure_token(name,qlex,QUEX_TKN_RBRACE);

}


void parse_tcol_1_0 (const Ogre::String &name,
                     quex::TColLexer* qlex,
                     TColFile &file)
{
        ensure_token(name,qlex,QUEX_TKN_TCOL);

        ensure_token(name,qlex,QUEX_TKN_ATTRIBUTES);
        ensure_token(name,qlex,QUEX_TKN_LBRACE);

        enum TriBool { UNKNOWN, YES, NO };

        TriBool is_static = UNKNOWN;
        bool have_inertia = false;
        file.inertia_x = 0;
        file.inertia_y = 0;
        file.inertia_z = 0;
        bool have_friction = false;
        file.friction = 0.5;
        bool have_restitution = false;
        file.restitution = 0;
        bool have_linear_damping = false;
        file.linearDamping = 0;
        bool have_angular_damping = false;
        file.angularDamping = 0.5;
        bool have_linear_sleep_threshold = false;
        file.linearSleepThreshold = 1;
        bool have_angular_sleep_threshold = false;
        file.angularSleepThreshold = 0.8;
        bool have_ccd_motion_threshold = false;
        file.ccdMotionThreshold = 0.0;
        bool have_ccd_swept_sphere_radius = false;
        file.ccdSweptSphereRadius = 0.0;

        do {
                quex::Token t; qlex->get_token(&t);
                switch (t.type_id()) {

                        case QUEX_TKN_STATIC:
                        if (is_static==NO)
                                err(name,qlex,"If static, do not give mass");
                        if (is_static==YES)
                                err(name,qlex,"Already have static");
                        is_static = YES;
                        file.mass = 0;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_MASS:
                        if (is_static==YES)
                                err(name,qlex,"If static, do not give mass");
                        if (is_static==NO)
                                err(name,qlex,"Already have mass");
                        file.mass = parse_positive_real(name,qlex);
                        is_static = NO;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_INERTIA:
                        if (have_inertia)
                                err(name,qlex,"Already have inertia");
                        file.inertia_x = parse_real(name,qlex);
                        file.inertia_y = parse_real(name,qlex);
                        file.inertia_z = parse_real(name,qlex);
                        have_inertia = true;
                        if (more_to_come(name,qlex)) continue; break;
                
                        case QUEX_TKN_FRICTION:
                        if (have_friction)
                                err(name,qlex,"Already have friction");
                        file.friction = parse_positive_real(name,qlex);
                        have_friction = true;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_RESTITUTION:
                        if (have_restitution)
                                err(name,qlex,"Already have restitution");
                        file.restitution = parse_positive_real(name,qlex);
                        have_restitution = true;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_LINEAR_DAMPING:
                        if (have_linear_damping)
                                err(name,qlex,"Already have linear_damping");
                        file.linearDamping = parse_positive_real(name,qlex);
                        have_linear_damping = true;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_ANGULAR_DAMPING:
                        if (have_angular_damping)
                                err(name,qlex,"Already have angular_damping");
                        file.angularDamping = parse_positive_real(name,qlex);
                        have_angular_damping = true;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_LINEAR_SLEEP_THRESHOLD:
                        if (have_linear_sleep_threshold)
                                err(name,qlex,
                                    "Already have linear_sleep_threshold");
                        file.linearSleepThreshold = parse_real(name,qlex);
                        have_linear_sleep_threshold = true;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_ANGULAR_SLEEP_THRESHOLD:
                        if (have_angular_sleep_threshold)
                               err(name,qlex,
                                   "Already have angular_sleep_threshold");
                        file.angularSleepThreshold = parse_real(name,qlex);
                        have_angular_sleep_threshold = true;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_CCD_MOTION_THRESHOLD:
                        if (have_ccd_motion_threshold)
                               err(name,qlex,
                                   "Already have ccd_motion_threshold");
                        file.ccdMotionThreshold = parse_real(name,qlex);
                        have_ccd_motion_threshold = true;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_CCD_SWEPT_SPHERE_RADIUS:
                        if (have_ccd_swept_sphere_radius)
                               err(name,qlex,
                                   "Already have ccd_swept_sphere_radius");
                        file.ccdSweptSphereRadius = parse_real(name,qlex);
                        have_ccd_swept_sphere_radius = true;
                        if (more_to_come(name,qlex)) continue; break;

                        case QUEX_TKN_RBRACE: break; 

                        default:
                        err(name,qlex,t,"mass, friction, etc or }");

                }

                break;
        } while (true);

        if (is_static==UNKNOWN)
                err(name,qlex,"Need either static or mass");


        file.usingTriMesh = false;
        file.usingCompound = false;

        quex::Token t; qlex->get_token(&t);
        switch (t.type_id()) {
                case QUEX_TKN_COMPOUND:
                file.usingCompound = true;
                parse_compound_shape(name,qlex,file.compound);
                qlex->get_token(&t);
                if (t.type_id()==QUEX_TKN_TERMINATION) break;
                if (t.type_id()!=QUEX_TKN_TRIMESH)
                        err(name,qlex,t,"trimesh or EOF");

                case QUEX_TKN_TRIMESH:
                if (is_static==YES) {
                        file.usingTriMesh = true;
                        parse_static_trimesh_shape(name,qlex,file.triMesh);
                } else {
                        err(name,qlex,"dynamic trimesh not implemented yet");
                }
                ensure_token(name,qlex,QUEX_TKN_TERMINATION);
                break;

                default: err(name,qlex,t,"compound or trimesh");
        }


}

void pretty_print_compound (std::ostream &o, Compound &c, const std::string &in)
{
        o << in << "compound {\n";

        for (size_t i=0 ; i<c.hulls.size() ; ++i) {
                Hull &h = c.hulls[i];
                o<<in<<"\t"<<"hull {\n";
                if (ffar(h.margin,0.04)) {
                        o<<in<<"\t\t"<<"margin "<<h.margin<<";\n";
                }
                o<<in<<"\t\t"<<"vertexes {\n";
                for (int j=0 ; j<h.vertexes.size() ; ++j) {
                        btVector3 &v = h.vertexes[j];
                        o<<in<<"\t\t\t"<<v.x()<<" "<<v.y()
                                       <<" "<<v.z()<<";"<<"\n";
                }
                o<<in<<"\t\t"<<"}\n";
                o<<in<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<c.boxes.size() ; ++i) {
                Box &b = c.boxes[i];
                o<<in<<"\t"<<"box {\n";
                if (ffar(b.margin,0.04)) {
                        o<<in<<"\t\t"<<"margin "<<b.margin<<";\n";
                }
                o<<in<<"\t\t"<<"centre "<<b.px
                             <<" "<<b.py<<" "<<b.pz<<";\n";
                if (ffar(b.qw,1) && ffar(b.qx,0) &&
                    ffar(b.qy,0) && ffar(b.qz,0)) {
                        o<<in<<"\t\t"<<"orientation "<<b.qw<<" "<<b.qx
                         <<" "<<b.qy<<" "<<b.qz<<";\n";
                }
                o<<in<<"\t\t"<<"dimensions "<<b.dx<<" "<<b.dy
                                            <<" "<<b.dz<<";\n";
                o<<in<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<c.cylinders.size() ; ++i) {
                Cylinder &cyl = c.cylinders[i];
                o<<in<<"\t"<<"cylinder {\n";
                if (ffar(cyl.margin,0.04)) {
                        o<<in<<"\t\t"<<"margin "<<cyl.margin<<";\n";
                }
                o<<in<<"\t\t"<<"centre "<<cyl.px
                             <<" "<<cyl.py<<" "<<cyl.pz<<";\n";
                if (ffar(cyl.qw,1) && ffar(cyl.qx,0) &&
                    ffar(cyl.qy,0) && ffar(cyl.qz,0)) {
                        o<<in<<"\t\t"<<"orientation "<<cyl.qw<<" "<<cyl.qx
                         <<" "<<cyl.qy<<" "<<cyl.qz<<";\n";
                }
                o<<in<<"\t\t"<<"dimensions "<<cyl.dx<<" "<<cyl.dy
                                            <<" "<<cyl.dz<<";\n";
                o<<in<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<c.cones.size() ; ++i) {
                Cone &cone = c.cones[i];
                o<<in<<"\t"<<"cone {\n";
                if (ffar(cone.margin,0.04)) {
                        o<<in<<"\t\t"<<"margin "<<cone.margin<<";\n";
                }
                o<<in<<"\t\t"<<"centre "<<cone.px
                             <<" "<<cone.py<<" "<<cone.pz<<";\n";
                if (ffar(cone.qw,1) && ffar(cone.qx,0) &&
                    ffar(cone.qy,0) && ffar(cone.qz,0)) {
                        o<<in<<"\t\t"<<"orientation "<<cone.qw<<" "<<cone.qx
                         <<" "<<cone.qy<<" "<<cone.qz<<";\n";
                }
                o<<in<<"\t\t"<<"radius "<<cone.radius<<";\n";
                o<<in<<"\t\t"<<"height "<<cone.height<<";\n";
                o<<in<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<c.planes.size() ; ++i) {
                Plane &p = c.planes[i];
                o<<in<<"\t"<<"plane {\n";
                o<<in<<"\t\t"<<"normal "<<p.nx<<" "<<p.ny<<" "<<p.nz<<";\n";
                o<<in<<"\t\t"<<"distance "<<p.d<<";\n";
                o<<in<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<c.spheres.size() ; ++i) {
                Sphere &s = c.spheres[i];
                o<<in<<"\t"<<"sphere {\n";
                o<<in<<"\t\t"<<"centre "<<s.px<<" "<<s.py<<" "<<s.pz<<";\n";
                o<<in<<"\t\t"<<"radius "<<s.radius<<";\n";
                o<<in<<"\t"<<"}\n";
        }

        for (size_t i=0 ; i<c.compounds.size() ; ++i) {
                Compound &c2 = c.compounds[i];
                pretty_print_compound (o,c2,in+"        ");
        }

        o << in << "}\n";
}


void pretty_print_tcol (std::ostream &os, TColFile &f)
{
        std::stringstream o;
        o << std::fixed; // use fixed point (no exponents)

        o << "TCOL1.0\n\n"
          << "attributes {\n";
        if (f.mass==0) {
                o << "\tstatic;\n";
        } else {
                o << "\tmass " << f.mass << ";\n";
        }
/*
        o << "\tinertia " << f.inertia_x << " "
                          << f.inertia_y << " "
                          << f.inertia_z << ";\n";
*/
        if (ffar(f.friction,0.5))
                o << "\tfriction " << f.friction << ";\n";
        if (ffar(f.restitution,0))
                o << "\trestitution " << f.restitution << ";\n";
        if (ffar(f.linearDamping,0))
                o << "\tlinear_damping " << f.linearDamping << ";\n";
        if (ffar(f.angularDamping,0.5))
                o << "\tangular_damping " << f.angularDamping << ";\n";
        if (ffar(f.linearSleepThreshold,1))
            o << "\tlinear_sleep_threshold " << f.linearSleepThreshold << ";\n";
        if (ffar(f.angularSleepThreshold,0.8))
          o << "\tangular_sleep_threshold " << f.angularSleepThreshold << ";\n";
        if (ffar(f.ccdMotionThreshold,0))
                o << "\tccd_motion_threshold " << f.ccdMotionThreshold << ";\n";
        if (ffar(f.ccdSweptSphereRadius,0))
           o << "\tccd_swept_sphere_radius " << f.ccdSweptSphereRadius << ";\n";
        o << "}\n\n";

        if (f.usingCompound) {
                pretty_print_compound(o,f.compound,"");
        }

        if (f.usingTriMesh) {
                o << "trimesh {\n";
                o << "\tvertexes {\n";
                for (int i=0 ; i<f.triMesh.vertexes.size() ; ++i) {
                        btVector3 &v = f.triMesh.vertexes[i];
                        o<<"\t\t"<<v.x()<<" "<<v.y()<<" "<<v.z()<<";"<<"\n";
                }
                o << "\t}\n";
                o << "\tfaces {\n";
                for (size_t i=0 ; i<f.triMesh.faces.size() ; ++i) {
                        Face &face = f.triMesh.faces[i];
                        o<<"\t\t"<<face.v1<<" "<<face.v2<<" "<<face.v3<<" "
                         <<std::hex<<"0x"<<face.flag<<std::dec<<";"<<"\n";
                }
                o << "\t}\n";
                o << "}\n";
        }
        os << o.str();
}

// vim: shiftwidth=8:tabstop=8:expandtab
