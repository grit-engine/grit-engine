#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>

#include "matbin.h"
#include "ogre_datastream_util.h"

#define myassert(e) if (!(e)) {\
        fprintf(stderr,"[0;1;33m%s failed! (%s:%d)[0m\n",\
                      #e,__FILE__,__LINE__); \
        perror("perror says"); \
        exit(EXIT_FAILURE); \
} else {}

struct mat_bin read_matbin(const Ogre::DataStreamPtr &f)
{
        struct mat_bin r;
        unsigned long fourcc = ods_read_u32(f);
        myassert(fourcc==0x4254414d); //"MATB"
        unsigned long version = ods_read_u16(f);
        myassert(version==1);
        r.flags = ods_read_u8(f);
        unsigned long len = ods_read_u32(f);
        r.name = ods_read_fixedstr(f,len);
        r.colour = ods_read_u32(f);
        len = ods_read_u32(f);
        r.tname = ods_read_fixedstr(f,len);
        return r;
}

std::vector<struct mat_bin> read_matbins (const Ogre::DataStreamPtr &f)
{
        std::vector<struct mat_bin> mats;
        size_t sz = f->size();
        while (f->tell()!=sz) {
                mats.push_back(read_matbin(f));
        }
        return mats;
}

#ifdef _MATBIN_TEST

size_t amount_read = 0;
size_t amount_seeked = 0;

int main(int argc, char **argv)
{
        try {
                if (argc!=2) {
                        std::cerr<<"Usage: "<<argv[0]<<" <matbin>"<<std::endl;
                        return EXIT_FAILURE;
                }

                std::ifstream *in = new std::ifstream();
                in->open(argv[1], std::ios::binary);
                if (!in->good()) {
                        char *error = strerror(errno);
                        OGRE_EXCEPT(Ogre::Exception::ERR_FILE_NOT_FOUND, error,"main");
                }

                Ogre::DataStreamPtr f =
                      Ogre::DataStreamPtr(new Ogre::FileStreamDataStream(argv[1],in));

                std::vector<struct mat_bin> mats = read_matbins(f);

                if (!getenv("FISH")) {
                        size_t num = mats.size();
                        for (size_t i=0 ; i<num ; i++) {
                                struct mat_bin& m = mats[i];
                                std::cout<<"\""<<m.name<<"\" 0x"<<std::hex<<(int)m.flags<<" 0x"<<std::setw(8)<<m.colour<<" \""<<m.tname<<"\""<<std::endl;
                        }
                } else {
                        size_t num = mats.size();
                        for (size_t i=0 ; i<num ; i++) {
                                struct mat_bin& m = mats[i];
                                std::cout<<"material "<<"\""<<m.name<<"\" { ";
                                if (m.flags & 0x1 || m.flags & 0x8) std::cout<<"alpha=true, ";
                                if (m.flags & 0x8) std::cout<<"alphaReject=0, ";
                                if (m.flags & 0x2) std::cout<<"backfaces=true, ";
                                if (!(m.flags & 0x4)) std::cout<<"normals=false, ";
                                if (m.colour!=0xffffffff)
                                        std::cout<<"diffuseColour=0x"
                                                 <<std::hex<<std::setw(8)<<m.colour<<", ";
                                if (m.tname!="")
                                        std::cout<<"diffuseMap=\""<<m.tname<<"\", ";
                                std::cout<<"}"<<std::endl;
                                
                        }
                }

        } catch (Ogre::Exception &e) {
        
                std::cerr << "ERROR: " << e.getFullDescription() << std::endl;

                return EXIT_FAILURE;

        }


        return EXIT_SUCCESS;
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
