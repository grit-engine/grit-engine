/* Copyright (c) The Grit Game Engine authors 2016
 *
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

#include <BulletCollision/CollisionShapes/btTriangleShape.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>

#include "../grit_object.h"
#include "../main.h"
#include <centralised_log.h>
#include "../option.h"
#include "../grit_lua_util.h"

#include "physics_world.h"
#include "lua_wrappers_physics.h"

static btDefaultCollisionConfiguration *col_conf;
static btCollisionDispatcher *col_disp;

static btBroadphaseInterface *broadphase;

static btConstraintSolver *con_solver;

static DynamicsWorld *world;

static btVector3 gravity; // cached in here in vector form

// {{{ get access to some protected members in btDiscreteDynamicsWorld

class DynamicsWorld : public btDiscreteDynamicsWorld {
    public:
    DynamicsWorld (btCollisionDispatcher *colDisp,
               btBroadphaseInterface *broadphase,
               btConstraintSolver *conSolver,
               btCollisionConfiguration *colConf)
          : btDiscreteDynamicsWorld(colDisp,broadphase,conSolver,colConf)
    { }

    // used to be protected
    void saveKinematicState (float step_size)
    { saveKinematicState(step_size); }

    // used to be protected
    void internalStepSimulation (float step_size)
    { internalSingleStepSimulation(step_size); }

};

// }}}


// {{{ PHYSICS OPTION

static PhysicsBoolOption option_keys_bool[] = {
    PHYSICS_AUTOUPDATE,

    PHYSICS_GIMPACT_ONE_WAY_MESH_HACK,
    PHYSICS_BUMPY_TRIANGLE_MESH_HACK,
    PHYSICS_USE_TRIANGLE_EDGE_INFO,
    PHYSICS_VERBOSE_CONTACTS,
    PHYSICS_VERBOSE_CASTS,
    PHYSICS_ERROR_CONTACTS,
    PHYSICS_ERROR_CASTS,
    PHYSICS_SOLVER_SPLIT_IMPULSE,
    PHYSICS_SOLVER_RANDOMISE_ORDER,
    PHYSICS_SOLVER_FRICTION_SEPARATE,
    PHYSICS_SOLVER_USE_WARM_STARTING,
    PHYSICS_SOLVER_CACHE_FRIENDLY,

    PHYSICS_DEBUG_WIREFRAME,
    PHYSICS_DEBUG_AABB,
    PHYSICS_DEBUG_FEATURES_TEXT,
    PHYSICS_DEBUG_CONTACT_POINTS,
    PHYSICS_DEBUG_CONSTRAINTS,
    PHYSICS_DEBUG_CONSTRAINTS_LIMITS,

    PHYSICS_DEBUG_NO_DEACTIVATION,
    PHYSICS_DEBUG_NO_HELP_TEXT,
    PHYSICS_DEBUG_DRAW_TEXT,
    PHYSICS_DEBUG_PROFILE_TIMINGS,
    PHYSICS_DEBUG_ENABLE_SAT_COMPARISON,
    PHYSICS_DEBUG_DISABLE_BULLET_LCP,
    PHYSICS_DEBUG_ENABLE_CCD,
    PHYSICS_DEBUG_FAST_WIREFRAME
};

static PhysicsIntOption option_keys_int[] = {
    PHYSICS_SOLVER_ITERATIONS
};

static PhysicsFloatOption option_keys_float[] = {
    PHYSICS_GRAVITY_X,
    PHYSICS_GRAVITY_Y,
    PHYSICS_GRAVITY_Z,
    PHYSICS_STEP_SIZE,
    PHYSICS_CONTACT_BREAKING_THRESHOLD,
    PHYSICS_DEACTIVATION_TIME,
    PHYSICS_SOLVER_DAMPING,
    PHYSICS_SOLVER_ERP,
    PHYSICS_SOLVER_ERP2,
    PHYSICS_SOLVER_SPLIT_IMPULSE_THRESHOLD,
    PHYSICS_SOLVER_LINEAR_SLOP,
    PHYSICS_SOLVER_WARM_STARTING_FACTOR
};

static std::map<PhysicsBoolOption,bool> options_bool;
static std::map<PhysicsIntOption,int> options_int;
static std::map<PhysicsFloatOption,float> options_float;
static std::map<PhysicsBoolOption,bool> new_options_bool;
static std::map<PhysicsIntOption,int> new_options_int;
static std::map<PhysicsFloatOption,float> new_options_float;

static std::map<PhysicsBoolOption,ValidOption<bool>*> valid_option_bool;
static std::map<PhysicsIntOption,ValidOption<int>*> valid_option_int;
static std::map<PhysicsFloatOption,ValidOption<float>*> valid_option_float;

static void valid_option (PhysicsBoolOption o, ValidOption<bool> *v) { valid_option_bool[o] = v; }
static void valid_option (PhysicsIntOption o, ValidOption<int> *v) { valid_option_int[o] = v; }
static void valid_option (PhysicsFloatOption o, ValidOption<float> *v) { valid_option_float[o] = v; }

static bool truefalse_[] = { false, true };
static ValidOptionList<bool,bool[2]> *truefalse = new ValidOptionList<bool,bool[2]>(truefalse_);

std::string physics_option_to_string (PhysicsBoolOption o)
{
    switch (o) {
        case PHYSICS_AUTOUPDATE: return "GIMPACT_AUTOUPDATE";
        case PHYSICS_GIMPACT_ONE_WAY_MESH_HACK: return "GIMPACT_ONE_WAY_MESH_HACK";
        case PHYSICS_BUMPY_TRIANGLE_MESH_HACK: return "BUMPY_TRIANGLE_MESH_HACK";
        case PHYSICS_USE_TRIANGLE_EDGE_INFO: return "USE_TRIANGLE_EDGE_INFO";
        case PHYSICS_VERBOSE_CONTACTS: return "VERBOSE_CONTACTS";
        case PHYSICS_VERBOSE_CASTS: return "VERBOSE_CASTS";
        case PHYSICS_ERROR_CONTACTS: return "ERROR_CONTACTS";
        case PHYSICS_ERROR_CASTS: return "ERROR_CASTS";
        case PHYSICS_SOLVER_SPLIT_IMPULSE: return "SOLVER_SPLIT_IMPULSE";
        case PHYSICS_SOLVER_RANDOMISE_ORDER: return "SOLVER_RANDOMISE_ORDER";
        case PHYSICS_SOLVER_FRICTION_SEPARATE: return "SOLVER_FRICTION_SEPARATE";
        case PHYSICS_SOLVER_USE_WARM_STARTING: return "SOLVER_USE_WARM_STARTING";
        case PHYSICS_SOLVER_CACHE_FRIENDLY: return "SOLVER_CACHE_FRIENDLY";
        case PHYSICS_DEBUG_WIREFRAME: return "DEBUG_WIREFRAME";
        case PHYSICS_DEBUG_AABB: return "DEBUG_AABB";
        case PHYSICS_DEBUG_FEATURES_TEXT: return "DEBUG_FEATURES_TEXT";
        case PHYSICS_DEBUG_CONTACT_POINTS: return "DEBUG_CONTACT_POINTS";
        case PHYSICS_DEBUG_CONSTRAINTS: return "DEBUG_CONSTRAINTS";
        case PHYSICS_DEBUG_CONSTRAINTS_LIMITS: return "DEBUG_CONSTRAINTS_LIMITS";
        case PHYSICS_DEBUG_NO_DEACTIVATION: return "DEBUG_NO_DEACTIVATION";
        case PHYSICS_DEBUG_NO_HELP_TEXT: return "DEBUG_NO_HELP_TEXT";
        case PHYSICS_DEBUG_DRAW_TEXT: return "DEBUG_DRAW_TEXT";
        case PHYSICS_DEBUG_PROFILE_TIMINGS: return "DEBUG_PROFILE_TIMINGS";
        case PHYSICS_DEBUG_ENABLE_SAT_COMPARISON: return "DEBUG_ENABLE_SAT_COMPARISON";
        case PHYSICS_DEBUG_DISABLE_BULLET_LCP: return "DEBUG_DISABLE_BULLET_LCP";
        case PHYSICS_DEBUG_ENABLE_CCD: return "DEBUG_ENABLE_CCD";
        case PHYSICS_DEBUG_FAST_WIREFRAME: return "DEBUG_FAST_WIREFRAME";
    }
    return "UNKNOWN_BOOL_OPTION";
}
std::string physics_option_to_string (PhysicsIntOption o)
{
    switch (o) {
        case PHYSICS_SOLVER_ITERATIONS: return "SOLVER_ITERATIONS";
    }
    return "UNKNOWN_INT_OPTION";
}
std::string physics_option_to_string (PhysicsFloatOption o)
{
    switch (o) {
        case PHYSICS_GRAVITY_X: return "GRAVITY_X";
        case PHYSICS_GRAVITY_Y: return "GRAVITY_Y";
        case PHYSICS_GRAVITY_Z: return "GRAVITY_Z";
        case PHYSICS_STEP_SIZE: return "STEP_SIZE";
        case PHYSICS_CONTACT_BREAKING_THRESHOLD: return "CONTACT_BREAKING_THRESHOLD";
        case PHYSICS_DEACTIVATION_TIME: return "DEACTIVATION_TIME";
        case PHYSICS_SOLVER_DAMPING: return "SOLVER_DAMPING";
        case PHYSICS_SOLVER_ERP: return "SOLVER_ERP";
        case PHYSICS_SOLVER_ERP2: return "SOLVER_ERP2";
        case PHYSICS_SOLVER_SPLIT_IMPULSE_THRESHOLD: return "SOLVER_SPLIT_IMPULSE_THRESHOLD";
        case PHYSICS_SOLVER_LINEAR_SLOP: return "SOLVER_LINEAR_SLOP";
        case PHYSICS_SOLVER_WARM_STARTING_FACTOR: return "WARM_STARTING_FACTOR";
    }
    return "UNKNOWN_FLOAT_OPTION";
}

// set's t to either 0,1,2 and fills in the approriate argument
void physics_option_from_string (const std::string &s,
                                 int &t,
                                 PhysicsBoolOption &o0,
                                 PhysicsIntOption &o1,
                                 PhysicsFloatOption &o2)
{
    if (s=="AUTOUPDATE") { t = 0; o0 = PHYSICS_AUTOUPDATE; }

    else if (s=="GIMPACT_ONE_WAY_MESH_HACK") { t = 0 ; o0 = PHYSICS_GIMPACT_ONE_WAY_MESH_HACK; }
    else if (s=="BUMPY_TRIANGLE_MESH_HACK") { t = 0 ; o0 = PHYSICS_BUMPY_TRIANGLE_MESH_HACK; }
    else if (s=="USE_TRIANGLE_EDGE_INFO") { t = 0 ; o0 = PHYSICS_USE_TRIANGLE_EDGE_INFO; }
    else if (s=="VERBOSE_CONTACTS") { t = 0 ; o0 = PHYSICS_VERBOSE_CONTACTS; }
    else if (s=="VERBOSE_CASTS") { t = 0 ; o0 = PHYSICS_VERBOSE_CASTS; }
    else if (s=="ERROR_CONTACTS") { t = 0 ; o0 = PHYSICS_ERROR_CONTACTS; }
    else if (s=="ERROR_CASTS") { t = 0 ; o0 = PHYSICS_ERROR_CASTS; }
    else if (s=="SOLVER_SPLIT_IMPULSE") { t = 0 ; o0 = PHYSICS_SOLVER_SPLIT_IMPULSE; }
    else if (s=="SOLVER_RANDOMISE_ORDER") { t = 0 ; o0 = PHYSICS_SOLVER_RANDOMISE_ORDER; }
    else if (s=="SOLVER_FRICTION_SEPARATE") { t = 0 ; o0 = PHYSICS_SOLVER_FRICTION_SEPARATE; }
    else if (s=="SOLVER_USE_WARM_STARTING") { t = 0 ; o0 = PHYSICS_SOLVER_USE_WARM_STARTING; }
    else if (s=="SOLVER_CACHE_FRIENDLY") { t = 0 ; o0 = PHYSICS_SOLVER_CACHE_FRIENDLY; }
    else if (s=="DEBUG_WIREFRAME") { t = 0 ; o0 = PHYSICS_DEBUG_WIREFRAME; }
    else if (s=="DEBUG_AABB") { t = 0 ; o0 = PHYSICS_DEBUG_AABB; }
    else if (s=="DEBUG_FEATURES_TEXT") { t = 0 ; o0 = PHYSICS_DEBUG_FEATURES_TEXT; }
    else if (s=="DEBUG_CONTACT_POINTS") { t = 0 ; o0 = PHYSICS_DEBUG_CONTACT_POINTS; }
    else if (s=="DEBUG_CONSTRAINTS") { t = 0 ; o0 = PHYSICS_DEBUG_CONSTRAINTS; }
    else if (s=="DEBUG_CONSTRAINTS_LIMITS") { t = 0 ; o0 = PHYSICS_DEBUG_CONSTRAINTS_LIMITS; }
    else if (s=="DEBUG_NO_DEACTIVATION") { t = 0 ; o0 = PHYSICS_DEBUG_NO_DEACTIVATION; }
    else if (s=="DEBUG_NO_HELP_TEXT") { t = 0 ; o0 = PHYSICS_DEBUG_NO_HELP_TEXT; }
    else if (s=="DEBUG_DRAW_TEXT") { t = 0 ; o0 = PHYSICS_DEBUG_DRAW_TEXT; }
    else if (s=="DEBUG_PROFILE_TIMINGS") { t = 0 ; o0 = PHYSICS_DEBUG_PROFILE_TIMINGS; }
    else if (s=="DEBUG_ENABLE_SAT_COMPARISON") { t = 0 ; o0 = PHYSICS_DEBUG_ENABLE_SAT_COMPARISON; }
    else if (s=="DEBUG_DISABLE_BULLET_LCP") { t = 0 ; o0 = PHYSICS_DEBUG_DISABLE_BULLET_LCP; }
    else if (s=="DEBUG_ENABLE_CCD") { t = 0 ; o0 = PHYSICS_DEBUG_ENABLE_CCD; }
    else if (s=="DEBUG_FAST_WIREFRAME") { t = 0 ; o0 = PHYSICS_DEBUG_FAST_WIREFRAME; }

    else if (s=="SOLVER_ITERATIONS") { t = 1 ; o1 = PHYSICS_SOLVER_ITERATIONS; }

    else if (s=="GRAVITY_X") { t = 2 ; o2 = PHYSICS_GRAVITY_X; }
    else if (s=="GRAVITY_Y") { t = 2 ; o2 = PHYSICS_GRAVITY_Y; }
    else if (s=="GRAVITY_Z") { t = 2 ; o2 = PHYSICS_GRAVITY_Z; }
    else if (s=="STEP_SIZE") { t = 2 ; o2 = PHYSICS_STEP_SIZE; }
    else if (s=="CONTACT_BREAKING_THRESHOLD") { t = 2 ; o2 = PHYSICS_CONTACT_BREAKING_THRESHOLD; }
    else if (s=="DEACTIVATION_TIME") { t = 2 ; o2 = PHYSICS_DEACTIVATION_TIME; }
    else if (s=="SOLVER_DAMPING") { t = 2 ; o2 = PHYSICS_SOLVER_DAMPING; }
    else if (s=="SOLVER_ERP") { t = 2 ; o2 = PHYSICS_SOLVER_ERP; }
    else if (s=="SOLVER_ERP2") { t = 2 ; o2 = PHYSICS_SOLVER_ERP2; }
    else if (s=="SOLVER_SPLIT_IMPULSE_THRESHOLD") { t = 2 ; o2 = PHYSICS_SOLVER_SPLIT_IMPULSE_THRESHOLD; }
    else if (s=="SOLVER_LINEAR_SLOP") { t = 2 ; o2 = PHYSICS_SOLVER_LINEAR_SLOP; }
    else if (s=="WARM_STARTING_FACTOR") { t = 2 ; o2 = PHYSICS_SOLVER_WARM_STARTING_FACTOR; }

    else t = -1;
}





static void options_update (bool flush)
{
    bool reset_gravity = flush;
    bool reset_solver_mode = flush;
    bool reset_debug_drawer = flush;

    for (unsigned i=0 ; i<sizeof(option_keys_bool)/sizeof(*option_keys_bool) ; ++i) {
        PhysicsBoolOption o = option_keys_bool[i];
        bool v_old = options_bool[o];
        bool v_new = new_options_bool[o];
        if (v_old == v_new) continue;
        switch (o) {
            case PHYSICS_AUTOUPDATE: break;

            case PHYSICS_GIMPACT_ONE_WAY_MESH_HACK:
            case PHYSICS_BUMPY_TRIANGLE_MESH_HACK:
            case PHYSICS_USE_TRIANGLE_EDGE_INFO:
            case PHYSICS_VERBOSE_CONTACTS:
            case PHYSICS_VERBOSE_CASTS:
            case PHYSICS_ERROR_CONTACTS:
            case PHYSICS_ERROR_CASTS:
            break;

            case PHYSICS_SOLVER_SPLIT_IMPULSE:
            world->getSolverInfo().m_splitImpulse = v_new;
            break;

            case PHYSICS_SOLVER_RANDOMISE_ORDER:
            case PHYSICS_SOLVER_FRICTION_SEPARATE:
            case PHYSICS_SOLVER_USE_WARM_STARTING:
            case PHYSICS_SOLVER_CACHE_FRIENDLY:
            reset_solver_mode = true;
            break;

            case PHYSICS_DEBUG_WIREFRAME:
            case PHYSICS_DEBUG_AABB:
            case PHYSICS_DEBUG_FEATURES_TEXT:
            case PHYSICS_DEBUG_CONTACT_POINTS:
            case PHYSICS_DEBUG_CONSTRAINTS:
            case PHYSICS_DEBUG_CONSTRAINTS_LIMITS:
            case PHYSICS_DEBUG_NO_DEACTIVATION:
            case PHYSICS_DEBUG_NO_HELP_TEXT:
            case PHYSICS_DEBUG_DRAW_TEXT:
            case PHYSICS_DEBUG_PROFILE_TIMINGS:
            case PHYSICS_DEBUG_ENABLE_SAT_COMPARISON:
            case PHYSICS_DEBUG_DISABLE_BULLET_LCP:
            case PHYSICS_DEBUG_ENABLE_CCD:
            case PHYSICS_DEBUG_FAST_WIREFRAME:
            reset_debug_drawer = true;
            break;
        }
    }
    for (unsigned i=0 ; i<sizeof(option_keys_int)/sizeof(*option_keys_int) ; ++i) {
        PhysicsIntOption o = option_keys_int[i];
        int v_old = options_int[o];
        int v_new = new_options_int[o];
        if (v_old == v_new) continue;
        switch (o) {
            case PHYSICS_SOLVER_ITERATIONS:
            world->getSolverInfo().m_numIterations = v_new;
            break;
        }
    }
    for (unsigned i=0 ; i<sizeof(option_keys_float)/sizeof(*option_keys_float) ; ++i) {
        PhysicsFloatOption o = option_keys_float[i];
        float v_old = options_float[o];
        float v_new = new_options_float[o];
        if (v_old == v_new) continue;
        switch (o) {
            case PHYSICS_STEP_SIZE:
            break;
            case PHYSICS_GRAVITY_X:
            case PHYSICS_GRAVITY_Y:
            case PHYSICS_GRAVITY_Z:
            reset_gravity = true;
            break;
            case PHYSICS_DEACTIVATION_TIME:
            gDeactivationTime = v_new;
            break;
            case PHYSICS_CONTACT_BREAKING_THRESHOLD:
            gContactBreakingThreshold = v_new;
            break;
            case PHYSICS_SOLVER_DAMPING:
            world->getSolverInfo().m_damping = v_new;
            break;
            case PHYSICS_SOLVER_ERP:
            world->getSolverInfo().m_erp = v_new;
            break;
            case PHYSICS_SOLVER_ERP2:
            world->getSolverInfo().m_erp2 = v_new;
            break;
            case PHYSICS_SOLVER_LINEAR_SLOP:
            world->getSolverInfo().m_linearSlop = v_new;
            break;
            case PHYSICS_SOLVER_WARM_STARTING_FACTOR:
            world->getSolverInfo().m_warmstartingFactor = v_new;
            break;
            case PHYSICS_SOLVER_SPLIT_IMPULSE_THRESHOLD:
            world->getSolverInfo().m_splitImpulsePenetrationThreshold = v_new;
            break;
        }
    }

    options_bool = new_options_bool;
    options_int = new_options_int;
    options_float = new_options_float;


    if (reset_gravity) {
        gravity = btVector3(physics_option(PHYSICS_GRAVITY_X), physics_option(PHYSICS_GRAVITY_Y), physics_option(PHYSICS_GRAVITY_Z));

        // the only force we apply to objects is gravity, everything else is impulses

        // iterate through all objects, clearing their forces
        world->clearForces();

        // apply the new gravity to all objects
        for (int i=0 ; i<world->getNumCollisionObjects() ; ++i) {
            btCollisionObject* victim = world->getCollisionObjectArray()[i];
            btRigidBody* victim2 = btRigidBody::upcast(victim);
            if (victim2==NULL) continue;
            victim2->applyForce(gravity / victim2->getInvMass(), btVector3(0,0,0));
            victim2->activate();
        }

    }

    if (reset_solver_mode) {
        world->getSolverInfo().m_solverMode = 0
          | (physics_option(PHYSICS_SOLVER_RANDOMISE_ORDER) ? SOLVER_RANDMIZE_ORDER : 0)
          | (physics_option(PHYSICS_SOLVER_FRICTION_SEPARATE) ? SOLVER_FRICTION_SEPARATE : 0)
          | (physics_option(PHYSICS_SOLVER_USE_WARM_STARTING) ? SOLVER_USE_WARMSTARTING : 0)
          | (physics_option(PHYSICS_SOLVER_CACHE_FRIENDLY) ? SOLVER_CACHE_FRIENDLY : 0);
    }
    
    if (reset_debug_drawer) {
        debug_drawer->setDebugMode(0
            | (physics_option(PHYSICS_DEBUG_WIREFRAME) ? BulletDebugDrawer::DBG_DrawWireframe : 0)
            | (physics_option(PHYSICS_DEBUG_AABB) ? BulletDebugDrawer::DBG_DrawAabb : 0)
            | (physics_option(PHYSICS_DEBUG_FEATURES_TEXT) ? BulletDebugDrawer::DBG_DrawFeaturesText : 0)
            | (physics_option(PHYSICS_DEBUG_CONTACT_POINTS) ? BulletDebugDrawer::DBG_DrawContactPoints : 0)
            | (physics_option(PHYSICS_DEBUG_NO_DEACTIVATION) ? BulletDebugDrawer::DBG_NoDeactivation : 0)
            | (physics_option(PHYSICS_DEBUG_NO_HELP_TEXT) ? BulletDebugDrawer::DBG_NoHelpText : 0)
            | (physics_option(PHYSICS_DEBUG_DRAW_TEXT) ? BulletDebugDrawer::DBG_DrawText : 0)
            | (physics_option(PHYSICS_DEBUG_PROFILE_TIMINGS) ? BulletDebugDrawer::DBG_ProfileTimings : 0)
//            | (physics_option(PHYSICS_DEBUG_ENABLE_SET_COMPARISON) ? BulletDebugDrawer::DBG_EnableSetComparison : 0)
            | (physics_option(PHYSICS_DEBUG_DISABLE_BULLET_LCP) ? BulletDebugDrawer::DBG_DisableBulletLCP : 0)
            | (physics_option(PHYSICS_DEBUG_ENABLE_CCD) ? BulletDebugDrawer::DBG_EnableCCD : 0)
            | (physics_option(PHYSICS_DEBUG_CONSTRAINTS) ? BulletDebugDrawer::DBG_DrawConstraints : 0)
            | (physics_option(PHYSICS_DEBUG_CONSTRAINTS_LIMITS) ? BulletDebugDrawer::DBG_DrawConstraintLimits : 0)
            | (physics_option(PHYSICS_DEBUG_FAST_WIREFRAME) ? BulletDebugDrawer::DBG_FastWireframe : 0)
        );
    }
}

void physics_option_reset (void)
{
    physics_option(PHYSICS_GIMPACT_ONE_WAY_MESH_HACK, true);
    physics_option(PHYSICS_BUMPY_TRIANGLE_MESH_HACK, false);
    physics_option(PHYSICS_USE_TRIANGLE_EDGE_INFO, false);
    physics_option(PHYSICS_VERBOSE_CONTACTS, false);
    physics_option(PHYSICS_VERBOSE_CASTS, false);
    physics_option(PHYSICS_ERROR_CONTACTS, true);
    physics_option(PHYSICS_ERROR_CASTS, true);
    physics_option(PHYSICS_SOLVER_SPLIT_IMPULSE, false);
    physics_option(PHYSICS_SOLVER_RANDOMISE_ORDER, false);
    physics_option(PHYSICS_SOLVER_FRICTION_SEPARATE, false);
    physics_option(PHYSICS_SOLVER_USE_WARM_STARTING, true);
    physics_option(PHYSICS_SOLVER_CACHE_FRIENDLY, false);

    physics_option(PHYSICS_DEBUG_WIREFRAME, false);
    physics_option(PHYSICS_DEBUG_AABB, false);
    physics_option(PHYSICS_DEBUG_FEATURES_TEXT, false);
    physics_option(PHYSICS_DEBUG_CONTACT_POINTS, false);
    physics_option(PHYSICS_DEBUG_CONSTRAINTS, false);
    physics_option(PHYSICS_DEBUG_CONSTRAINTS_LIMITS, false);
    physics_option(PHYSICS_DEBUG_NO_DEACTIVATION, false);
    physics_option(PHYSICS_DEBUG_NO_HELP_TEXT, false);
    physics_option(PHYSICS_DEBUG_DRAW_TEXT, false);
    physics_option(PHYSICS_DEBUG_PROFILE_TIMINGS, false);
    physics_option(PHYSICS_DEBUG_ENABLE_SAT_COMPARISON, false);
    physics_option(PHYSICS_DEBUG_DISABLE_BULLET_LCP, false);
    physics_option(PHYSICS_DEBUG_ENABLE_CCD, false);
    physics_option(PHYSICS_DEBUG_FAST_WIREFRAME, false);

    physics_option(PHYSICS_SOLVER_ITERATIONS, 10);

    physics_option(PHYSICS_GRAVITY_X, 0.0f);
    physics_option(PHYSICS_GRAVITY_Y, 0.0f);
    physics_option(PHYSICS_GRAVITY_Z, -9.807f);
    physics_option(PHYSICS_STEP_SIZE, 1/200.0f);
    physics_option(PHYSICS_CONTACT_BREAKING_THRESHOLD, 0.02f);
    physics_option(PHYSICS_DEACTIVATION_TIME, 2.0f);
    physics_option(PHYSICS_SOLVER_DAMPING, 1.0f);
    physics_option(PHYSICS_SOLVER_ERP, 0.2f);
    physics_option(PHYSICS_SOLVER_ERP2, 0.1f);
    physics_option(PHYSICS_SOLVER_SPLIT_IMPULSE_THRESHOLD, -0.02f);
    physics_option(PHYSICS_SOLVER_WARM_STARTING_FACTOR, 0.85f);
}

static void init_options (void)
{

    for (unsigned i=0 ; i < sizeof(option_keys_bool) / sizeof(*option_keys_bool) ; ++i) {
        valid_option(option_keys_bool[i], truefalse);

    }

    valid_option(PHYSICS_SOLVER_ITERATIONS, new ValidOptionRange<int>(0,1000));

    valid_option(PHYSICS_GRAVITY_X, new ValidOptionRange<float>(-1000, 1000));
    valid_option(PHYSICS_GRAVITY_Y, new ValidOptionRange<float>(-1000, 1000));
    valid_option(PHYSICS_GRAVITY_Z, new ValidOptionRange<float>(-1000, 1000));
    valid_option(PHYSICS_STEP_SIZE, new ValidOptionRange<float>(0.00001f,100));
    valid_option(PHYSICS_CONTACT_BREAKING_THRESHOLD, new ValidOptionRange<float>(0.00001f,100));
    valid_option(PHYSICS_DEACTIVATION_TIME, new ValidOptionRange<float>(0.00001f,1000));
    valid_option(PHYSICS_SOLVER_DAMPING, new ValidOptionRange<float>(0.00001f,1000));
    valid_option(PHYSICS_SOLVER_ERP, new ValidOptionRange<float>(0.00001f,1000));
    valid_option(PHYSICS_SOLVER_ERP2, new ValidOptionRange<float>(0.00001f,1000));
    valid_option(PHYSICS_SOLVER_SPLIT_IMPULSE_THRESHOLD, new ValidOptionRange<float>(-1000,1000));
    valid_option(PHYSICS_SOLVER_WARM_STARTING_FACTOR, new ValidOptionRange<float>(0.00001f,1000));

    physics_option(PHYSICS_AUTOUPDATE, false);
    physics_option_reset();
    options_update(true);
    // This will call options_update(false) but it will be a no-op this time.
    physics_option(PHYSICS_AUTOUPDATE, true);
}

void physics_option (PhysicsBoolOption o, bool v)
{
    valid_option_bool[o]->maybeThrow("Physics", v);
    new_options_bool[o] = v;
    if (new_options_bool[PHYSICS_AUTOUPDATE]) options_update(false);
}
bool physics_option (PhysicsBoolOption o)
{
    return options_bool[o];
}

void physics_option (PhysicsIntOption o, int v)
{
    valid_option_int[o]->maybeThrow("Physics", v);
    new_options_int[o] = v;
    if (new_options_bool[PHYSICS_AUTOUPDATE]) options_update(false);
}
int physics_option (PhysicsIntOption o)
{
    return options_int[o];
}

void physics_option (PhysicsFloatOption o, float v)
{
    valid_option_float[o]->maybeThrow("Physics", v);
    new_options_float[o] = v;
    if (new_options_bool[PHYSICS_AUTOUPDATE]) options_update(false);
}
float physics_option (PhysicsFloatOption o)
{
    return options_float[o];
}



// }}}


// {{{ conversion and nan checking utilities

static inline btVector3 check_nan_ (const btVector3 &v, const char *file, int line)
{
    if (std::isnan(v.x()) || std::isnan(v.y()) || std::isnan(v.z())) {
        CLog(file,line,true) << "Vect3 NaN from Bullet." << std::endl;
        return btVector3(0,0,0);
    }
    return v;
}

static inline Vector3 check_nan_ (const Vector3 &v, const char *file, int line)
{
    if (std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z)) {
        CLog(file,line,true) << "Vect3 NaN from Grit." << std::endl;
        return Vector3(0,0,0);
    }
    return v;
}

static inline btQuaternion check_nan_ (const btQuaternion &v, const char *file, int line)
{
    if (std::isnan(v.w()) || std::isnan(v.x()) || std::isnan(v.y()) || std::isnan(v.z())) {
        CLog(file,line,true) << "Quat NaN from Bullet." << std::endl;
        return btQuaternion(0,0,0,1);
    }
    return v;
}

static inline Quaternion check_nan_ (const Quaternion &v, const char *file, int line)
{
    if (std::isnan(v.w) || std::isnan(v.x) || std::isnan(v.y) || std::isnan(v.z)) {
        CLog(file,line,true) << "Quat NaN from Grit." << std::endl;
        return Quaternion(1,0,0,0);
    }
    return v;
}

static inline Vector3 from_bullet (const btVector3 &from)
{ return Vector3 (from.x(), from.y(), from.z()); }

static inline Quaternion from_bullet (const btQuaternion &from)
{ return Quaternion (from.w(), from.x(), from.y(), from.z()); }

static inline btVector3 to_bullet (const Vector3 &from)
{ return btVector3(from.x,from.y,from.z); }

static inline btQuaternion to_bullet (const Quaternion &from)
{ return btQuaternion(from.x, from.y, from.z, from.w); }

#define check_nan(x) check_nan_(x,__FILE__,__LINE__)

// }}}



// {{{ Handling contacts, materials, etc

bool physics_verbose_contacts = false;

static std::ostream &operator << (std::ostream &o, const btVector3 &v)
{
    return o << "("<<v.x()<<", "<<v.y()<<", "<<v.z()<<")";
}

static std::string shape_str (int s)
{
    switch (s) {
        case BOX_SHAPE_PROXYTYPE: return "box";
        case TRIANGLE_SHAPE_PROXYTYPE: return "tri";
        case CONVEX_HULL_SHAPE_PROXYTYPE: return "hul";
        case SPHERE_SHAPE_PROXYTYPE: return "sph";
        case CAPSULE_SHAPE_PROXYTYPE: return "cap";
        case CONE_SHAPE_PROXYTYPE: return "con";
        case CYLINDER_SHAPE_PROXYTYPE: return "cyl";
        case TRIANGLE_MESH_SHAPE_PROXYTYPE: return "sta";
        case GIMPACT_SHAPE_PROXYTYPE: return "gim";
        case STATIC_PLANE_PROXYTYPE: return "pla";
        case COMPOUND_SHAPE_PROXYTYPE: return "com";
        default:
        std::stringstream ss;
        ss << s;
        return ss.str();
    }
    return "";
}

/* bullet does not support putting a gimpact triangle mesh in a compound shape so I am hacking the parent in this case to be the triangle mesh instead of the compound shape
*/
static void fix_parent (const btCollisionShape *shape, const btCollisionShape *&parent)
{
    APP_ASSERT(shape!=NULL);
    APP_ASSERT(parent!=NULL);
    if (shape->getShapeType()!=TRIANGLE_SHAPE_PROXYTYPE) return;
    if (parent->getShapeType()!=COMPOUND_SHAPE_PROXYTYPE) return;
    // iterate through children of compound shape searching for the gimpact shape

    // assume there is exactly 1 gimpact shape in the compound if the child shape
    // is a triangle

    const btCompoundShape *compound = static_cast<const btCompoundShape*>(parent);
    const btCollisionShape *new_parent = NULL;

    for (int i=0,i_=compound->getNumChildShapes() ; i<i_ ; ++i) {
        const btCollisionShape *child = compound->getChildShape(i);
        if (child->getShapeType()==GIMPACT_SHAPE_PROXYTYPE) {
            new_parent = child;
            break;
        }
        if (child->getShapeType()==TRIANGLE_MESH_SHAPE_PROXYTYPE) {
            new_parent = child;
            break;
        }
    }

    // this can happen with static triangle meshes in a compound
    //if (new_parent==NULL) return;
    APP_ASSERT(new_parent!=NULL);

    parent = new_parent;
}

static int get_material (const CollisionMesh *cmesh, const btCollisionShape *shape,
                         int id, bool *err, bool verb)
{
    // * when one gimpact shape hits another (not in compounds), we don't get the triangle
    // we get the whole gimpact shape for some reason
    // * when casting rays, we get the whole shape in the case of static meshes
    if (shape->getShapeType()==TRIANGLE_SHAPE_PROXYTYPE
        || shape->getShapeType()==GIMPACT_SHAPE_PROXYTYPE
        || shape->getShapeType()==TRIANGLE_MESH_SHAPE_PROXYTYPE) {
        int max = cmesh->faceMaterials.size();
        if (id < 0 || id >= max) {
            if (verb) {
                CERR << "index from bullet was garbage: " << id
                     << " >= " << max
                     << " cmesh: \"" << cmesh->getName() << "\""
                     << std::endl;
                if (err) *err = true;
            }
            id = 0;
        }
        return cmesh->getMaterialFromFace(id)->id;
    } else {
        int max = cmesh->partMaterials.size();
        if (id < 0 || id >= max) {
            if (verb) {
                CERR << "index from bullet was garbage: " << id
                     << " >= " << max
                     << " cmesh: \"" << cmesh->getName() << "\""
                     << std::endl;
                if (err) *err = true;
            }
            id = 0;
        }
        return cmesh->getMaterialFromPart(id)->id;
    }
}

static void get_shape_and_parent(const btCollisionObject* colObj,
                                 const btCollisionShape *&shape,
                                 const btCollisionShape *&parent)
{
    shape  = colObj->getCollisionShape();
    parent = colObj->getRootCollisionShape();
    fix_parent(shape, parent);
    APP_ASSERT(shape!=NULL);
    APP_ASSERT(parent!=NULL);
}

bool contact_added_callback (btManifoldPoint& cp,
                 const btCollisionObject* colObj0, int part0, int index0,
                 const btCollisionObject* colObj1, int part1, int index1)
{
    const btRigidBody *bbody0 = dynamic_cast<const btRigidBody*>(colObj0);
    const btRigidBody *bbody1 = dynamic_cast<const btRigidBody*>(colObj1);
    APP_ASSERT(bbody0!=NULL);
    APP_ASSERT(bbody1!=NULL);

    const RigidBody *body0 = static_cast<const RigidBody*>(bbody0->getMotionState());
    const RigidBody *body1 = static_cast<const RigidBody*>(bbody1->getMotionState());
    APP_ASSERT(body0!=NULL);
    APP_ASSERT(body1!=NULL);

    const CollisionMesh *cmesh0 = body0->colMesh, *cmesh1 = body1->colMesh;

    const btCollisionShape *shape0, *parent0, *shape1, *parent1;

    get_shape_and_parent(colObj0, shape0, parent0);
    get_shape_and_parent(colObj1, shape1, parent1);

    bool err = false;
    bool verb = physics_option(PHYSICS_ERROR_CONTACTS);
    bool verb_contacts = physics_option(PHYSICS_VERBOSE_CONTACTS);

    int mat0 = get_material(cmesh0, shape0, index0, &err, verb);
    int mat1 = get_material(cmesh1, shape1, index1, &err, verb);

    // FIXME: HACK! store materials in the part ids, I do not need the part ids and I think
    // Bullet does not either so this should be OK.
    cp.m_partId0 = mat0;
    cp.m_partId1 = mat1;

    phys_mats.getFrictionRestitution(mat0, mat1, cp.m_combinedFriction, cp.m_combinedRestitution);

    if (err || verb_contacts) {
        CLOG << mat0 << "[" << shape_str(shape0->getShapeType()) << "]"
             << "(" << shape_str(parent0->getShapeType()) << ")"
             << " " << part0 << " " << index0
             << "  AGAINST  " << mat1 << "[" << shape_str(shape1->getShapeType()) << "]"
             << "(" << shape_str(parent1->getShapeType()) << ")"
             << " " << part1 << " " << index1 << std::endl;
        CLOG << cp.m_lifeTime << " " << cp.m_positionWorldOnA
                      << " " << cp.m_positionWorldOnB
             << " " << cp.m_normalWorldOnB << " " << cp.m_distance1
             << " *" << cp.m_appliedImpulse << "* |" << cp.m_combinedFriction
             << "| >" << cp.m_combinedRestitution << "<" << std::endl;
        /*
        bool    m_lateralFrictionInitialized
        btScalar    m_appliedImpulseLateral1
        btScalar    m_appliedImpulseLateral2
        btVector3       m_lateralFrictionDir1
        btVector3       m_lateralFrictionDir2
        */
    }
    
    if (parent0->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE ||
        parent1->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE) {

        const btRigidBody *sta_body;
        const btRigidBody *dyn_body;

        if (parent0->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE) {
            sta_body = bbody0;
            dyn_body = bbody1;
        } else {
            dyn_body = bbody0;
            sta_body = bbody1;
        }


        if (physics_option(PHYSICS_USE_TRIANGLE_EDGE_INFO)) {
            btAdjustInternalEdgeContacts(cp,sta_body, dyn_body, part1,index1);
            if (verb_contacts) {
                CLOG << cp.m_lifeTime << " " << cp.m_positionWorldOnA
                              << " " << cp.m_positionWorldOnB
                     << " " << cp.m_normalWorldOnB << " " << cp.m_distance1
                     << " *" << cp.m_appliedImpulse << "* |" << cp.m_combinedFriction
                     << "| >" << cp.m_combinedRestitution << "<   (CORRECTION)" << std::endl;
                /*
                bool    m_lateralFrictionInitialized
                btScalar    m_appliedImpulseLateral1
                btScalar    m_appliedImpulseLateral2
                btVector3       m_lateralFrictionDir1
                btVector3       m_lateralFrictionDir2
                */
            }
        }

        if (physics_option(PHYSICS_BUMPY_TRIANGLE_MESH_HACK)) {
            const btTriangleShape *tshape =
                   static_cast<const btTriangleShape*>(sta_body->getCollisionShape());

            btVector3 normal;
            tshape->calcNormal(normal);

            normal = sta_body->getWorldTransform().getBasis() * normal;

            btScalar dot = normal.dot(cp.m_normalWorldOnB);

            btScalar magnitude = cp.m_normalWorldOnB.length();
            normal *= dot > 0 ? magnitude : -magnitude;

            cp.m_normalWorldOnB = normal;

            // Reproject collision point along normal.
            cp.m_positionWorldOnB =
                cp.m_positionWorldOnA - cp.m_normalWorldOnB * cp.m_distance1;
            cp.m_localPointB =
                sta_body->getWorldTransform().invXform(cp.m_positionWorldOnB);
        }

    }

    if (parent0->getShapeType() == GIMPACT_SHAPE_PROXYTYPE ||
        parent1->getShapeType() == GIMPACT_SHAPE_PROXYTYPE) {

        const btRigidBody *gim_body;

        if (parent0->getShapeType() == GIMPACT_SHAPE_PROXYTYPE) {
            gim_body = bbody0;
        } else {
            gim_body = bbody1;
        }

        if (physics_option(PHYSICS_GIMPACT_ONE_WAY_MESH_HACK)) {
            const btTriangleShape *tshape =
                   static_cast<const btTriangleShape*>(gim_body->getCollisionShape());

            btVector3 normal;
            tshape->calcNormal(normal);

            normal = gim_body->getWorldTransform().getBasis() * normal;

            btScalar dot = normal.dot(cp.m_normalWorldOnB) * (gim_body == bbody0 ? 1 : -1);

            if (dot > 0) {
                cp.m_normalWorldOnB -= 2 * dot * normal;
            }
        }
    }

    return true;
}

extern ContactAddedCallback gContactAddedCallback;

// }}}


// C++ requires this to not be a local class
namespace {
    struct Info {
        RigidBody *body, *other;
        float life, imp, dist;
        unsigned mat, matOther;
        Vector3 pos, posOther, norm;
    };
}

void physics_update (lua_State *L)
{
    float step_size = physics_option(PHYSICS_STEP_SIZE);
    world->internalStepSimulation(step_size);

    // NAN CHECKS
    // check whether NaN has crept in anywhere
    std::vector<RigidBody*> nan_bodies;
    for (int i=0 ; i<world->getNumCollisionObjects() ; ++i) {

        btCollisionObject* victim = world->getCollisionObjectArray()[i];
        btRigidBody* victim2 = btRigidBody::upcast(victim);
        if (victim2==NULL) continue;
        RigidBody *rb = static_cast<RigidBody*>(victim2->getMotionState());

        const btTransform &current_xform = victim2->getWorldTransform();
        const btVector3 &pos = current_xform.getOrigin();
        btQuaternion quat;
        current_xform.getBasis().getRotation(quat);
        float x=pos.x(), y=pos.y(), z=pos.z();
        float qw=quat.w(), qx=quat.x(), qy=quat.y(), qz=quat.z();
        if (std::isnan(x) || std::isnan(y) || std::isnan(z) ||
            std::isnan(qw) || std::isnan(qx) || std::isnan(qy) || std::isnan(qz)) {
            CERR << "NaN from physics engine position update." << std::endl;
            nan_bodies.push_back(rb);
        }
    }
    // chuck them out if they have misbehaved
    for (unsigned int i=0 ; i<nan_bodies.size() ; ++i) {
        nan_bodies[i]->destroy(L);
    }

    // COLLISION CALLBACKS
    std::vector<Info> infos;
    // first, check for collisions
    unsigned num_manifolds = world->getDispatcher()->getNumManifolds();
    for (unsigned i=0 ; i<num_manifolds; ++i) {
        btPersistentManifold* manifold =
            world->getDispatcher()->getManifoldByIndexInternal(i);
        btCollisionObject
            *ob_a = static_cast<btCollisionObject*>(manifold->getBody0()),
            *ob_b = static_cast<btCollisionObject*>(manifold->getBody1());
        APP_ASSERT(ob_a != NULL);
        APP_ASSERT(ob_b != NULL);

        btRigidBody* brb_a = btRigidBody::upcast(ob_a);
        btRigidBody* brb_b = btRigidBody::upcast(ob_b);

        APP_ASSERT(brb_a);
        APP_ASSERT(brb_b);

        RigidBody *rb_a = static_cast<RigidBody*>(brb_a->getMotionState());
        RigidBody *rb_b = static_cast<RigidBody*>(brb_b->getMotionState());

        // each manifold has a number of points (usually 3?) that provide
        // a stable foundation
        unsigned num_contacts = manifold->getNumContacts();
        for (unsigned j=0 ; j<num_contacts ; ++j) {
            btManifoldPoint &p = manifold->getContactPoint(j);
            unsigned mat0 = p.m_partId0;
            unsigned mat1 = p.m_partId1;
            Info infoA = {
                rb_a, rb_b, 
                (float)p.getLifeTime(), p.getAppliedImpulse(), -p.getDistance(),
                mat0, mat1, from_bullet(p.m_positionWorldOnA),
                from_bullet(p.m_positionWorldOnB), -from_bullet(p.m_normalWorldOnB)
            };
            infos.push_back(infoA);
            Info infoB = {
                rb_b, rb_a, 
                (float)p.getLifeTime(), p.getAppliedImpulse(), p.getDistance(),
                mat1, mat0, from_bullet(p.m_positionWorldOnB),
                from_bullet(p.m_positionWorldOnA), from_bullet(p.m_normalWorldOnB)
            };
            infos.push_back(infoB);
        }
    }
    for (unsigned i=0 ; i<infos.size(); ++i) {
        Info &info = infos[i];
        if (info.body->destroyed()) continue;
        info.body->collisionCallback(L, info.life, info.imp, info.other,
                                        info.mat, info.matOther,
                                        info.dist, info.pos, info.posOther, info.norm);
    }

    // STEP CALLBACKS
    for (int i=0 ; i<world->getNumCollisionObjects() ; ++i) {
        btCollisionObject* victim = world->getCollisionObjectArray()[i];
        btRigidBody* victim2 = btRigidBody::upcast(victim);
        if (victim2==NULL) continue;
        RigidBody *rb = static_cast<RigidBody*>(victim2->getMotionState());
        rb->stepCallback(L, step_size);
    }

    // STABILISE CALLBACKS
    for (int i=0 ; i<world->getNumCollisionObjects() ; ++i) {

        btCollisionObject* victim = world->getCollisionObjectArray()[i];
        btRigidBody* victim2 = btRigidBody::upcast(victim);
        if (victim2==NULL) continue;
        RigidBody *rb = static_cast<RigidBody*>(victim2->getMotionState());
        rb->stabiliseCallback(L, step_size);
    }
}

void physics_update_graphics (lua_State *L, float extrapolate)
{
    // to handle errors raised by the lua callback
    push_cfunction(L, my_lua_error_handler);

    // call all the graphic update callbacks
    for (int i=0 ; i<world->getNumCollisionObjects() ; ++i) {
        btCollisionObject* victim = world->getCollisionObjectArray()[i];
        btRigidBody* victim2 = btRigidBody::upcast(victim);
        if (victim2==NULL) continue;
        RigidBody *rb = static_cast<RigidBody*>(victim2->getMotionState());
        rb->updateGraphicsCallback(L, extrapolate);
    }

    lua_pop(L,1); // error handler
}

class BulletRayCallback : public btCollisionWorld::RayResultCallback {
    public:
    BulletRayCallback (SweepCallback &scb_) : scb(scb_) { }
    virtual btScalar addSingleResult (btCollisionWorld::LocalRayResult&r, bool)
    {
        btRigidBody *body = btRigidBody::upcast(r.m_collisionObject);
        if (body == NULL) return r.m_hitFraction;
        RigidBody *rb= dynamic_cast<RigidBody*>(body->getMotionState());
        if (rb == NULL) return r.m_hitFraction;
        APP_ASSERT(r.m_localShapeInfo!=NULL);
        int part, index;
        bool err = false;
        if (r.m_localShapeInfo) {
            part = r.m_localShapeInfo->m_shapePart;
            index = r.m_localShapeInfo->m_triangleIndex;
        } else {
            err = true;
            part = 0;
            index = 0;
        }

        const btCollisionShape *shape, *parent;
        get_shape_and_parent(body, shape, parent);

        bool verb = physics_option(PHYSICS_ERROR_CASTS);
        int m = get_material(rb->colMesh, shape, index, &err, verb);

        if (err || physics_option(PHYSICS_VERBOSE_CASTS)) {
            CLOG << "RAY HIT  " << m << "[" << shape_str(shape->getShapeType()) << "]"
                 << "(" << shape_str(parent->getShapeType()) << ")"
                 << " " << part << " " << index << std::endl;
            CLOG << r.m_hitFraction << " " << r.m_hitNormalLocal << std::endl;
        }


        scb.result(*rb, r.m_hitFraction, from_bullet(r.m_hitNormalLocal), m);
        return r.m_hitFraction;
    }
    protected:
    SweepCallback &scb;
};

class BulletSweepCallback : public btCollisionWorld::ConvexResultCallback {
    public:
    BulletSweepCallback (SweepCallback &scb_) : scb(scb_) { }
    virtual btScalar addSingleResult (btCollisionWorld::LocalConvexResult&r, bool)
    {
        btRigidBody *body = btRigidBody::upcast(r.m_hitCollisionObject);
        if (body == NULL) return r.m_hitFraction;
        RigidBody *rb= dynamic_cast<RigidBody*>(body->getMotionState());
        if (rb == NULL) return r.m_hitFraction;
        APP_ASSERT(r.m_localShapeInfo!=NULL);
        int part, index;
        bool err = false;
        if (r.m_localShapeInfo) {
            part = r.m_localShapeInfo->m_shapePart;
            index = r.m_localShapeInfo->m_triangleIndex;
        } else {
            part = 0;
            index = 0;
            err = true;
        }

        const btCollisionShape *shape, *parent;
        get_shape_and_parent(body, shape, parent);

        bool verb = physics_option(PHYSICS_ERROR_CASTS);
        int m = get_material(rb->colMesh, shape, index, &err, verb);

        if (err || physics_option(PHYSICS_VERBOSE_CASTS)) {
            CLOG << "SWEEP HIT  " << m << "[" << shape_str(shape->getShapeType()) << "]"
                 << "(" << shape_str(parent->getShapeType()) << ")"
                 << " " << part << " " << index << std::endl;
            CLOG << r.m_hitFraction << " " << r.m_hitNormalLocal << std::endl;
        }
        scb.result(*rb, r.m_hitFraction, from_bullet(r.m_hitNormalLocal), m);
        return r.m_hitFraction;
    }
    protected:
    SweepCallback &scb;
};

void physics_ray (const Vector3 &start,
                  const Vector3 &end,
                  SweepCallback &scb)
{
    BulletRayCallback brcb(scb);
    world->rayTest(to_bullet(start),to_bullet(end),brcb);
}

void physics_sweep_sphere (const Vector3 &start, const Vector3 &end,
                           SweepCallback &scb, float radius)
{
    BulletSweepCallback bscb(scb);
    btSphereShape tmpShape(radius);
    btTransform from(btQuaternion(0,0,0,1),to_bullet(start));
    btTransform to(btQuaternion(0,0,0,1),to_bullet(end));
    world->convexSweepTest(&tmpShape,from,to,bscb);
}

void physics_sweep_box (const Vector3 &start, const Quaternion &startq,
                        const Vector3 &end,
                        SweepCallback &scb, const Vector3 &size)
{
    BulletSweepCallback bscb(scb);
    btBoxShape tmpShape(to_bullet(size/2));
    btTransform from(to_bullet(startq),to_bullet(start));
    btTransform to(to_bullet(startq),to_bullet(end));
    world->convexSweepTest(&tmpShape,from,to,bscb);
}

void physics_sweep_cylinder (const Vector3 &start, const Quaternion &startq,
                             const Vector3 &end,
                             SweepCallback &scb, float radius, float height)
{
    BulletSweepCallback bscb(scb);
    btCylinderShapeZ tmpShape(btVector3(radius, radius, height/2));
    btTransform from(to_bullet(startq),to_bullet(start));
    btTransform to(to_bullet(startq),to_bullet(end));
    world->convexSweepTest(&tmpShape,from,to,bscb);
}

// Currently only supports a single hull.
void physics_sweep_col_mesh (const Vector3 &startp, const Quaternion &startq,
                             const Vector3 &endp, const Quaternion &endq,
                             SweepCallback &scb, const CollisionMesh *col_mesh)
{
    BulletSweepCallback bscb(scb);
    btTransform start(to_bullet(startq),to_bullet(startp));
    btTransform end(to_bullet(endq),to_bullet(endp));
    btCompoundShape *compound = col_mesh->getMasterShape();
    if (compound->getNumChildShapes() != 1)
        EXCEPT << "Can only sweep collision meshes comprised of a single convex hull." << std::endl;
    btConvexHullShape *conv = dynamic_cast<btConvexHullShape*>(compound->getChildShape(0));
    if (conv == nullptr)
        EXCEPT << "Can only sweep collision meshes comprised of a single convex hull." << std::endl;
    world->convexSweepTest(conv, start, end, bscb);
}

class BulletTestCallback : public btCollisionWorld::ContactResultCallback {
    
    public:
    
    /*! You may also want to set m_collisionFilterGroup and m_collisionFilterMask
     *  (supplied by the superclass) for needsCollision() */
    BulletTestCallback (btCollisionObject& obj_,
                        TestCallback &tcb_,
                        bool dyn_only)
        : obj(obj_), tcb(tcb_), dynOnly(dyn_only) { }
    
    virtual btScalar addSingleResult (btManifoldPoint& cp,
        const btCollisionObject* colObj0,int,int index0,
        const btCollisionObject* colObj1,int,int index1)
    {
        const btCollisionObject *colObj;
        Vector3 pos, wpos, norm;
        int index;
        if(colObj1==&obj) {
            colObj = colObj0;
            pos = from_bullet(cp.m_localPointA);
            wpos = from_bullet(cp.m_positionWorldOnA);
            norm = -from_bullet(cp.m_normalWorldOnB);
            index = index0;
        } else {
            colObj = colObj1;
            pos = from_bullet(cp.m_localPointB);
            wpos = from_bullet(cp.m_positionWorldOnB);
            norm = from_bullet(cp.m_normalWorldOnB);
            index = index1;
        }

        const btRigidBody *bbody = dynamic_cast<const btRigidBody*>(colObj);
        APP_ASSERT(bbody!=NULL);

        RigidBody *body = const_cast<RigidBody*>(static_cast<const RigidBody*>(
                                                                          bbody->getMotionState()));
        APP_ASSERT(body!=NULL);

        if (body->getMass()==0 && dynOnly) return 0;

        CollisionMesh *cmesh = body->colMesh;

        const btCollisionShape *shape, *parent;

        get_shape_and_parent(colObj, shape, parent);

        bool err = false;
        bool verb = physics_option(PHYSICS_ERROR_CONTACTS);

        int mat = get_material(cmesh, shape, index, &err, verb);

        tcb.result(body, pos, wpos, norm, -cp.getDistance(), mat);

        return 0;
    }

    public:
    btCollisionObject &obj;
    TestCallback &tcb;
    bool dynOnly;
};

void physics_test (const CollisionMesh *col_mesh, const Vector3 &pos, const Quaternion &quat,
                         bool dyn_only, TestCallback &cb_)
{
    btCollisionObject encroacher;
    encroacher.setCollisionShape(col_mesh->getMasterShape());
    encroacher.setWorldTransform(btTransform(to_bullet(quat), to_bullet(pos)));
    
    BulletTestCallback cb(encroacher,cb_,dyn_only);
    world->contactTest(&encroacher,cb);
}


void physics_test_sphere (float rad, const Vector3 &pos, bool dyn_only, TestCallback &cb_)
{
    btCollisionObject encroacher;
    btSphereShape sphere(rad);
    encroacher.setCollisionShape(&sphere);
    encroacher.setWorldTransform(btTransform(btQuaternion(0,0,0,1), to_bullet(pos)));
    
    BulletTestCallback cb(encroacher,cb_,dyn_only);
    world->contactTest(&encroacher,cb);
}



void physics_draw (void)
{
    world->debugDrawWorld();
}




// {{{ RigidBody

RigidBody::RigidBody (const std::string &col_mesh,
                      const Vector3 &pos,
                      const Quaternion &quat)
      : lastXform(to_bullet(quat),to_bullet(pos)), refCount(0)
{
    DiskResource *dr = disk_resource_get_or_make(col_mesh);
    colMesh = dynamic_cast<CollisionMesh*>(dr);
    if (colMesh==NULL) GRIT_EXCEPT("Not a collision mesh: \""+col_mesh+"\"");
    if (!colMesh->isLoaded()) GRIT_EXCEPT("Not loaded: \""+col_mesh+"\"");

    colMesh->registerReloadWatcher(this);
    body = NULL;
    addToWorld();
}


static btCompoundShape *clone_compound (btCompoundShape *master)
{
    btCompoundShape *slave = new btCompoundShape();
    for (int i=0 ; i<master->getNumChildShapes() ; ++i) {
        btCollisionShape *s = master->getChildShape(i);
        btTransform &t = master->getChildTransform(i);
        slave->addChildShape(t,s);
    }
    return slave;
}

/* The intention is that the object is always in the world.  The exceptions are when it is created,
 * after it has been destroyed, and in the brief moments when it is pulled out and put back in again
 * in response to a CollisionMesh reload. */

// Another exception:  When it has been polluted by a NaN and must be pulled out

void RigidBody::addToWorld (void)
{
    shape = clone_compound(colMesh->getMasterShape());
    localChanges.resize(shape->getNumChildShapes());
    // by default, turn everything on, leave it transformed as found in the master copy
    for (int i=0 ; i<localChanges.size() ; ++i) {
        localChanges[i].enabled = true;
        localChanges[i].offset.setIdentity();
    }

    btRigidBody::btRigidBodyConstructionInfo
        info(colMesh->getMass(), this, shape, to_bullet(colMesh->getInertia()));

    mass = colMesh->getMass();
    ghost = false;

    info.m_linearDamping = colMesh->getLinearDamping();
    info.m_angularDamping = colMesh->getAngularDamping();
    info.m_friction = 0;
    info.m_restitution = 0;
    info.m_linearSleepingThreshold = colMesh->getLinearSleepThreshold();
    info.m_angularSleepingThreshold = colMesh->getAngularSleepThreshold();

    APP_ASSERT(body==NULL);
    body = new btRigidBody(info);

    world->addRigidBody(body);

    // Only do CCD if speed of body exceeeds this
    body->setCcdMotionThreshold(colMesh->getCCDMotionThreshold());
    body->setCcdSweptSphereRadius(colMesh->getCCDSweptSphereRadius());
    body->applyForce(gravity * mass, btVector3(0,0,0));

    updateCollisionFlags();
}

void RigidBody::removeFromWorld (void)
{
    lastXform = body->getCenterOfMassTransform();
    world->removeRigidBody(body);
    delete body;
    delete shape;
    body = NULL;
}

void RigidBody::destroy (lua_State *L)
{
    colMesh->unregisterReloadWatcher(this);
    if (body==NULL) return;
    removeFromWorld();
    stepCallbackPtr.setNil(L);
    updateCallbackPtr.setNil(L);
    collisionCallbackPtr.setNil(L);
    stabiliseCallbackPtr.setNil(L);
}

void RigidBody::incRefCount (void)
{
    refCount++;
}

void RigidBody::decRefCount (lua_State *L)
{
    refCount--;
    if (refCount == 0) {
        destroy(L);
        // We don't have a destroy callback so no need to add extra complexity
        // like we have in HudObject.  We should probably unify all these approaches into
        // a generic superclass.
        APP_ASSERT(refCount == 0);
        delete this;
    }
}

RigidBody::~RigidBody (void)
{
    colMesh->unregisterReloadWatcher(this);
    if (body==NULL) return;
    CERR << "destructing RigidBody: destroy() was not called" << std::endl;
    // just leak stuff, this is not meant to happen
}

void RigidBody::getWorldTransform (btTransform& into_here) const
{
    into_here = lastXform;
}

void RigidBody::setWorldTransform (const btTransform& )
{
}

void RigidBody::updateGraphicsCallback (lua_State *L, float extrapolate)
{
    if (updateCallbackPtr.isNil()) return;

    btTransform current_xform;

    btTransformUtil::integrateTransform(
        body->getInterpolationWorldTransform(),
        body->getInterpolationLinearVelocity(),
        body->getInterpolationAngularVelocity(),
        extrapolate*body->getHitFraction(),
        current_xform);


    STACK_BASE;

    // args
    const btVector3 &pos = check_nan(current_xform.getOrigin());
    btQuaternion quat;
    current_xform.getBasis().getRotation(quat);
    quat = check_nan(quat);

    int error_handler = lua_gettop(L);

    // get callback
    updateCallbackPtr.push(L);

    push_v3(L,from_bullet(pos)); // arg 1
    push_quat(L,from_bullet(quat)); // arg 1

    // call callback (7 args, no return values)
    int status = lua_pcall(L,2,0,error_handler);
    if (status) {
        // pop the error message since the error handler will
        // have already printed it out
        lua_pop(L,1);
        updateCallbackPtr.setNil(L);
    }

    STACK_CHECK;
}

void RigidBody::stepCallback (lua_State *L, float step_size)
{
    if (body->getInvMass()==0) {
        btTransform after;
        btTransformUtil::integrateTransform(body->getCenterOfMassTransform(),
                            body->getLinearVelocity(),
                            body->getInterpolationAngularVelocity(),
                            step_size,
                            after);
        body->proceedToTransform(after);
    }
    if (stepCallbackPtr.isNil()) return;

    STACK_BASE;

    // to handle errors raised by the lua callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    // get callback
    stepCallbackPtr.push(L);
    lua_pushnumber(L, step_size);

    // call callback (no args, no return values)
    int status = lua_pcall(L,1,0,error_handler);
    if (status) {
        lua_pop(L,1);
        stepCallbackPtr.setNil(L);
    }

    lua_pop(L,1); // error handler

    STACK_CHECK;
}

void RigidBody::collisionCallback (lua_State *L, int lifetime, float impulse,
                                   RigidBody *other, int m, int m2,
                                   float penetration, const Vector3 &pos, const Vector3 &pos2,
                                   const Vector3 &wnormal)
{
    if (collisionCallbackPtr.isNil()) return;

    STACK_BASE;

    // to handle errors raised by the lua callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    // get callback
    collisionCallbackPtr.push(L);

    lua_pushnumber(L,lifetime);
    lua_pushnumber(L,impulse);
    push_rbody(L,other);
    lua_pushstring(L,phys_mats.getMaterial(m)->name.c_str());
    lua_pushstring(L,phys_mats.getMaterial(m2)->name.c_str());
    lua_pushnumber(L,penetration);
    push_v3(L,pos);
    push_v3(L,pos2);
    push_v3(L,wnormal);
    int status = lua_pcall(L,9,0,error_handler);
    if (status) {
        lua_pop(L,1);
        collisionCallbackPtr.setNil(L);
    }

    lua_pop(L,1); // error handler

    STACK_CHECK;
}

void RigidBody::stabiliseCallback (lua_State *L, float elapsed)
{
    if (stabiliseCallbackPtr.isNil()) return;

    STACK_BASE;

    // to handle errors raised by the lua callback
    push_cfunction(L, my_lua_error_handler);
    int error_handler = lua_gettop(L);

    // get callback
    stabiliseCallbackPtr.push(L);

    lua_pushnumber(L, elapsed);

    // call callback (no args, no return values)
    int status = lua_pcall(L,1,0,error_handler);
    if (status) {
        lua_pop(L,1);
        stabiliseCallbackPtr.setNil(L);
    }

    lua_pop(L,1); // error handler

    STACK_CHECK;
}

void RigidBody::force (const Vector3 &force)
{
    if (std::isnan(force.x) || std::isnan(force.y) || std::isnan(force.z))
        GRIT_EXCEPT("RigidBody::force received NaN element in force vector");
    if (body==NULL) return;
    body->applyCentralImpulse(to_bullet(force*physics_option(PHYSICS_STEP_SIZE)));
    body->activate();
}

void RigidBody::force (const Vector3 &force,
               const Vector3 &rel_pos)
{
    if (std::isnan(force.x) || std::isnan(force.y) || std::isnan(force.z))
        GRIT_EXCEPT("RigidBody::force received NaN element in force vector");
    if (std::isnan(rel_pos.x) || std::isnan(rel_pos.y) || std::isnan(rel_pos.z))
        GRIT_EXCEPT("RigidBody::force received NaN element in position vector");
    if (body==NULL) return;
    body->applyImpulse(to_bullet(force*physics_option(PHYSICS_STEP_SIZE)),
               to_bullet(rel_pos));
    body->activate();
}

void RigidBody::impulse (const Vector3 &impulse)
{
    if (std::isnan(impulse.x) || std::isnan(impulse.y) || std::isnan(impulse.z))
        GRIT_EXCEPT("RigidBody::impulse received NaN element in impulse vector");
    if (body==NULL) return;
    body->applyCentralImpulse(to_bullet(impulse));
    body->activate();
}

void RigidBody::impulse (const Vector3 &impulse,
             const Vector3 &rel_pos)
{
    if (std::isnan(impulse.x) || std::isnan(impulse.y) || std::isnan(impulse.z))
        GRIT_EXCEPT("RigidBody::impulse received NaN element in impulse vector");
    if (std::isnan(rel_pos.x) || std::isnan(rel_pos.y) || std::isnan(rel_pos.z))
        GRIT_EXCEPT("RigidBody::impulse received NaN element in position vector");
    if (body==NULL) return;
    body->applyImpulse(to_bullet(impulse),to_bullet(rel_pos));
    body->activate();
}

void RigidBody::torque (const Vector3 &torque)
{
    if (std::isnan(torque.x) || std::isnan(torque.y) || std::isnan(torque.z))
        GRIT_EXCEPT("RigidBody::torque received NaN element in torque vector");
    if (body==NULL) return;
    body->applyTorqueImpulse(to_bullet(torque*physics_option(PHYSICS_STEP_SIZE)));
    body->activate();
}

void RigidBody::torqueImpulse (const Vector3 &torque)
{
    if (std::isnan(torque.x) || std::isnan(torque.y) || std::isnan(torque.z))
        GRIT_EXCEPT("RigidBody::torque received NaN element in torque vector");
    if (body==NULL) return;
    body->applyTorqueImpulse(to_bullet(torque));
    body->activate();
}

float RigidBody::getContactProcessingThreshold (void) const
{
    if (body==NULL) return 0;
    return body->getContactProcessingThreshold();
}

void RigidBody::setContactProcessingThreshold (float r)
{
    if (body==NULL) return;
    body->setContactProcessingThreshold(r);
}

float RigidBody::getLinearDamping (void) const
{
    if (body==NULL) return 0;
    return body->getLinearDamping();
}

void RigidBody::setLinearDamping (float r)
{
    if (body==NULL) return;
    body->setDamping(r,getAngularDamping());
}

float RigidBody::getAngularDamping (void) const
{
    if (body==NULL) return 0;
    return body->getAngularDamping();
}

void RigidBody::setAngularDamping (float r)
{
    if (body==NULL) return;
    body->setDamping(getLinearDamping(),r);
}


float RigidBody::getLinearSleepThreshold (void) const
{
    if (body==NULL) return 0;
    return body->getLinearSleepingThreshold();
}

void RigidBody::setLinearSleepThreshold (float r)
{
    if (body==NULL) return;
    body->setSleepingThresholds(r,getAngularSleepThreshold());
}

float RigidBody::getAngularSleepThreshold (void) const
{
    if (body==NULL) return 0;
    return body->getAngularSleepingThreshold();
}

void RigidBody::setAngularSleepThreshold (float r)
{
    if (body==NULL) return;
    body->setSleepingThresholds(getLinearSleepThreshold(),r);
}


void RigidBody::activate (void)
{
    if (body==NULL) return;
    body->activate();
}

void RigidBody::deactivate (void)
{
    body->setActivationState( WANTS_DEACTIVATION );
}

Vector3 RigidBody::getLinearVelocity (void) const
{
    if (body==NULL) return Vector3(0,0,0);
    return from_bullet(body->getLinearVelocity());
}

void RigidBody::setLinearVelocity (const Vector3 &v)
{
    if (body==NULL) return;
    body->setLinearVelocity(to_bullet(v));
    body->activate();
}

Vector3 RigidBody::getAngularVelocity (void) const
{
    if (body==NULL) return Vector3(0,0,0);
    return from_bullet(body->getAngularVelocity());
}

Vector3 RigidBody::getLocalVelocity (const Vector3 &v) const
{
    if (body==NULL) return Vector3(0,0,0);
    return from_bullet(body->getVelocityInLocalPoint(to_bullet(v)));
}

void RigidBody::setAngularVelocity (const Vector3 &v)
{
    if (body==NULL) return;
    body->setAngularVelocity(to_bullet(v));
    body->activate();
}

static inline float invert0 (float v)
{
    return v==0 ? 0 : 1/v;
}

static inline Vector3 invert0 (const Vector3 &v)
{
    return Vector3(invert0(v.x),invert0(v.y),invert0(v.z));
}

float RigidBody::getMass (void) const
{
    if (body==NULL) return 0;
    return mass;
}

void RigidBody::setMass (float r)
{
    if (body==NULL) return;
    mass = r;
    body->setMassProps(r,to_bullet(getInertia()));
}


Vector3 RigidBody::getInertia (void) const
{
    if (body==NULL) return Vector3(0,0,0);
    return invert0(from_bullet(body->getInvInertiaDiagLocal()));
}

void RigidBody::setInertia (const Vector3 &v)
{
    if (body==NULL) return;
    body->setMassProps(getMass(),to_bullet(v));
}

static int get_child_index(btCompoundShape* c, btCollisionShape* shape)
{
    for (int i=0 ; i<c->getNumChildShapes() ; ++i) {
        if(c->getChildShape(i) == shape)
            return i;
    }
    CERR << "Could not find child shape in compound." << std::endl;
    return 0;
}


void RigidBody::setElementEnabled (int i, bool v)
{
    if (getElementEnabled(i)==v) return;
    localChanges[i].enabled = v;
    if (v) {
        btCollisionShape *s = colMesh->getMasterShape()->getChildShape(i);
        btTransform &t = colMesh->getMasterShape()->getChildTransform(i);
        t = t * localChanges[i].offset;
        shape->addChildShape(t,s);
    } else {
        int i2 = get_child_index(shape, colMesh->getMasterShape()->getChildShape(i));
        world->getBroadphase()->getOverlappingPairCache()
            ->cleanProxyFromPairs(body->getBroadphaseHandle(), world->getDispatcher());
        shape->removeChildShapeByIndex(i2);
    }
}

bool RigidBody::getElementEnabled (int i)
{
    APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
    return localChanges[i].enabled;
}

Vector3 RigidBody::getElementPositionMaster (int i)
{
    APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
    return from_bullet(colMesh->getMasterShape()->getChildTransform(i).getOrigin());
}

void RigidBody::setElementPositionOffset (int i, const Vector3 &v)
{
    APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
    localChanges[i].offset.setOrigin(to_bullet(v));
    if (localChanges[i].enabled) {
        int i2 = get_child_index(shape, colMesh->getMasterShape()->getChildShape(i));
        shape->updateChildTransform(i2, colMesh->getMasterShape()->getChildTransform(i) * localChanges[i].offset);
    }
}

Vector3 RigidBody::getElementPositionOffset (int i)
{
    APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
    return from_bullet(localChanges[i].offset.getOrigin());
}

Quaternion RigidBody::getElementOrientationMaster (int i)
{
    APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
    return from_bullet(colMesh->getMasterShape()->getChildTransform(i).getRotation());
}

void RigidBody::setElementOrientationOffset (int i, const Quaternion &q)
{
    APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
    localChanges[i].offset.setRotation(to_bullet(q));
    if (localChanges[i].enabled) {
        int i2 = get_child_index(shape, colMesh->getMasterShape()->getChildShape(i));
        shape->updateChildTransform(i2, colMesh->getMasterShape()->getChildTransform(i) * localChanges[i].offset);
    }
}

Quaternion RigidBody::getElementOrientationOffset (int i)
{
    APP_ASSERT(i>=0); APP_ASSERT(i<(int)localChanges.size());
    return from_bullet(localChanges[i].offset.getRotation());
}

Vector3 RigidBody::getPosition (void) const
{
    if (body==NULL) return Vector3(0,0,0); // deactivated
    return from_bullet(body->getCenterOfMassPosition());
}

Quaternion RigidBody::getOrientation (void) const
{
    if (body==NULL) return Quaternion(0,1,0,0); // deactivated
    return from_bullet(body->getOrientation());
}

void RigidBody::setPosition (const Vector3 &v)
{
    if (body==NULL) return; // deactivated
    body->setCenterOfMassTransform(
        btTransform(body->getOrientation(), to_bullet(v)));
    world->updateSingleAabb(body);
    body->activate();
}

void RigidBody::setOrientation (const Quaternion &q)
{
    if (body==NULL) return; // deactivated
    body->setCenterOfMassTransform(
         btTransform(to_bullet(q),body->getCenterOfMassPosition()));
    world->updateSingleAabb(body);
    body->activate();
}


void RigidBody::updateCollisionFlags (void)
{
    if (body==NULL) return; // deactivated
    body->setCollisionFlags(
        btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK
      | (ghost ? btCollisionObject::CF_NO_CONTACT_RESPONSE : 0)
      | (mass == 0 ? btCollisionObject::CF_STATIC_OBJECT : 0)
    );
    body->activate();
}

// }}}


void physics_init (void)
{
    col_conf = new btDefaultCollisionConfiguration();
    col_disp = new btCollisionDispatcher(col_conf);

    broadphase = new btDbvtBroadphase();

    con_solver = new btSequentialImpulseConstraintSolver();

    world = new DynamicsWorld(col_disp,broadphase,con_solver,col_conf);

    gContactAddedCallback = contact_added_callback;
    
    world->setDebugDrawer(debug_drawer);

    init_options();
}

void physics_shutdown (void)
{
    // they should probably all be gone by now but just in case some
    // aren't linked to RigidBodies, this will clean them up
    for (int i=world->getNumCollisionObjects()-1 ; i>=0 ; --i) {

        btCollisionObject* victim = world->getCollisionObjectArray()[i];

        btRigidBody* victim2 = btRigidBody::upcast(victim);

        if (victim2)
            world->removeRigidBody(victim2);

        delete victim;
    }


    delete world;
    delete con_solver;
    delete broadphase;
    delete col_disp;
    delete col_conf;
}
