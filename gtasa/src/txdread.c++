#include <fstream>
#include <string>
#include <algorithm>
#include <vector>
#include <locale>

#include "ios_util.h"
#include "txdread.h"

//DDS HEADER SIZE
#define HDSZ (124)
#define HDSZT (124+4)

#define DDSD_CAPS        0x00000001 // required
#define DDSD_HEIGHT      0x00000002 // required
#define DDSD_WIDTH       0x00000004 // required
#define DDSD_PITCH       0x00000008 // used for uncompressed textures
#define DDSD_PIXELFORMAT 0x00001000 // required
#define DDSD_MIPMAPCOUNT 0x00020000
#define DDSD_LINEARSIZE  0x00080000 // used for compressed textures
#define DDSD_DEPTH       0x00800000 // don't need this

#define DDPF_ALPHAPIXELS 0x00000001
#define DDPF_FOURCC 0x00000004
#define DDPF_RGB 0x00000040

#define DDSCAPS_COMPLEX 0x00000008
#define DDSCAPS_TEXTURE 0x00001000
#define DDSCAPS_MIPMAP 0x00400000
#define DDSCAPS2_CUBEMAP 0x00000200
#define DDSCAPS2_CUBEMAP_POSITIVEX 0x00000400
#define DDSCAPS2_CUBEMAP_NEGATIVEX 0x00000800
#define DDSCAPS2_CUBEMAP_POSITIVEY 0x00001000
#define DDSCAPS2_CUBEMAP_NEGATIVEY 0x00002000
#define DDSCAPS2_CUBEMAP_POSITIVEZ 0x00004000
#define DDSCAPS2_CUBEMAP_NEGATIVEZ 0x00008000
#define DDSCAPS2_VOLUME 0x00200000


#define RWDATA 0x1
#define RWEXT 0x3
#define RWTEX 0x15
#define RWTEXDICT 0x16

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

// these are for writing to memory blocks

static int tolowr (int c)
{
        return std::tolower(char(c),std::cout.getloc());
}

static std::string& strlower (std::string& s)
{
        std::transform(s.begin(),s.end(), s.begin(),tolowr);
        return s;
}

void Txd::readTx (std::istream &f,
                  const std::string &dest_dir,
                  unsigned long file_ver,
                  bool output)
{

        unsigned long type, texsize;
        ios_read_header(f,&type,&texsize,NULL,&file_ver);
        ASSERT(type==RWTEX);


        unsigned long size;
        ios_read_header(f,&type,&size,NULL,&file_ver);
        ASSERT(type==RWDATA);

        unsigned long vers = ios_read_u32(f);
        ASSERT(vers==8 || vers==9); // always the case in gta_sa

        unsigned long filter_flags = ios_read_u32(f);
        ASSERT(filter_flags==0x1101 || filter_flags==0x1102 ||
               filter_flags==0x1106 || filter_flags==0x1206 ||
               filter_flags==0x2106);

        std::string tex_name = ios_read_fixedstr(f,32);
        strlower(tex_name);
        names.insert(tex_name);

        // don't know what this is for?
        std::string alpha_name = ios_read_fixedstr(f,32);

        unsigned long alpha_flags = ios_read_u32(f);
        // don't know what these mean
        ASSERT(alpha_flags==0x100 || alpha_flags==0x200 ||
               alpha_flags==0x300 || alpha_flags==0x500 ||
               alpha_flags==0x600 || alpha_flags==0x2600 ||
               alpha_flags==0x8200);

        unsigned long d3d_tex_format = ios_read_u32(f);
        // 0 is unknown, 0x15 and 0x16 are raw with/without alpha
        // the other two are DXT1 and DXT3 (code is ascii DWORD)
        ASSERT(d3d_tex_format==0 || d3d_tex_format==0x15 || d3d_tex_format==0x16
            || d3d_tex_format==0x31545844 || d3d_tex_format==0x33545844);

        bool compressed= d3d_tex_format==0x31545844||d3d_tex_format==0x33545844;

        unsigned short width = ios_read_u16(f);
        unsigned short height = ios_read_u16(f);

        unsigned char depth = ios_read_u8(f);
        // no 24 bit textures in gta sa
        ASSERT(depth==8 || depth==16 || depth==32);

        unsigned char levels = ios_read_u8(f);
        ASSERT(levels>0);

        unsigned char tex_code_type = ios_read_u8(f);
        ASSERT(tex_code_type==4); // don't know what this means

        unsigned char flags = ios_read_u8(f);
        // bit 1 = has alpha, bit 3 = compressed
        ASSERT(flags==0x0 || flags==0x1 || flags==0x8 || flags==0x9); 

        unsigned char palette_r[256];
        unsigned char palette_g[256];
        unsigned char palette_b[256];
        unsigned char palette_a[256];
        if (depth==8) {
                for (int i=0 ; i<256 ; i++) {
                        palette_r[i] = ios_read_u8(f);
                        palette_g[i] = ios_read_u8(f);
                        palette_b[i] = ios_read_u8(f);
                        palette_a[i] = ios_read_u8(f);
                }
        }

        // useful: "DDS File Reference", found at:
        // http://msdn2.microsoft.com/en-us/library/bb172993.aspx

        if (d3d_tex_format==0) {
                ASSERT(vers==8);
                if (depth==16) {
                        ASSERT(flags==0x1);
                        d3d_tex_format = 0x31545844; //dxt1
                } else if (depth==32) {
                        ASSERT(flags==0x0);
                        d3d_tex_format = 0x15;
                } else {
                        IOS_EXCEPT("unrecognised file type");
                }
        }

        if (!output) {
                for (unsigned char i=0 ; i<levels ; i++) {
                        unsigned long imgsize = ios_read_u32(f);
                        ios_read_byte_array(f, NULL, imgsize);
                }
                ios_read_header(f,&type,&size,NULL,&file_ver);
                ASSERT(type==RWEXT);
                ASSERT(size==0);
                return;
        }

        unsigned long imgsize = ios_read_u32(f);

        std::ofstream ddsf;
        std::string ddsname = dest_dir + "/" + tex_name + ".dds";
        ddsf.open(ddsname.c_str(),std::ios::binary);
        ASSERT_IO_SUCCESSFUL(ddsf,"creating new dds file: \""+ddsname+"\"");

        bool has_alpha = flags&1;

        ios_write_u32(ddsf,0x20534444); // "DDS "
        ios_write_u32(ddsf,HDSZ);
        unsigned long ddsflags = 0;
        ddsflags |= DDSD_CAPS | DDSD_PIXELFORMAT | DDSD_WIDTH | DDSD_HEIGHT;
        //ddsflags |= compressed ? DDSD_LINEARSIZE : DDSD_PITCH;
        ddsflags |= DDSD_MIPMAPCOUNT;
        ios_write_u32(ddsf,ddsflags);
        ios_write_u32(ddsf,height);
        ios_write_u32(ddsf,width);
        ios_write_u32(ddsf,0); //compressed?imgsize:width*depth/8);
        ios_write_u32(ddsf,0); // not a volume texture
        ios_write_u32(ddsf, levels);
        for (int i=0 ; i<11 ; i++)
                ios_write_u32(ddsf,0); // "reserved"

        // pixelformat structure
        ios_write_u32(ddsf,32); // size of struct that follows, always 32
        ddsflags = compressed ? DDPF_FOURCC : DDPF_RGB;
        ddsflags |= !compressed && has_alpha ? DDPF_ALPHAPIXELS : 0;
        ios_write_u32(ddsf,ddsflags);
        ios_write_u32(ddsf,compressed?d3d_tex_format:0);
        if (depth==8) {
                // will convert to 32 bit later
                ios_write_u32(ddsf,32);
        } else {
                ASSERT(depth==16 || depth==32);
                ios_write_u32(ddsf,compressed?0:depth);
        }
        if (compressed) {
                ios_write_u32(ddsf,0);
                ios_write_u32(ddsf,0);
                ios_write_u32(ddsf,0);
                ios_write_u32(ddsf,0);
        } else {
                ios_write_u32(ddsf,0x00FF0000); //r
                ios_write_u32(ddsf,0x0000FF00); //g
                ios_write_u32(ddsf,0x000000FF); //b
                ios_write_u32(ddsf,has_alpha ? 0xFF000000 : 0x00000000); //a
        }

        // capabilities structure
        ios_write_u32(ddsf,DDSCAPS_TEXTURE|DDSCAPS_MIPMAP|DDSCAPS_COMPLEX);
        ios_write_u32(ddsf,0); // not a cube/volume map
        ios_write_u32(ddsf,0); // "reserved"
        ios_write_u32(ddsf,0); // "reserved"

        // unused
        ios_write_u32(ddsf,0);

        for (unsigned char i=0 ; i<levels ; i++) {
                if (i>0) {
                        imgsize = ios_read_u32(f);
                }

                if (imgsize==0 && compressed) {
                        // sometimes txds do not give a mipmap level
                        // if it is smaller (width or height) than 4 pixels
                        // as dxt1-3 are stored in 4x4 pixel chunks
                        // so we generate black mipmap levels here
                        unsigned long mmwidth = width, mmheight=height;
                        for (unsigned char j=0 ; j<i ; ++j) {
                                mmwidth = std::max(1UL,mmwidth/2);
                                mmheight = std::max(1UL,mmheight/2);
                        }
                        // we need width/height measured in whole 4x4 blocks
                        mmwidth = (mmwidth+3)/4;
                        mmheight = (mmheight+3)/4;
                        unsigned long blocks = mmwidth * mmheight;
                        const unsigned char block8[] = {0,0,0,0,
                                                       0,0,0,0};
                        const unsigned char block16[] = {255,255,0,0,
                                                       0,0,0,0,
                                                       0,0,0,0,
                                                       0,0,0,0};
                        switch (d3d_tex_format) {
                                case 0x31545844: // dxt1: 8 bytes per block
                                for (unsigned long j=0 ; j<blocks ; ++j) {
                                        ios_write_byte_array(ddsf,block8,8);
                                }
                                break;
                                case 0x33545844: // dxt3: 16 bytes per block
                                for (unsigned long j=0 ; j<blocks ; ++j) {
                                        ios_write_byte_array(ddsf,block16,16);
                                }
                        }
                        continue;
                }

                unsigned char *dds = new unsigned char[imgsize];
                ios_read_byte_array(f, dds, imgsize);

                // convert evil 8bit images to sensible 32bit images
                if (depth==8) {
                        unsigned char *new_dds = new unsigned char[imgsize*4];
                        for (size_t i=0 ; i<imgsize ; i++) {
                                unsigned char c = dds[i];
                                new_dds[4*i+0] = palette_b[c];
                                new_dds[4*i+1] = palette_g[c];
                                new_dds[4*i+2] = palette_r[c];
                                new_dds[4*i+3] = palette_a[c];
                        }
                        imgsize *= 4;
                        delete [] dds;
                        dds = new_dds;
                }

                ios_write_byte_array(ddsf,dds,imgsize);

                delete [] dds;
        }

        // these 3 lines are duplicated at the early return, above
        ios_read_header(f,&type,&size,NULL,&file_ver);
        ASSERT(type==RWEXT);
        ASSERT(size==0);

}


Txd::Txd (std::istream &f, const std::string &dest_dir, bool output)
{
        unsigned long type, txdsize, file_ver;
        ios_read_header(f,&type,&txdsize,&file_ver,NULL);
        ASSERT(type==RWTEXDICT);

        unsigned long size;
        ios_read_header(f,&type,&size,NULL,&file_ver);
        ASSERT(type==RWDATA);

        unsigned long num_files = ios_read_u16(f);
        ios_read_u16(f);

        for (unsigned long i=0 ; i<num_files ; i++) {
                readTx(f, dest_dir, file_ver, output);
        }

        ios_read_header(f,&type,&size,NULL,&file_ver);
        ASSERT(type==RWEXT);
        ASSERT(size==0);

}

// vim: shiftwidth=8:tabstop=8:expandtab
