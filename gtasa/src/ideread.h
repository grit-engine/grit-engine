#ifndef IDEREAD_H
#define IDEREAD_H


#include <string>
#include <vector>
#include <istream>

#define OBJ_FLAG_WET 1<<0 // 1 0x1
#define OBJ_FLAG_NIGHT 1<<1 // 2 0x2
#define OBJ_FLAG_ALPHA1 1<<2 // 4 0x4
#define OBJ_FLAG_ALPHA2 1<<3 // 8 0x8
#define OBJ_FLAG_DAY 1<<4 // 16 0x10
#define OBJ_FLAG_INTERIOR 1<<5 // 32 0x20   door?
#define OBJ_FLAG_NO_SHADOW 1<<6 // 64 0x40
#define OBJ_FLAG_NO_COL 1<<7 // 128 0x80
#define OBJ_FLAG_NO_DRAW_DIST 1<<8 // 256 0x100
#define OBJ_FLAG_BREAK_GLASS 1<<9 // 512 0x200
#define OBJ_FLAG_BREAK_GLASS_CRACK 1<<10 // 1024 0x400
#define OBJ_FLAG_GARAGE_DOOR 1<<11 // 2048 0x800
#define OBJ_FLAG_2CLUMP 1<<12 // 4096 0x1000
#define OBJ_FLAG_SWAYS 1<<13 // 8192 0x2000
#define OBJ_FLAG_OTHER_VEG 1<<14 // 16384 0x4000
#define OBJ_FLAG_POLE_SHADOW 1<<15 // 32768 0x8000
#define OBJ_FLAG_EXPLOSIVE 1<<16 // 65536 0x10000
#define OBJ_FLAG_UNK1 1<<17 // 131072 0x20000
#define OBJ_FLAG_UNK2 1<<18 // 262144 0x40000
#define OBJ_FLAG_UNK3 1<<19 // 524288 0x80000
#define OBJ_FLAG_GRAFITTI 1<<20 // 1048576 0x100000
#define OBJ_FLAG_DRAW_BACKFACE 1<<21 // 2097152 0x200000
#define OBJ_FLAG_UNK4 1<<22 // 4194304 0x400000

class Obj {
public:
        unsigned long id;
        std::string dff;
        std::string txd;
        float draw_distance;
        unsigned long flags;
};
typedef std::vector<Obj> Objs;

class TObj : public Obj {
public:
        unsigned char hour_on;
        unsigned char hour_off;
};
typedef std::vector<TObj> TObjs;

class Anim : public Obj {
public:
        std::string ifp_file;
};
typedef std::vector<Anim> Anims;

class TXDP {
public:
        std::string txd1;
        std::string txd2;
};
typedef std::vector<TXDP> TXDPs;

class Ped {
public:
        unsigned long id;
        std::string dff;
        std::string txd;
        std::string stat_type; 
        std::string type; 
        std::string anim_group;
        unsigned long can_drive; // peds.ide has the values
        bool buys_drugs; // just 0 or 1
        std::string anim_file;
        unsigned long radio1; // peds.ide has the values
        unsigned long radio2; // peds.ide has the values
        std::string unk1;
        std::string unk2;
        std::string unk3;
};
typedef std::vector<Ped> Peds;

class Vehicle {
public:
        unsigned long id;
        std::string dff;
        std::string txd;
        //bike bmx boat car heli mtruck plane quad trailer train
        std::string type; 
        std::string handling_id;
        std::string game_name; // for gxt apparently
        // BF_injection biked bikeh bikes bikev bmx bus choppa coach dozer KART
        // mtb nevada null quad rustler shamal tank truck van vortex wayfarer
        std::string anims;
        // bicycle big executive ignore leisureboat moped motorbike normal
        // poorfamily richfamily taxi worker workerboat
        std::string class_;
        unsigned int freq;
        unsigned int flags; // 0 1 or 7
        // 0 1012 1f10 1f341210 2ff0 3012 30123345 3210 3f01 3f10 3f341210 4fff
        unsigned int comp_rules;
        // boats do not have the following:
        int unk1;  // on bikes, 16 or 23, otherwise -1
        float front_wheel_size;
        float rear_wheel_size;
        int unk2;  // if present -1, 0, 1, 2 (on non-cars always -1)
};
typedef std::vector<Vehicle> Vehicles;

class Weap {
public:
        unsigned long id;
        std::string dff;
        std::string txd;
        std::string type;
        unsigned int unk_one;
        unsigned int unk_num;
        unsigned int unk_zero;
};
typedef std::vector<Weap> Weaps;

struct ide {
        Objs objs;
        TObjs tobjs;
        TXDPs txdps;
        Anims anims;
        Weaps weaps;
        Vehicles vehicles;
        Peds peds;
};

void read_ide(std::istream &f, struct ide *ide);

#endif


// vim: shiftwidth=8:tabstop=8:expandtab
