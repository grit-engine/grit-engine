#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <locale>


#include "handling.h"
#include "ios_util.h"

static float f (const std::string &s) { return (float)strtod(s.c_str(), NULL); }
static int dec (const std::string &s) { return (int)strtod(s.c_str(), NULL); }
static unsigned long hex (const std::string &s) { return strtoul(s.c_str(), NULL, 16); }

static int tolowr (int c)
{
        return std::tolower(char(c),std::cout.getloc());
}

static std::string strlower (std::string s)
{
        std::transform(s.begin(),s.end(), s.begin(),tolowr);
        return s;
}

void read_handling (Csv &csv, HandlingData &data)
{
        const CsvSection &s = csv["nosection"];
        for (unsigned i=0 ; i<s.size() ; ++i) {
                const CsvLine &line = s[i];
                ASSERT(line.size()>0);

                if (line[0] == "%") continue;
                if (line[0] == "!") continue;
                if (line[0] == "$") continue;
                if (line[0] == "^") continue;

                ASSERT(line.size()==36);

                VehicleData v;
                v.name = strlower(line[0]);
                v.mass = f(line[1]);
                v.turn_mass = f(line[2]);
                v.drag = f(line[3]);

                v.com_x = f(line[4]);
                v.com_y = f(line[5]);
                v.com_z = f(line[6]);
                v.bouyancy = f(line[7]);
                v.traction_mult = f(line[8]);
                v.traction_loss = f(line[9]);
                v.traction_bias = f(line[10]);

                v.gears = dec(line[11]);
                v.max_vel = f(line[12]);
                v.engine_accel = f(line[13]);
                v.engine_innertia = f(line[14]);

                char drive = line[15][0];
                v.front_wheel_drive = drive=='F' || drive=='4';
                v.back_wheel_drive = drive=='R' || drive=='4';

                char et = line[16][0];
                switch (et) {
                        case 'P': v.engine_type = 0; break;
                        case 'D': v.engine_type = 1; break;
                        case 'E': v.engine_type = 2; break;
                }

                v.brakes = f(line[17]);
                v.brake_bias = f(line[18]);
                v.abs = dec(line[19])==1;
                ASSERT(!v.abs);
                v.steering_lock = f(line[20]);

                v.susp_force = f(line[21]);
                v.susp_damp = f(line[22]);
                v.susp_high_spd_com_damp = f(line[23]);
                v.susp_upper = f(line[24]);
                v.susp_lower = f(line[25]);
                v.susp_bias = f(line[26]);
                v.susp_anti_dive = f(line[27]);

                v.seat_offset = f(line[28]);
                v.damage_mult = f(line[29]);
                v.value = dec(line[30]); // in dollars

                unsigned long mflags = hex(line[31]);
                v.is_van = mflags & 0x1;
                v.is_bus = mflags & 0x2;
                v.is_low = mflags & 0x4;
                v.is_big = mflags & 0x8;
                v.reverse_bonnet = mflags & 0x10;
                v.hanging_boot = mflags & 0x20;
                v.tailgate_boot = mflags & 0x40;
                v.noswing_boot = mflags & 0x80;
                v.no_doors = mflags & 0x100;
                v.tandem_seats = mflags & 0x200;
                v.sit_in_boat = mflags & 0x400;
                v.convertible = mflags & 0x800;
                v.no_exhaust = mflags & 0x100;
                v.double_exhaust = mflags & 0x2000;
                v.no1fps_look_behind = mflags & 0x4000;
                v.force_door_check = mflags & 0x8000;
                v.axle_f_notilt = mflags & 0x1000;
                v.axle_f_solid = mflags & 0x2000;
                v.axle_f_mcpherson = mflags & 0x40000;
                v.axle_f_reverse = mflags & 0x80000;
                v.axle_r_notilt = mflags & 0x10000;
                v.axle_r_solid = mflags & 0x20000;
                v.axle_r_mcpherson = mflags & 0x400000;
                v.axle_r_reverse = mflags & 0x800000;
                v.is_bike = mflags & 0x1000000;
                v.is_heli = mflags & 0x2000000;
                v.is_plane = mflags & 0x4000000;
                v.is_boat = mflags & 0x8000000;
                v.bounce_panels = mflags & 0x10000000;
                v.double_rwheels = mflags & 0x20000000;
                v.force_ground_clearance = mflags & 0x40000000;
                v.is_hatchback = mflags & 0x80000000;

                unsigned long hflags = hex(line[32]);
                v.one_g_boost = hflags & 0x1;
                v.two_g_boost = hflags & 0x2;
                v.npc_anti_roll = hflags & 0x4;
                v.npc_neutral_handl = hflags & 0x8;
                v.no_handbrake = hflags & 0x10;
                v.steer_rearwheels = hflags & 0x20;
                v.hb_rearwheel_steer = hflags & 0x40;
                v.alt_steer_opt = hflags & 0x80;
                v.wheel_f_narrow2 = hflags & 0x100;
                v.wheel_f_narrow = hflags & 0x200;
                v.wheel_f_wide = hflags & 0x400;
                v.wheel_f_wide2 = hflags & 0x800;
                v.wheel_r_narrow2 = hflags & 0x1000;
                v.wheel_r_narrow = hflags & 0x2000;
                v.wheel_r_wide = hflags & 0x4000;
                v.wheel_r_wide2 = hflags & 0x8000;
                v.hydraulic_geom = hflags & 0x10000;
                v.hydraulic_inst = hflags & 0x20000;
                v.hydraulic_none = hflags & 0x40000;
                v.nos_inst = hflags & 0x80000;
                v.offread_ability = hflags & 0x100000;
                v.offroad_ability2 = hflags & 0x200000;
                v.halogen_lights = hflags & 0x400000;
                v.proc_rearwheel_1st = hflags & 0x800000;
                v.use_maxsp_limit = hflags & 0x1000000;
                v.low_rider = hflags & 0x2000000;
                v.street_race = hflags & 0x4000000;
                v.swinging_chassis = hflags & 0x10000000;

                v.front_lights = dec(line[33]); // 0=long, 1=small, 2=big, 3=tall
                v.rear_lights = dec(line[34]);

                v.anim_group = dec(line[35]);

                v.has_boat_data = false;
                v.has_bike_data = false;
                v.has_plane_data = false;

                data.vehicles.push_back(v);
        }
        for (unsigned i=0 ; i<data.vehicles.size() ; ++i) {
                VehicleData &v = data.vehicles[i];
                data[v.name] = &v;
        }
        for (unsigned i=0 ; i<s.size() ; ++i) {
                const CsvLine &line = s[i];
                ASSERT(line.size()>0);
                if (line[0] == "%") {
                        ASSERT(line.size()==16);
                        const std::string &name = strlower(line[1]);
                        ASSERT(data[name]!=NULL);
                        VehicleData &v = *data[name];
                        v.has_boat_data = true;
                        // boat
                        v.boat_thrust_y = f(line[2]);
                        v.boat_thrust_z = f(line[3]);
                        v.boat_thrust_app_z = f(line[4]);
                        v.boat_aq_plane_force = f(line[5]);
                        v.boat_aq_plane_limit = f(line[6]);
                        v.boat_aq_plane_offset = f(line[7]);
                        v.boat_wave_audio_muilt = f(line[8]);
                        v.boat_move_res_x = f(line[9]);
                        v.boat_move_res_y = f(line[10]);
                        v.boat_move_res_z = f(line[11]);
                        v.boat_turn_res_x = f(line[12]);
                        v.boat_turn_res_y = f(line[13]);
                        v.boat_turn_res_z = f(line[14]);
                        v.boat_look_l_r_behind_cam_height = f(line[15]);
                } else if (line[0] == "!") {
                        ASSERT(line.size()==17);
                        const std::string &name = strlower(line[1]);
                        ASSERT(data[name]!=NULL);
                        VehicleData &v = *data[name];
                        v.has_bike_data = true;
                        // bike
                        v.bike_lean_fwd_com = f(line[2]);
                        v.bike_lean_fwd_force = f(line[3]);
                        v.bike_lean_back_com = f(line[4]);
                        v.bike_lean_back_force = f(line[5]);
                        v.bike_max_lean = f(line[6]);
                        v.bike_full_anim_lean = f(line[7]);
                        v.bike_des_lean = f(line[8]);
                        v.bike_speed_steer = f(line[9]);
                        v.bike_slip_steer = f(line[10]);
                        v.bike_no_player_com_z = f(line[11]);
                        v.bike_wheelie_ang = f(line[12]);
                        v.bike_stoppie_ang = f(line[13]);
                        v.bike_wheelie_steer = f(line[14]);
                        v.bike_wheelie_stab_mult = f(line[15]);
                        v.bike_stoppie_stab_mult = f(line[16]);
                } else if (line[0] == "$") {
                        ASSERT(line.size()==23);
                        const std::string &name = strlower(line[1]);
                        ASSERT(data[name]!=NULL);
                        VehicleData &v = *data[name];
                        v.has_plane_data = true;
                        // plane
                        v.plane_thrust = f(line[2]);
                        v.plane_thrust_falloff = f(line[3]);
                        v.plane_yaw = f(line[4]);
                        v.plane_yaw_stab = f(line[5]);
                        v.plane_side_slip = f(line[6]);
                        v.plane_roll = f(line[7]);
                        v.plane_roll_stab = f(line[8]);
                        v.plane_pitch = f(line[9]);
                        v.plane_pitch_stab = f(line[10]);
                        v.plane_form_lift = f(line[11]);
                        v.plane_attack_lift = f(line[12]);
                        v.plane_gear_up_r = f(line[13]);
                        v.plane_gear_down_l = f(line[14]);
                        v.plane_wind_mult = f(line[15]);
                        v.plane_move_res = f(line[16]);
                        v.plane_turn_res_x = f(line[17]);
                        v.plane_turn_res_y = f(line[18]);
                        v.plane_turn_res_z = f(line[19]);
                        v.plane_speed_res_x = f(line[20]);
                        v.plane_speed_res_y = f(line[21]);
                        v.plane_speed_res_z = f(line[22]);
                }
        }
}



#ifdef _HANDLING_EXEC

#include "console_colour.h"

void app_verbose(char const* file, int line, const std::string& msg)
{
        std::cout<<BOLD GREEN"VERBOSE "RESET
                 <<BOLD<<file<<NOBOLD":"BOLD<<line<<NOBOLD
                 <<": \""BOLD BLUE<<msg<<RESET"\"";
        std::cout<<std::endl;
}

void app_error(char const* file, int line,
               const std::string& i_was, const std::string& msg)
{
        std::cout<<BOLD RED"ERROR "RESET
                 <<BOLD<<file<<NOBOLD":"BOLD<<line<<NOBOLD
                 <<": \""BOLD YELLOW<<msg<<RESET"\"";
        if (i_was!="")
                std::cout<<" ("BOLD YELLOW<<i_was<<RESET")";
        std::cout<<std::endl;
}

void app_line(const std::string &msg)
{
        std::cout<<BOLD<<msg<<NOBOLD<<std::endl;
}

void app_fatal()
{
        abort();
}

int main(int argc, char *argv[])
{
        if (argc!=2) {
                std::cerr<<"Usage: "<<argv[0]<<" <handling.cfg file>"<<std::endl;
                return EXIT_FAILURE;
        }

        try {

                std::ifstream handlingfstream;
                std::istream *handlingstream = &handlingfstream;
                std::string filename;

                if (strcmp(argv[1],"-")==0) {
                        handlingstream = &std::cin;
                        filename = "<stdin>";
                } else {
                        filename = argv[1];
                        handlingfstream.open (filename.c_str());
                        ASSERT_IO_SUCCESSFUL(handlingfstream,
                                          "Opening handling: "+filename);
                        if (handlingfstream.fail() || handlingfstream.bad()) {
                                std::stringstream ss;
                                ss << filename << ": IO Error: " << strerror(errno) << "\n";
                                IOS_EXCEPT(ss.str());
                        }
                }

                Csv handling;
                handling.filename = filename;
                read_csv(*handlingstream,handling);
                HandlingData data;
                read_handling(handling, data);

                for (HandlingData::iterator i=data.begin(), i_=data.end() ; i!=i_ ; ++i) {
                        const std::string name = i->first;
                        const VehicleData &v = *i->second;

                        ASSERT(name == v.name);

                        std::cout << name << std::endl;
                }

        } catch (Exception &e) {

                std::cerr << "ERROR: " << e.getFullDescription() << std::endl;
                return EXIT_FAILURE;

        }

        return EXIT_SUCCESS;
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
