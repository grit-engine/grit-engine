/* Copyright (c) Alexey Shmakov and the Grit Game Engine project 2014
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
	
	uint32_t total_decoded_size(); //returns a size of buffer needed to hold the whole decoded pcm for all channels
	
	uint32_t raw_total();
	uint32_t pcm_total();
	double time_total(); //total size is in seconds
	
	uint32_t raw_seek(uint32_t pos); //when speed is priority use of this function is recommended
	uint32_t pcm_seek(uint32_t pos);
	uint32_t time_seek(double pos); //pos is in seconds
	
	uint32_t raw_tell();
	uint32_t pcm_tell();
	double time_tell();
	
	long read(char *buffer, int length); //note this actually reads 2 byte samples
	//TODO: expose ov_read_float()
private:
	const std::string &name; //used in thrown exceptions
	const Ogre::DataStreamPtr &file;
	
	bool prepared; //is initial headers were read?
	
	vorbis_info* info;
	
	OggVorbis_File vf; //needed for vorbis file decoding
	
	ov_callbacks callbacks;
};

#endif //OGG_VORBIS_DECODER_H
