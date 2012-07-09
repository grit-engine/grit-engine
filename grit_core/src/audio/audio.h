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

#include "../SharedPtr.h"

class AudioSource;
typedef SharedPtr<AudioSource> AudioSourcePtr;

#ifndef audio_h
#define audio_h

#include <AL/al.h>

#include "../math_util.h"
#include "../BackgroundLoader.h"
#include "../DiskResource.h"

#include "AudioDiskResource.h"

enum AudioBoolOption {
    AUDIO_AUTOUPDATE,
    AUDIO_DOPPLER_ENABLED,
    AUDIO_MUTE
};

enum AudioFloatOption {
    AUDIO_MASTER_VOLUME
};

enum AudioIntOption {
    AUDIO_MAX_SOUNDS
};

void audio_init (void);
void audio_update (const Vector3& position, const Vector3& velocity, const Quaternion& rotation);
void audio_play (const std::string& filename, bool ambient, const Vector3& position, float volume, float ref_dist, float roll_off, float pitch);

std::string audio_option_to_string (AudioBoolOption o);
std::string audio_option_to_string (AudioIntOption o);
std::string audio_option_to_string (AudioFloatOption o);

// set's t to either 0,1,2 and fills in the approriate argument
void audio_option_from_string (const std::string &s,
                               int &t,
                               AudioBoolOption &o0,
                               AudioIntOption &o1,
                               AudioFloatOption &o2);

void audio_option (AudioBoolOption o, bool v);
bool audio_option (AudioBoolOption o);
void audio_option (AudioIntOption o, int v);
int audio_option (AudioIntOption o);
void audio_option (AudioFloatOption o, float v);
float audio_option (AudioFloatOption o);

class AudioSource : public DiskResource::ReloadWatcher
{
    private:
        Demand demand;
        AudioDiskResource *resource;

        Vector3 position;
        Vector3 velocity;
        bool looping;
        float pitch;
        float volume;
        bool ambient;
        float referenceDistance;
        float rollOff;

        ALuint alSource;

        AudioSource (const std::string &filename, bool ambient);
        ~AudioSource (void);

        void reinitialise (void);

        void notifyReloaded (DiskResource *dr);

    public:

        static AudioSourcePtr make (const std::string &filename, bool ambient)
        { return AudioSourcePtr(new AudioSource(filename, ambient)); }


        Vector3 getPosition (void) { return position; }
        void setPosition (const Vector3& v);

        Vector3 getVelocity (void) { return velocity; }
        void setVelocity (const Vector3& v);

        bool getLooping (void) { return looping; }
        void setLooping (bool v);

        float getVolume (void) { return volume; }
        void setVolume (float v);

        float getPitch (void) { return pitch; }
        void setPitch (float v);

        float getReferenceDistance (void) { return referenceDistance; }
        void setReferenceDistance (float v);

        float getRollOff (void) { return rollOff; }
        void setRollOff (float v);

        bool getAmbient (void) { return ambient; }

        bool playing (void);
        void play (void);
        void pause (void);
        void stop (void);

        void destroy (void);

        friend class SharedPtr<AudioSource>;
};


#endif
