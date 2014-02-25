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

#include "OggVorbisDecoder.h"

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *datasource)
{
	Ogre::DataStreamPtr file = * static_cast<Ogre::DataStreamPtr*>(datasource);
	return file->read(ptr, size*nmemb);
}

int seek_callback(void *datasource, ogg_int64_t offset, int type)
{
	Ogre::DataStreamPtr file = * static_cast<Ogre::DataStreamPtr*>(datasource);
	
	switch(type) {
		case SEEK_CUR:
			file->seek(file->tell()+offset);
			break;
		case SEEK_END:
			file->seek(file->size());
			break;
		case SEEK_SET:
			file->seek(offset);
			break;
	}
	return 0;
}
/*
int close_callback(void *datasource)
{
	Ogre::DataStreamPtr file = static_cast<Ogre::DataStreamPtr>(datasource);
	file->close();
	return 0;
}
*/
long tell_callback(void *datasource)
{
	Ogre::DataStreamPtr file = * static_cast<Ogre::DataStreamPtr*>(datasource);
	return file->tell();
}

OggVorbisDecoder::OggVorbisDecoder(const std::string &_name, const Ogre::DataStreamPtr &_file)
	: name(_name), file(_file), prepared(false)
{
	callbacks.read_func = read_callback;
	callbacks.seek_func = seek_callback;
	callbacks.close_func = NULL; //do nothing, will clean up my data on my own
	callbacks.tell_func = tell_callback;
}

OggVorbisDecoder::~OggVorbisDecoder()
{
	ov_clear(&vf);
}

void OggVorbisDecoder::prepare()
{
	if(ov_open_callbacks(const_cast<void*>((const void*)&file), &vf, NULL, 0, callbacks) < 0)
		GRIT_EXCEPT("File \""+name+"\" is not in Ogg format.");
		
	info = ov_info(&vf, -1);
	if(info->channels > 2)
		GRIT_EXCEPT("Ogg Vorbis file \""+name+"\" has more than two channels.");
		
	prepared = true;
}

bool OggVorbisDecoder::isOggVorbis()
{
	if(prepared) return true; //if we passed initialization successfully then we're definitely an ogg vorbis file
	if(ov_test_callbacks(const_cast<void*>((const void*)&file), &vf, NULL, 0, callbacks) == 0) return true;
	return false;
}
	
bool OggVorbisDecoder::stereo()
{
	if(!prepared) prepare();
	return (info->channels == 2);
}

int OggVorbisDecoder::rate()
{
	if(!prepared) prepare();
	return info->rate;
}

uint32_t OggVorbisDecoder::total_decoded_size(int bytes_per_sample)
{
	if ((bytes_per_sample != 2) ||  (bytes_per_sample != 1))
		GRIT_EXCEPT("bytes_per_sample can only be 1 or 2");
	if(!prepared) prepare();
	int r = ov_pcm_total(&vf, -1);
	if(r < 0)
		GRIT_EXCEPT("error getting total size for file \""+name+"\"");
	return r * info->channels * bytes_per_sample;
}

uint32_t OggVorbisDecoder::raw_total()
{
	if(!prepared) prepare();
	int r = ov_raw_total(&vf, -1);
	if(r < 0)
		GRIT_EXCEPT("error getting raw total size for file \""+name+"\"");
	return r;
}
uint32_t OggVorbisDecoder::pcm_total()
{
	if(!prepared) prepare();
	int r = ov_pcm_total(&vf, -1);
	if(r < 0)
		GRIT_EXCEPT("error getting pcm total size for file \""+name+"\"");
	return r;

}
double OggVorbisDecoder::time_total()
{
	if(!prepared) prepare();
	double r = ov_pcm_total(&vf, -1);
	if(r < 0)
		GRIT_EXCEPT("error getting time total size for file \""+name+"\"");
	return r;
}

uint32_t OggVorbisDecoder::raw_seek(uint32_t pos)
{
	if(!prepared) prepare();
	int r = ov_raw_seek(&vf, pos);
	if(r != 0)
		GRIT_EXCEPT("error raw seek for file \""+name+"\"");
	return r;
}
uint32_t OggVorbisDecoder::pcm_seek(uint32_t pos)
{
	if(!prepared) prepare();
	int r = ov_pcm_seek(&vf, pos);
	if(r != 0)
		GRIT_EXCEPT("error pcm seek for file \""+name+"\"");
	return r;
}
uint32_t OggVorbisDecoder::time_seek(double pos)
{
	if(!prepared) prepare();
	int r = ov_time_seek(&vf, pos);
	if(r != 0)
		GRIT_EXCEPT("error time seek for file \""+name+"\"");
	return r;
}

uint32_t OggVorbisDecoder::raw_tell()
{
	if(!prepared) prepare();
	int r = ov_raw_tell(&vf);
	if(r < 0)
		GRIT_EXCEPT("error raw tell for file \""+name+"\"");
	return r;
}
uint32_t OggVorbisDecoder::pcm_tell()
{
	if(!prepared) prepare();
	int r = ov_pcm_tell(&vf);
	if(r < 0)
		GRIT_EXCEPT("error pcm tell for file \""+name+"\"");
	return r;
}
double OggVorbisDecoder::time_tell()
{
	if(!prepared) prepare();
	double r = ov_time_tell(&vf);
	if(r < 0)
		GRIT_EXCEPT("error time tell for file \""+name+"\"");
	return r;
}

long OggVorbisDecoder::read(uint8_t *buffer, int length, int bytes_per_sample)
{
	if(!prepared) prepare();

	int current_section;

	long bytes_read = ov_read(&vf, reinterpret_cast<char*>(buffer), length, 0/*0 = little endian, 1 = big endian*/, bytes_per_sample/*1 = 8 bit samples, 2 = 16 bit samples*/, 0/*0 = unsigned samples, 1 = signed samples*/, &current_section);
	
	if(bytes_read < 0){	
			GRIT_EXCEPT("Error during decoding Ogg file \""+name+"\"");
	}
	
	return bytes_read;
}
