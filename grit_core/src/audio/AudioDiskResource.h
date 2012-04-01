/* Copyright (c) David Cunningham and the Grit Game Engine project 2010
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

#include "../CentralisedLog.h"
#include "../BackgroundLoader.h"

#include <OgreResourceGroupManager.h>

class AudioDiskResource : public DiskResource {

public:
	AudioDiskResource (const std::string &name)
		: name(name)
	{
	}


	virtual void load (void);

	virtual void unload (void);

	virtual bool isGPUResource (void);

	virtual const std::string &getName (void) const { return name; }

	ALuint getALBuffer(void) { return alBuffer; }

private:

	void loadWAV (Ogre::DataStreamPtr &file);
	
	const std::string name;

	ALuint alBuffer;

};

#endif
