#include <string>
#include <vector>
#include <map>
#include <fstream>

class Img {

    public:

        void init (std::istream &f, std::string name_="IMG file");

        virtual const std::string &fileName (unsigned long i) const;

        virtual bool fileExists (const std::string &fname) const;

        virtual void fileOffset (std::istream &f, unsigned long i) const;

        virtual void fileOffset (std::istream &f,const std::string &fname)const;

        virtual unsigned long fileOffset (unsigned long i) const;

        virtual unsigned long fileOffset (const std::string &fname)const;

        virtual unsigned long fileSize (unsigned long i) const;

        virtual unsigned long fileSize (const std::string &fname) const;

        virtual unsigned long size (void) const { return numFiles; }


    protected:

        std::string name;

        unsigned long numFiles;

        std::vector<std::string> names;

        std::vector<unsigned long> offsets;

        std::vector<unsigned long> sizes;

        typedef std::map<std::string,unsigned long> Dir;
        Dir dir;

        Dir::const_iterator find (const std::string &fname) const;

};

// vim: shiftwidth=8:tabstop=8:expandtab
