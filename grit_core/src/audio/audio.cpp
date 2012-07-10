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

#include <AL/al.h>
#include <AL/alc.h>

#include "../CentralisedLog.h"
#include "../vect_util.h"
#include "../option.h"

#include "audio.h"
#include "AudioDiskResource.h"

ALCdevice* alDevice;
ALCcontext* alContext;

static AudioBoolOption option_keys_bool[] = {
    AUDIO_AUTOUPDATE,
    AUDIO_DOPPLER_ENABLED,
    AUDIO_MUTE
};

static AudioFloatOption option_keys_float[] = {
    AUDIO_MASTER_VOLUME
};

static AudioIntOption option_keys_int[] = {
    AUDIO_MAX_SOUNDS
};

static std::map<AudioBoolOption,bool> options_bool;
static std::map<AudioIntOption,int> options_int;
static std::map<AudioFloatOption,float> options_float;
static std::map<AudioBoolOption,bool> new_options_bool;
static std::map<AudioIntOption,int> new_options_int;
static std::map<AudioFloatOption,float> new_options_float;

static std::map<AudioBoolOption,ValidOption<bool>*> valid_option_bool;
static std::map<AudioIntOption,ValidOption<int>*> valid_option_int;
static std::map<AudioFloatOption,ValidOption<float>*> valid_option_float;

static void valid_option (AudioBoolOption o, ValidOption<bool> *v) { valid_option_bool[o] = v; }
static void valid_option (AudioIntOption o, ValidOption<int> *v) { valid_option_int[o] = v; }
static void valid_option (AudioFloatOption o, ValidOption<float> *v) { valid_option_float[o] = v; }

static bool truefalse_[] = { false, true };
static ValidOptionList<bool,bool[2]> *truefalse = new ValidOptionList<bool,bool[2]>(truefalse_);


std::string audio_option_to_string (AudioBoolOption o)
{
    switch (o) {
        case AUDIO_AUTOUPDATE: return "AUTOUPDATE";
        case AUDIO_DOPPLER_ENABLED: return "DOPPLER_ENABLED";
        case AUDIO_MUTE: return "MUTE";
    }
    return "UNKNOWN_BOOL_OPTION";
}
std::string audio_option_to_string (AudioIntOption o)
{
    switch (o) {
        case AUDIO_MAX_SOUNDS: return "MAX_SOUNDS";
    }
    return "UNKNOWN_INT_OPTION";
}
std::string audio_option_to_string (AudioFloatOption o)
{
    switch (o) {
        case AUDIO_MASTER_VOLUME: return "MASTER_VOLUME";
    }
    return "UNKNOWN_FLOAT_OPTION";
}

void audio_option_from_string (const std::string &s,
                               int &t,
                               AudioBoolOption &o0,
                               AudioIntOption &o1,
                               AudioFloatOption &o2)
{
    if (s=="AUTOUPDATE") { t = 0; o0 = AUDIO_AUTOUPDATE; }

    else if (s=="DOPPLER_ENABLED") { t = 0 ; o0 = AUDIO_DOPPLER_ENABLED; }
    else if (s=="MUTE") { t = 0 ; o0 = AUDIO_MUTE; }

    else if (s=="MAX_SOUNDS") { t = 1 ; o1 = AUDIO_MAX_SOUNDS; }

    else if (s=="MASTER_VOLUME") { t = 2 ; o2 = AUDIO_MASTER_VOLUME; }

    else t = -1;
}

static void options_update (bool flush)
{
    (void) flush;

    for (unsigned i=0 ; i<sizeof(option_keys_bool)/sizeof(*option_keys_bool) ; ++i) {
        AudioBoolOption o = option_keys_bool[i];
        bool v_old = options_bool[o];
        bool v_new = new_options_bool[o];
        if (v_old == v_new) continue;
        switch (o) {
            case AUDIO_AUTOUPDATE: break;
            case AUDIO_DOPPLER_ENABLED:
            case AUDIO_MUTE:
            break;
        }
    }
    for (unsigned i=0 ; i<sizeof(option_keys_int)/sizeof(*option_keys_int) ; ++i) {
        AudioIntOption o = option_keys_int[i];
        int v_old = options_int[o];
        int v_new = new_options_int[o];
        if (v_old == v_new) continue;
        switch (o) {
            case AUDIO_MAX_SOUNDS:
            break;
        }
    }
    for (unsigned i=0 ; i<sizeof(option_keys_float)/sizeof(*option_keys_float) ; ++i) {
        AudioFloatOption o = option_keys_float[i];
        float v_old = options_float[o];
        float v_new = new_options_float[o];
        if (v_old == v_new) continue;
        switch (o) {
            case AUDIO_MASTER_VOLUME:
            break;
        }
    }

    options_bool = new_options_bool;
    options_int = new_options_int;
    options_float = new_options_float;
}


static void init_options (void)
{

    for (unsigned i=0 ; i < sizeof(option_keys_bool) / sizeof(*option_keys_bool) ; ++i) {
        valid_option(option_keys_bool[i], truefalse);

    }

    valid_option(AUDIO_MAX_SOUNDS, new ValidOptionRange<int>(1,1000));

    valid_option(AUDIO_MASTER_VOLUME, new ValidOptionRange<float>(0, 2));

    audio_option(AUDIO_AUTOUPDATE, false);

    audio_option(AUDIO_MUTE, false);
    audio_option(AUDIO_DOPPLER_ENABLED, true);

    audio_option(AUDIO_MAX_SOUNDS, 1000);

    audio_option(AUDIO_MASTER_VOLUME, 1.0f);

    options_update(true);

    audio_option(AUDIO_AUTOUPDATE, true);

}


void audio_option (AudioBoolOption o, bool v)
{
    valid_option_bool[o]->maybeThrow("Audio", v);
    new_options_bool[o] = v;
    if (new_options_bool[AUDIO_AUTOUPDATE]) options_update(false);
}
bool audio_option (AudioBoolOption o)
{
    return options_bool[o];
}

void audio_option (AudioIntOption o, int v)
{
    valid_option_int[o]->maybeThrow("Audio", v);
    new_options_int[o] = v;
    if (new_options_bool[AUDIO_AUTOUPDATE]) options_update(false);
}
int audio_option (AudioIntOption o)
{
    return options_int[o];
}

void audio_option (AudioFloatOption o, float v)
{
    valid_option_float[o]->maybeThrow("Audio", v);
    new_options_float[o] = v;
    if (new_options_bool[AUDIO_AUTOUPDATE]) options_update(false);
}
float audio_option (AudioFloatOption o)
{
    return options_float[o];
}


void audio_init (void)
{
    init_options();

    alDevice = alcOpenDevice(NULL);

    if (!alDevice)
    {
        GRIT_EXCEPT("Couldn't initialize the OpenAL device.");
    }

    alContext = alcCreateContext(alDevice, NULL);
    alcMakeContextCurrent(alContext);
}

static std::vector<ALuint> one_shot_sounds;

void audio_play (const std::string& filename, bool ambient, const Vector3& position, float volume, float ref_dist, float roll_off, float pitch)
{
    DiskResource *dr = disk_resource_get(filename);
    if (dr == NULL) GRIT_EXCEPT("Resource not found: \""+filename+"\"");
    AudioDiskResource *resource = dynamic_cast<AudioDiskResource*>(dr);
    if (resource == NULL) GRIT_EXCEPT("Not an audio resource: \""+filename+"\"");
    if (!resource->isLoaded()) {
        GRIT_EXCEPT("Cannot play an unloaded audio resource: " + resource->getName());
    }
    if (!ambient && resource->getAmbientOnly()) {
        GRIT_EXCEPT("Can only play a stereo audio resource as ambient sound: " + resource->getName());
    }
    
    one_shot_sounds.push_back(ALuint());
    ALuint &src = one_shot_sounds[one_shot_sounds.size()-1];

    alGenSources(1, &src);
    alSourcei(src, AL_BUFFER, resource->getALBuffer());
    alSourcef(src, AL_GAIN, volume);
    alSourcef(src, AL_PITCH, pitch);
    alSourcef(src, AL_REFERENCE_DISTANCE, ref_dist);
    alSourcef(src, AL_ROLLOFF_FACTOR, roll_off);
    if (ambient) {
        alSourcei(src, AL_SOURCE_RELATIVE, AL_TRUE);
    } else {
        alSource3f(src, AL_POSITION, position.x, position.y, position.z);
    }
    alSourcePlay(src);
}

void audio_update (const Vector3& position, const Vector3& velocity, const Quaternion& rotation)
{
    ALfloat pos[] = { position.x, position.y, position.z };
    ALfloat vel[] = { velocity.x, velocity.y, velocity.z };
    ALfloat zeros[] = { 0.0f, 0.0f, 0.0f };

    Vector3 at = rotation * Vector3(0.0f, 1.0f, 0.0f);
    Vector3 up = rotation * Vector3(0.0f, 0.0f, 1.0f);

    ALfloat ori[] = { at.x, at.y, at.z, up.x, up.y, up.z };

    alcSuspendContext(alContext);
    alListenerfv(AL_POSITION, pos);
    alListenerfv(AL_VELOCITY, audio_option(AUDIO_DOPPLER_ENABLED) ? vel : zeros);
    alListenerfv(AL_ORIENTATION, ori);
    float volume = audio_option(AUDIO_MUTE) ? 0 : audio_option(AUDIO_MASTER_VOLUME);
    alListenerfv(AL_GAIN, &volume);
    alcProcessContext(alContext);

    // destroy any one-shot sounds that finished playing
    // NOTE: this won't account for cases where there's still a sound playing when the game exits

    for (unsigned i=0 ; i<one_shot_sounds.size() ; ++i) {
        ALuint &src = one_shot_sounds[i];

        ALint playing;
        alGetSourcei(src, AL_SOURCE_STATE, &playing);

        if (playing != AL_PLAYING) {

            alDeleteSources(1, &src);

            one_shot_sounds[i] = one_shot_sounds[one_shot_sounds.size()-1];
            one_shot_sounds.pop_back();
            i--; // re-examine index i again, next iteration

        }
    }
}

AudioSource::AudioSource (const std::string &filename, bool ambient)
      : resource(NULL),
        position(Vector3(0,0,0)),
        velocity(Vector3(0,0,0)),
        looping(false),
        pitch(1.0f),
        volume(1.0f),
        ambient(ambient),
        referenceDistance(1),
        rollOff(1)
{
    DiskResource *dr = disk_resource_get(filename);
    if (dr == NULL) {
        GRIT_EXCEPT("Resource not found: \""+filename+"\"");
    }

    resource = dynamic_cast<AudioDiskResource*>(dr);
    if (resource == NULL) {
        GRIT_EXCEPT("Not an audio resource: \""+filename+"\"");
    }

    alGenSources(1, &alSource);

    reinitialise();

    resource->registerReloadWatcher(this);

}

AudioSource::~AudioSource (void)
{
    if (resource == NULL) return;
    resource->unregisterReloadWatcher(this);
}

void AudioSource::notifyReloaded (DiskResource *r)
{
    (void) r;
    bool was_playing = playing();
    if (was_playing) stop();
    reinitialise();
    if (was_playing) play();
}

void AudioSource::reinitialise (void)
{
    alSourcei(alSource, AL_BUFFER, AL_NONE);
    if (!resource->isLoaded()) {
        CERR << "Cannot create an AudioSource for an unloaded audio resource: " + resource->getName() << std::endl;
        return;
    }
    if (!ambient && resource->getAmbientOnly()) {
        CERR << "Can only play a stereo audio resource as ambient sound: " + resource->getName() << std::endl;
        return;
    }

    // use the sound from the resource
    alSourcei(alSource, AL_BUFFER, resource->getALBuffer());
}

void AudioSource::destroy (void)
{
    alDeleteSources(1, &alSource);
}

void AudioSource::setPosition (const Vector3& v)
{
    if (ambient) return;
    position = v;
    alSource3f(alSource, AL_POSITION, v.x, v.y, v.z);
}

void AudioSource::setVelocity (const Vector3& v)
{
    velocity = v;
    alSource3f(alSource, AL_VELOCITY, v.x, v.y, v.z);
}

void AudioSource::setLooping (bool v)
{
    looping = v;
    alSourcei(alSource, AL_LOOPING, v);
}

void AudioSource::setVolume (float v)
{
    volume = v;
    alSourcef(alSource, AL_GAIN, v);
}

void AudioSource::setPitch (float v)
{
    pitch = v;
    alSourcef(alSource, AL_PITCH, v);
}

void AudioSource::setReferenceDistance (float v)
{
    referenceDistance = v;
    alSourcef(alSource, AL_REFERENCE_DISTANCE, v);
}

void AudioSource::setRollOff (float v)
{
    rollOff = v;
    alSourcef(alSource, AL_ROLLOFF_FACTOR, v);
}

bool AudioSource::playing (void)
{
    ALint value;
    alGetSourcei(alSource, AL_SOURCE_STATE, &value);

    return (value == AL_PLAYING);
}

void AudioSource::play (void)
{
    alSourcePlay(alSource);
}

void AudioSource::pause (void)
{
    // I guess we should check if the source is actually playing, but whatever
    alSourcePause(alSource);
}

void AudioSource::stop (void)
{
    alSourceStop(alSource);
}
