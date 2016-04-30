/* Copyright (c) David Cunningham and the Grit Game Engine project 2015
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

class AudioDiskResource;

#ifndef AUDIO_DISK_RESOURCE_H
#define AUDIO_DISK_RESOURCE_H

#include <AL/al.h>

#include <OgreResourceGroupManager.h>

#include <centralised_log.h>
#include "../background_loader.h"

/** A disk resource that represents a sound file on disk.  Currently only .wav is supported.  Can be stereo or mono (detected at load time).
 */
class AudioDiskResource : public DiskResource {

public:
    /** To create a resource, call disk_resource_get_or_make.  This function is for internal use only. */
	AudioDiskResource (const std::string &name)
		: name(name), stereo(false)
	{
	}

    /** Specialised loading functionality for audio files. */
	virtual void loadImpl (void);

    /** Specialised unloading functionality for audio files. */
	virtual void unloadImpl (void);

    /** The name of the resource, i.e. the filename on disk as an absolute Grit path. */
	virtual const std::string &getName (void) const { return name; }

    /** A buffer containing either both channels interleaved (in the case of stereo) or just the single channel. */
	ALuint getALBufferAll(void) { return stereo ? alBuffer : alBufferLeft; }

    /** A buffer containing just the left channel. */
	ALuint getALBufferLeft(void) { return alBufferLeft; }

    /** A buffer containing just the right channel, or 0 if sound is mono. */
	ALuint getALBufferRight(void) { return alBufferRight; }

    /** Is the loaded file a stereo one?  Discovered at loading time. */
    bool getStereo (void) { return stereo; }

private:

    /** Utility function to load a PCM file with .wav header from the byte stream. */
	void loadWAV (Ogre::DataStreamPtr &file);
	
    /** Utility function to load and decode Ogg Vorbis file. */
	void loadOGG (Ogre::DataStreamPtr &file);
	
    /** Cache of the name. */
	const std::string name;

    /** The buffer containing interleaved left and right channels, used for stereo playback.  If resource is not stereo, is unused. */
	ALuint alBuffer;

    /** The buffer containing only the left channel, or the mono channel in the case of a mono resource. */
	ALuint alBufferLeft;

    /** The buffer containing only the right channel, unused if the resource is not stereo. */
	ALuint alBufferRight;

    /** Is the loaded file a stereo one? */
    bool stereo;

};

#endif
