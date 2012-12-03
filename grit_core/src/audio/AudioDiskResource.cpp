/* Copyright (c) David Cunningham and the Grit Game Engine project 2012
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

#include "AudioDiskResource.h"
#include "audio.h"
#include "../portable_io.h"

void AudioDiskResource::loadImpl (void)
{
	// head radio. because your head NEEDS IT.

	// why using OGRE anyways? I'm just following CollisionMesh.c++'s example and doing so here
    // [DC] we are still using ogre's IO code, that understands putting files in zips etc.
	Ogre::DataStreamPtr file;
    try {
        file = Ogre::ResourceGroupManager::getSingleton().openResource(name.substr(1),"GRIT");
    } catch (Ogre::Exception &e) {
        GRIT_EXCEPT(e.getDescription());
    }

	uint32_t fourcc = 0;
	file->read(&fourcc, sizeof(fourcc));

	if (fourcc == 0x46464952) // RIFF
	{
		file->seek(0);
		loadWAV(file);
	}
	else
	{
		GRIT_EXCEPT("Unknown audio file format in " + name);
	}

}

struct chunk_header
{
	uint32_t fourcc;
	uint32_t size;
};

struct riff_header
{
	chunk_header chunk;
	uint32_t format;
};

struct wave_format
{
	uint16_t format_tag;
	uint16_t channels;
	uint32_t samples;
	uint32_t bytesPerSec;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	uint16_t size;
};

void AudioDiskResource::loadWAV(Ogre::DataStreamPtr &file)
{
	riff_header header;
	file->read(&header, sizeof(header));

	if (header.format != 0x45564157) // WAVE
	{
		GRIT_EXCEPT(name + " is not a WAVE file.");
	}

	// FIXME: possibly use more advanced chunk reading?
	chunk_header fmtHeader;
	file->read(&fmtHeader, sizeof(fmtHeader));

	if (fmtHeader.fourcc != 0x20746d66)
	{
		GRIT_EXCEPT("First chunk in " + name + " wasn't 'fmt'.");
	}

	// save the fmt position so we can seek from it later on
	size_t fmtStart = file->tell();

	// read the format header
	wave_format fmt;
	file->read(&fmt, sizeof(fmt));

	if (fmt.format_tag != 1) // PCM
	{
		GRIT_EXCEPT(name + " isn't PCM (only PCM is supported right now)");
	}

	if (fmt.channels > 2)
	{
		GRIT_EXCEPT(name + " has more than two channels.");
	}

	if (fmt.bitsPerSample != 8 && fmt.bitsPerSample != 16)
	{
		GRIT_EXCEPT(name + " isn't 8/16 bits per sample.");
	}

	// seek to the data start
	file->seek(fmtStart + fmtHeader.size);

	// read the data and the data header
	chunk_header dataHeader;
	file->read(&dataHeader, sizeof(dataHeader));

	if (dataHeader.fourcc != 0x61746164)
	{
		GRIT_EXCEPT("Second chunk in " + name + " wasn't 'data'.");
	}

    if (fmt.channels == 0) {
        GRIT_EXCEPT("Wave file has zero channels: \""+name+"\"");
    }

    if (fmt.channels > 2) {
        GRIT_EXCEPT("Wave file has too many channels: \""+name+"\"");
    }

    size_t bytes_per_sample = fmt.bitsPerSample/8;
    size_t samples = dataHeader.size / fmt.channels / bytes_per_sample;

	uint8_t* data = new uint8_t[dataHeader.size];
	file->read(data, dataHeader.size);

	// generate an AL buffer

    alBufferLeft = 0;
    alBufferRight = 0;
    alBuffer = 0;

    ALenum mono_format = (fmt.bitsPerSample == 8) ? AL_FORMAT_MONO8 : AL_FORMAT_MONO16;
    ALenum stereo_format = (fmt.bitsPerSample == 8) ? AL_FORMAT_STEREO8 : AL_FORMAT_STEREO16;

	if (fmt.channels == 1) {
        stereo = false;
        alGenBuffers(1, &alBufferLeft);
        alBufferData(alBuffer, mono_format, data, dataHeader.size, fmt.samples);
	} else {
        stereo = true;
        alGenBuffers(1, &alBuffer);
        alBufferData(alBuffer, stereo_format, data, dataHeader.size, fmt.samples);
        uint8_t* data1 = new uint8_t[dataHeader.size / 2];
        for (size_t i=0 ; i<samples ; ++i) memcpy(&data1[i*bytes_per_sample], &data[2*i*bytes_per_sample], bytes_per_sample);
        alGenBuffers(1, &alBufferLeft);
        alBufferData(alBufferLeft, mono_format, data1, dataHeader.size/2, fmt.samples);
        for (size_t i=0 ; i<samples ; ++i) memcpy(&data1[i*bytes_per_sample], &data[(2*i+1)*bytes_per_sample], bytes_per_sample);
        alGenBuffers(1, &alBufferRight);
        alBufferData(alBufferRight, mono_format, data1, dataHeader.size/2, fmt.samples);
        delete[] data1;
    }


	// free the data (as memory is in the AL buffer now)
	delete[] data;

	// close the stream
	file->close();
}

void AudioDiskResource::unloadImpl (void)
{
	if (alBuffer != 0) alDeleteBuffers(1, &alBuffer);
	if (alBufferLeft != 0) alDeleteBuffers(1, &alBufferLeft);
	if (alBufferRight != 0) alDeleteBuffers(1, &alBufferRight);
}

bool AudioDiskResource::isGPUResource (void)
{
	return false;
}
