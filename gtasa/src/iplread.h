#ifndef IPLREAD_H
#define IPLREAD_H

#include <string>
#include <vector>

struct Inst {
        unsigned long id;
        std::string dff;
        unsigned long interior;
        double x,y,z, rx,ry,rz,rw;
        long near_for;
        bool is_low_detail;
};

typedef std::vector<struct Inst> Insts;

class IPL {
    public:
        virtual void addMore (std::istream &f);
        virtual const Insts &getInsts (void) const { return insts; }
        virtual const std::string &getName (void) const { return name; }
        virtual void setName (const std::string &v) { name = v; }
    protected:
        virtual void addText (std::istream &f);
        virtual void addBinary (std::istream &f);
        Insts insts;
        std::string name;
};

#endif


// vim: shiftwidth=8:tabstop=8:expandtab
