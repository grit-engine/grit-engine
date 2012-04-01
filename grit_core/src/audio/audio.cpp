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

#include <AL/al.h>
#include <AL/alc.h>

#include "../CentralisedLog.h"

#include "audio.h"
#include "AudioDiskResource.h"

ALCdevice* alDevice;
ALCcontext* alContext;

void audio_init (void)
{
	alDevice = alcOpenDevice(NULL);

	if (!alDevice)
	{
		GRIT_EXCEPT("Couldn't initialize the OpenAL device.");
	}

	alContext = alcCreateContext(alDevice, NULL);
	alcMakeContextCurrent(alContext);
}

void audio_set_listener (const Vector3& position, const Vector3& velocity, const Quaternion& rotation)
{
	ALfloat pos[] = { position.x, position.y, position.z };
	ALfloat vel[] = { velocity.x, velocity.y, velocity.z };

	Vector3 at = rotation * Vector3(0.0f, 1.0f, 0.0f);
	Vector3 up = rotation * Vector3(0.0f, 0.0f, 1.0f);

	ALfloat ori[] = { at.x, at.y, at.z, up.x, up.y, up.z };

	alcSuspendContext(alContext);
	alListenerfv(AL_POSITION, pos);
	alListenerfv(AL_VELOCITY, vel);
	alListenerfv(AL_ORIENTATION, ori);
	alcProcessContext(alContext);
}

static std::list<AudioSource*> oneShotSounds;

void audio_play (const std::string& filename, const Vector3& position)
{
	// TEMP HACK until we get manual disk resource loading
	Demand demand;
	demand.addDiskResource(filename);
	demand.immediateLoad();

	// create an audio source object
	AudioSource* source = new AudioSource(filename);
    source->setPosition(position);
	source->play();

	// add it to the one-shot list
	oneShotSounds.push_back(source);
}

void audio_update (void)
{
	// destroy any one-shot sounds that finished playing
	// NOTE: this won't account for cases where there's still a sound playing when the game exits
	std::list<AudioSource*> toRemove;

	for (std::list<AudioSource*>::iterator iter = oneShotSounds.begin(); iter != oneShotSounds.end(); iter++)
	{
		AudioSource* source = *iter;

		if (!source->playing())
		{
			toRemove.push_back(source);
			source->destroy();
			delete source;
		}
	}

	for (std::list<AudioSource*>::iterator iter = toRemove.begin(); iter != toRemove.end(); iter++)
	{
		oneShotSounds.remove(*iter);
	}
}

static float master_volume;

void audio_master_volume (float f) {
    master_volume = f;
	alListenerfv(AL_GAIN, &f);
}

float audio_master_volume (void) {
    return master_volume;
}

AudioSource::AudioSource (const std::string &filename)
{
	alGenSources(1, &alSource);
	resourceName = filename;
}

void AudioSource::destroy (void)
{
	alDeleteSources(1, &alSource);
}

void AudioSource::setPosition (const Vector3& position)
{
	this->position = position;
	alSource3f(alSource, AL_POSITION, position.x, position.y, position.z);
}

void AudioSource::setVelocity (const Vector3& velocity)
{
	this->velocity = velocity;
	alSource3f(alSource, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void AudioSource::setLoop (bool looping)
{
	this->looping = looping;
	alSourcei(alSource, AL_LOOPING, looping);
}

void AudioSource::setVolume (float volume)
{
	alSourcef(alSource, AL_GAIN, volume);
}

void AudioSource::setPitch (float pitch)
{
	alSourcef(alSource, AL_PITCH, pitch);
}

bool AudioSource::playing (void)
{
	ALint value;
	alGetSourcei(alSource, AL_SOURCE_STATE, &value);

	return (value == AL_PLAYING);
}

void AudioSource::play (void)
{
	AudioDiskResource* resource = (AudioDiskResource*)disk_resource_get(resourceName);

	if (!resource)
	{
		GRIT_EXCEPT("Can not play a non-loaded audio source: " + resourceName);
	}

	ALuint buffer = resource->getALBuffer();
	alSourcei(alSource, AL_BUFFER, buffer);
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
