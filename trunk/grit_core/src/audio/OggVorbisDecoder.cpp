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
/*
	name = name;
	file = file;
	prepared = false;
*/
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

long OggVorbisDecoder::decode(uint8_t *buffer, int length, int bytes_per_sample)
{
	if(!prepared) prepare();

	int current_section;

	long bytes_read = ov_read(&vf, reinterpret_cast<char*>(buffer), length, 0/*0 = little endian, 1 = big endian*/, bytes_per_sample/*1 = 8 bit samples, 2 = 16 bit samples*/, 0/*0 = unsigned samples, 1 = signed samples*/, &current_section);
	
	if(bytes_read < 0){	
			GRIT_EXCEPT("Error during decoding Ogg file \""+name+"\"");
	}
	
	return bytes_read;
}
