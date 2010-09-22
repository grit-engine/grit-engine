/* Copyright (c) David Cunningham 2010 
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
                         
#ifndef HANDLING_H               
#define HANDLING_H       

#include <istream>
#include <vector>
#include <string> 
#include <map>  

#include "csvread.h"

// http://projectcerbera.com/gta/sa/tutorials/handling
struct VehicleData {
    std::string name;
    float mass, turn_mass, drag;

    float com_x, com_y, com_z;
    float bouyancy;
    float traction_mult, traction_loss, traction_bias;

    int gears;
    float max_vel, engine_accel, engine_innertia; // val in km/h
    bool front_wheel_drive;
    bool back_wheel_drive; // can both be true for 4wd
    int engine_type; // 0=petrol, 1=diesel, 2=electric

    float brakes;
    float brake_bias;
    bool abs;
    float steering_lock;

    float susp_force, susp_damp, susp_high_spd_com_damp, susp_upper, susp_lower, susp_bias;
    float susp_anti_dive;

    float seat_offset; // 'towards centre' apparently
    float damage_mult;
    int value; // in dollars

    bool is_van, is_bus, is_low, is_big;
    bool reverse_bonnet, hanging_boot, tailgate_boot, noswing_boot;
    bool no_doors, tandem_seats, sit_in_boat, convertible;
    bool no_exhaust, double_exhaust, no1fps_look_behind, force_door_check;
    bool axle_f_notilt, axle_f_solid, axle_f_mcpherson, axle_f_reverse;
    bool axle_r_notilt, axle_r_solid, axle_r_mcpherson, axle_r_reverse;
    bool is_bike, is_heli, is_plane, is_boat;
    bool bounce_panels, double_rwheels, force_ground_clearance, is_hatchback;
    
    bool one_g_boost, two_g_boost, npc_anti_roll, npc_neutral_handl;
    bool no_handbrake, steer_rearwheels, hb_rearwheel_steer, alt_steer_opt;
    bool wheel_f_narrow2, wheel_f_narrow, wheel_f_wide, wheel_f_wide2;
    bool wheel_r_narrow2, wheel_r_narrow, wheel_r_wide, wheel_r_wide2;
    bool hydraulic_geom, hydraulic_inst, hydraulic_none, nos_inst;
    bool offread_ability, offroad_ability2, halogen_lights, proc_rearwheel_1st;
    bool use_maxsp_limit, low_rider, street_race;
    bool swinging_chassis;

    int front_lights; // 0=long, 1=small, 2=big, 3=tall
    int rear_lights;

    int anim_group;

    // boat
    bool has_boat_data;
    float boat_thrust_y, boat_thrust_z, boat_thrust_app_z;
    float boat_aq_plane_force, boat_aq_plane_limit, boat_aq_plane_offset;
    float boat_wave_audio_muilt;
    float boat_move_res_x, boat_move_res_y, boat_move_res_z;
    float boat_turn_res_x, boat_turn_res_y, boat_turn_res_z;
    float boat_look_l_r_behind_cam_height;

    // bike
    bool has_bike_data;
    float bike_lean_fwd_com, bike_lean_fwd_force;
    float bike_lean_back_com, bike_lean_back_force;
    float bike_max_lean, bike_full_anim_lean, bike_des_lean;
    float bike_speed_steer, bike_slip_steer, bike_no_player_com_z;
    float bike_wheelie_ang, bike_stoppie_ang, bike_wheelie_steer;
    float bike_wheelie_stab_mult, bike_stoppie_stab_mult;

    // plane
    bool has_plane_data;
    float plane_thrust, plane_thrust_falloff, plane_yaw, plane_yaw_stab, plane_side_slip;
    float plane_roll, plane_roll_stab, plane_pitch, plane_pitch_stab;
    float plane_form_lift, plane_attack_lift, plane_gear_up_r, plane_gear_down_l;
    float plane_wind_mult, plane_move_res;
    float plane_turn_res_x, plane_turn_res_y, plane_turn_res_z;
    float plane_speed_res_x, plane_speed_res_y, plane_speed_res_z;
};

struct HandlingData {    
    std::vector<VehicleData> vehicles;

    // the map points into the vehicles vector above, so don't resize the above vector
    typedef std::map<std::string,VehicleData*> VehicleDataMap;
    typedef VehicleDataMap::iterator iterator;
    VehicleDataMap map;

    VehicleData *&operator [] (const std::string &i) { return map[i]; }
                        
    iterator begin() { return map.begin(); }
    iterator end() { return map.end(); }
    iterator find(const std::string &i) { return map.find(i); }

};              

// the csv should be already read in
void read_handling (Csv &csv, HandlingData &data);
                        
#endif                  

