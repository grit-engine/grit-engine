
#ifndef OGG_VORBIS_DECODER_H
#define OGG_VORBIS_DECODER_H

#include <string>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <OgreResourceGroupManager.h>
#include "../CentralisedLog.h"

class OggVorbisDecoder {
public:
	OggVorbisDecoder (const std::string& name, const Ogre::DataStreamPtr &file);
	~OggVorbisDecoder();
	void prepare(); //read initial headers
	
	bool isOggVorbis();
	bool stereo();
	int rate();
	
	long decode(uint8_t *buffer, int length, int bytes_per_sample);
	
private:
	const std::string &name; //used in thrown exceptions
	const Ogre::DataStreamPtr &file;
	
	bool prepared; //is initial headers were read?
	
	vorbis_info* info;
	
	OggVorbis_File vf; //needed for vorbis file decoding
	
	ov_callbacks callbacks;
};

#endif //OGG_VORBIS_DECODER_H
