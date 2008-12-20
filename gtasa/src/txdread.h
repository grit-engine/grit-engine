#include <string>
#include <set>
#include <istream>

class Txd {

    public:

        Txd (std::istream &f, const std::string &dest_dir, bool output=true);

        typedef std::set<std::string> Names;

        const Names &getNames (void) const { return names; }

    protected:

        void readTx (std::istream &f,
                     const std::string &dest_dir,
                     unsigned long ver,
                     bool output);

        Names names;

};

// vim: shiftwidth=8:tabstop=8:expandtab
