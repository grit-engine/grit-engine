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

#include "../SharedPtr.h"

class AudioSource;
typedef SharedPtr<AudioSource> AudioSourcePtr;

#ifndef audio_h
#define audio_h

#include <AL/al.h>

#include "../math_util.h"
#include "../BackgroundLoader.h"

void audio_init (void);
void audio_set_listener (const Vector3& position, const Vector3& velocity, const Quaternion& rotation);
void audio_play (const std::string& filename, const Vector3& position);
void audio_update (void);

void audio_master_volume (float v);
float audio_master_volume (void);

class AudioSource
{
	private:
		Demand demand;
		std::string resourceName;

		bool hasFile;

		Vector3 position;
		Vector3 velocity;
		bool looping;

		ALuint alSource;
	public:
		AudioSource (const std::string &filename);
		~AudioSource () { };

		static AudioSourcePtr make (const std::string &filename)
		{ return AudioSourcePtr(new AudioSource(filename)); }

		Vector3 getPosition (void) { return position; }
		Vector3 getVelocity (void) { return velocity; }
		bool getLoop (void) { return looping; }

		void setPosition (const Vector3& position);
		void setVelocity (const Vector3& velocity);
		void setLoop (bool looping);
		void setVolume (float volume);
		void setPitch (float pitch);

		bool playing (void);
		void play (void);
		void pause (void);
		void stop (void);

		void destroy (void);

		friend class SharedPtr<AudioSource>;
};


#endif
