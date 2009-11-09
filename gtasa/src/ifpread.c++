#include <math.h>

#include <sstream>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include "ios_util.h"

void ifpread(std::istream &f, std::string p)
{
        unsigned long fourcc = ios_read_u32(f);
        ASSERT(fourcc==0x33504e41); // ANP3

        unsigned long body_size = ios_read_u32(f);
        std::cout<<p<<".body_size: "<<body_size<<"\n";
        std::string filename = ios_read_fixedstr(f,24);
        std::cout<<p<<".filename: "<<filename<<"\n";

        unsigned long num_anims = ios_read_u32(f);
        std::cout<<p<<".num_anims: "<<num_anims<<"\n";

        for (unsigned long i=0 ; i<num_anims ; i++) {
                std::stringstream p2_;
                p2_ << p << ".anim[" << i << "]";
                std::string p2(p2_.str());
                std::string anim_name = ios_read_fixedstr(f,24);
                std::cout<<p2<<".anim_name: "<<anim_name<<"\n";
                unsigned long num_bones = ios_read_u32(f);
                std::cout<<p2<<".num_bones: "<<num_bones<<"\n";
                unsigned long size = ios_read_u32(f);
                std::cout<<p2<<".anim_size: "<<size<<"\n";
                unsigned long one = ios_read_u32(f);
                ASSERT(one==1);
                for (unsigned long j=0 ; j<num_bones ; j++) {
                        std::stringstream p3_;
                        p3_ << p2 << ".bone["<<j<<"]";
                        std::string p3(p3_.str());
                        std::string bone_name = ios_read_fixedstr(f,24);
                        std::cout<<p3<<".bone_name: "<<bone_name<<"\n";
                        unsigned long frame_type = ios_read_u32(f);
                        std::cout<<p3<<".frame_type: "<<frame_type<<"\n";
                        ASSERT(frame_type==3 || frame_type==4);
                        unsigned long num_frames = ios_read_u32(f);
                        std::cout<<p3<<".num_frames: "<<num_frames<<"\n";
                        unsigned long bone_id = ios_read_u32(f);
                        std::cout<<p3<<".bone_id: "<<bone_id<<"\n";
                        for (unsigned long k=0 ; k<num_frames ; k++) {
                                std::stringstream p4_;
                                p4_ << p3 << ".frame["<<k<<"]";
                                std::string p4(p4_.str());
                                float qx = ios_read_s16(f) / 4096.0f;
                                float qy = ios_read_s16(f) / 4096.0f;
                                float qz = ios_read_s16(f) / 4096.0f;
                                float qw = ios_read_s16(f) / 4096.0f;
                                std::cout<<p4<<".quat_x: "<<qx<<"\n";
                                std::cout<<p4<<".quat_y: "<<qy<<"\n";
                                std::cout<<p4<<".quat_z: "<<qz<<"\n";
                                std::cout<<p4<<".quat_w: "<<qw<<"\n";
                                float time = ios_read_u16(f) / 60.0f;
                                std::cout<<p4<<".time: "<<time<<"\n";
                                if (frame_type==4) {
                                        float px = ios_read_s16(f) / 1024.0f;
                                        float py = ios_read_s16(f) / 1024.0f;
                                        float pz = ios_read_s16(f) / 1024.0f;
                                        std::cout<<p4<<".pos_x: "<<px<<"\n";
                                        std::cout<<p4<<".pos_y: "<<py<<"\n";
                                        std::cout<<p4<<".pos_z: "<<pz<<"\n";
                                }
                        }
                }
        }
}



#ifdef _IFPREAD_EXEC

size_t amount_read = 0;
size_t amount_seeked = 0;

int main(int argc, char *argv[])
{
        if (argc!=2) {
                std::cerr<<"Usage: "<<argv[0]<<" <ifp file>"<<std::endl;
                return EXIT_FAILURE;
        }

        try {

                std::string ifp_name = argv[1];

                std::ifstream f;
                f.open(ifp_name.c_str(),std::ios::binary);
                ASSERT_IO_SUCCESSFUL(f,"Opening ifp file: "+ifp_name);

                ifpread(f,ifp_name);

        } catch (Exception &e) {
                
                std::cerr << "ERROR: " << e.getFullDescription() << std::endl;
                return EXIT_FAILURE;

        }



        return EXIT_SUCCESS;
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
