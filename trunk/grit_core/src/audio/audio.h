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

#include "../shared_ptr.h"

class AudioBody;
typedef SharedPtr<AudioBody> AudioBodyPtr;

#ifndef audio_h
#define audio_h

#include <AL/al.h>

#include <math_util.h>

#include "../background_loader.h"
#include "../disk_resource.h"

#include "audio_disk_resource.h"

enum AudioBoolOption {
    /** Whether or not setting the next option will cause fresh option values to be
     * processed (default true).  To update more than one option concurrently, set
     * autoupdate to false, set the options, then set it to true.  */
    AUDIO_AUTOUPDATE,
    /** Whether or not to use the OpenAL doppler shift feature. */
    AUDIO_DOPPLER_ENABLED,
    /** Whether or not to completely suppress all audio. */
    AUDIO_MUTE
};

enum AudioFloatOption {
    /** A central multiplier for controling the amplitude of all audio. */
    AUDIO_MASTER_VOLUME
};

enum AudioIntOption {
    /** UNUSED. */
    AUDIO_MAX_SOUNDS
};

/** Initialise the audio subsystem.
 * \param devname A string to identify the audio device, or NULL for the system default.
 */
void audio_init (const char *devname);

/** Close the audio device, to prevent audio stuttering.
 */
void audio_shutdown ();

/** Call every frame.
 * \param position Worldspace location of player.
 * \param velocity Worldspace velocity of player.
 * \param rotation Orientation of player.
 */
void audio_update (const Vector3& position, const Vector3& velocity, const Quaternion& rotation);

/** Make an instantaneous ambient sound.  Useful for distant car horns, and
 * other short duration environmental sounds.  The sound cannot be controlled
 * after playback has begun.
 * \param filename The name of an audio disk resource, must be loaded.
 * \param volume Amplitude multiplier.
 * \param pitch Frequency multiplier (use 1 for verbatim playback).
 */
void audio_play_ambient (const std::string& filename, float volume, float pitch);

/** Make an instantaneous sound.  Useful for bangs, knocks, and other short
 * duration sounds.  The sound cannot be controlled after playback has begun.
 * \param filename The name of an audio disk resource, must be loaded.
 * \param position The worldspace location to emit from.  Stereo sounds have channels fused.
 * \param volume Amplitude multiplier.
 * \param ref_dist Reference distance - the radius of a sphere inside which the
 *        volume is maximum.
 * \param roll_off The strength at which volume decreases per distance outside
 *        the ref_dist sphere.
 * \param pitch Frequency multiplier (use 1 for verbatim playback).
 */
void audio_play (const std::string& filename, const Vector3& position,
                 float volume, float ref_dist, float roll_off, float pitch);

/** Convert the enum value to a human-readable string. */
std::string audio_option_to_string (AudioBoolOption o);
/** Convert the enum value to a human-readable string. */
std::string audio_option_to_string (AudioIntOption o);
/** Convert the enum value to a human-readable string. */
std::string audio_option_to_string (AudioFloatOption o);

/** Returns the enum value of the option described by s.  Only one of o0, o1,
 * o2 is modified, and t is used to tell the caller which one. */
void audio_option_from_string (const std::string &s,
                               int &t,
                               AudioBoolOption &o0,
                               AudioIntOption &o1,
                               AudioFloatOption &o2);

/** Set the option to a particular value. */
void audio_option (AudioBoolOption o, bool v);
/** Return the current value of the option. */
bool audio_option (AudioBoolOption o);
/** Set the option to a particular value. */
void audio_option (AudioIntOption o, int v);
/** Return the current value of the option. */
int audio_option (AudioIntOption o);
/** Set the option to a particular value. */
void audio_option (AudioFloatOption o, float v);
/** Return the current value of the option. */
float audio_option (AudioFloatOption o);

/** A sound emitter whose parameters can be modified while the sound is
 * playing.  Stereo sounds are implemented using two emitters. */
class AudioBody : public DiskResource::ReloadWatcher
{
    private:
        AudioDiskResource *resource;

        Vector3 position;
        Quaternion orientation; // for stereo
        float separation; // for stereo
        Vector3 velocity;
        bool looping;
        float pitch;
        float volume;
        bool ambient;
        float referenceDistance;
        float rollOff;

        ALuint alSourceLeft;
        ALuint alSourceRight;

        AudioBody (const std::string &filename, bool ambient);
        virtual ~AudioBody (void);

        void reinitialise (void);

        void notifyReloaded (const DiskResource *dr);

        void updatePositions (void);

    public:

        /** Make a new audio source and wrap in a smart pointer.
         * \param filename The disk resource that will be played.  Must be loaded.
         * \param ambient Whether or not the sound attenuates with player distance.
         */
        static AudioBodyPtr make (const std::string &filename, bool ambient)
        { return AudioBodyPtr(new AudioBody(filename, ambient)); }

        /** Return the worldspace location where sound is emitted. */
        Vector3 getPosition (void) { return position; }
        /** Set the worldspace location where sound is emitted. */
        void setPosition (const Vector3& v);

        /** Return the worldspace orientation of the body.
         */
        Quaternion getOrientation (void) { return orientation; }
        /** Set the worldspace orientation of the body.
         * This only has an effect for stereo sounds, where two mono emitters are used.
         */
        void setOrientation (const Quaternion& v);

        /** Return the separation of the two mono emitters used for a stereo sound. */
        float getSeparation (void) { return separation; }
        /** Set the separation of the two mono emitters used for a stereo sound. */
        void setSeparation (float v);

        /** Return the world space velocity of the sound (used for doppler). */
        Vector3 getVelocity (void) { return velocity; }
        /** Set the world space velocity of the sound (used for doppler). */
        void setVelocity (const Vector3& v);

        /** Return whether or not the sound plays forever. */
        bool getLooping (void) { return looping; }
        /** Set whether or not the sound plays forever. */
        void setLooping (bool v);

        /** Return the sound's amplitude multiplier. */
        float getVolume (void) { return volume; }
        /** Set the sound's amplitude multiplier. */
        void setVolume (float v);

        /** Return the sound's frequency multiplier. */
        float getPitch (void) { return pitch; }
        /** Set the sound's frequency multiplier. */
        void setPitch (float v);

        /** Return the radius of the sphere within which the volume is not attenuated. */
        float getReferenceDistance (void) { return referenceDistance; }
        /** Set the radius of the sphere within which the volume is not attenuated. */
        void setReferenceDistance (float v);

        /** Return the parameter that controls the strength of the attenuation due to distance from the edge of the reference distance sphere. */
        float getRollOff (void) { return rollOff; }
        /** Set the rolloff. \see getRollOff. */
        void setRollOff (float v);

        /** Return whether or not the sound attenuates with distance from the player. */
        bool getAmbient (void) { return ambient; }

        /** Is the sound currently playing. */
        bool playing (void);
        /** Cause the sound to play. */
        void play (void);
        /** Pause playback (resume with \see play). */
        void pause (void);
        /** Stop playback (restart with \see play). */
        void stop (void);

        /** Destroy TODO: why? */
        void destroy (void);

        friend class SharedPtr<AudioBody>;
};


#endif
