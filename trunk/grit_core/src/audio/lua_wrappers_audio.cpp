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

#include "../main.h"
#include "../external_table.h"
#include "../lua_ptr.h"
#include "../path_util.h"

#include "audio.h"
#include "lua_wrappers_audio.h"

#define AUDIOSOURCE_TAG "Grit/AudioSource"

void push_audiosource (lua_State *L, const AudioSourcePtr &self)
{
    if (self.isNull())
        lua_pushnil(L);
    else
        push(L,new AudioSourcePtr(self),AUDIOSOURCE_TAG);
}

GC_MACRO(AudioSourcePtr,audiosource,AUDIOSOURCE_TAG)

static int audiosource_play (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(AudioSourcePtr,self,1,AUDIOSOURCE_TAG);
    self->play();
    return 0;
TRY_END
}

static int audiosource_pause (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(AudioSourcePtr,self,1,AUDIOSOURCE_TAG);
    self->pause();
    return 0;
TRY_END
}

static int audiosource_stop (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(AudioSourcePtr,self,1,AUDIOSOURCE_TAG);
    self->stop();
    return 0;
TRY_END
}

static int audiosource_destroy (lua_State *L)
{
TRY_START
    check_args(L,1);
    GET_UD_MACRO(AudioSourcePtr,self,1,AUDIOSOURCE_TAG);
    self->destroy();
    return 0;
TRY_END
}

TOSTRING_SMART_PTR_MACRO (audiosource,AudioSourcePtr,AUDIOSOURCE_TAG)

static int audiosource_index (lua_State *L)
{
TRY_START
    check_args(L,2);
    GET_UD_MACRO(AudioSourcePtr,self,1,AUDIOSOURCE_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"position")) {
        push_v3(L, self->getPosition());
    } else if (!::strcmp(key,"orientation")) {
        push_quat(L, self->getOrientation());
    } else if (!::strcmp(key,"separation")) {
        lua_pushnumber(L, self->getSeparation());
    } else if (!::strcmp(key,"velocity")) {
        push_v3(L, self->getVelocity());
    } else if (!::strcmp(key,"looping")) {
        lua_pushboolean(L, self->getLooping());
    } else if (!::strcmp(key,"pitch")) {
        lua_pushnumber(L, self->getPitch());
    } else if (!::strcmp(key,"volume")) {
        lua_pushnumber(L, self->getVolume());
    } else if (!::strcmp(key,"ambient")) {
        lua_pushboolean(L, self->getAmbient());
    } else if (!::strcmp(key,"referenceDistance")) {
        lua_pushnumber(L, self->getReferenceDistance());
    } else if (!::strcmp(key,"rollOff")) {
        lua_pushnumber(L, self->getRollOff());
    } else if (!::strcmp(key,"playing")) {
        lua_pushboolean(L, self->playing());
	} else if (!::strcmp(key,"play")) {
		push_cfunction(L,audiosource_play);
	} else if (!::strcmp(key,"pause")) {
		push_cfunction(L,audiosource_pause);
	} else if (!::strcmp(key,"stop")) {
		push_cfunction(L,audiosource_stop);
    } else if (!::strcmp(key,"destroy")) {
        push_cfunction(L,audiosource_destroy);
    } else {
        my_lua_error(L,"Not a readable AudioSource member: "+std::string(key));
    }
    return 1;
TRY_END
}


static int audiosource_newindex (lua_State *L)
{
TRY_START
    check_args(L,3);
    GET_UD_MACRO(AudioSourcePtr,self,1,AUDIOSOURCE_TAG);
    const char *key = luaL_checkstring(L,2);
    if (!::strcmp(key,"position")) {
        Vector3 v = check_v3(L,3);
        self->setPosition(v);
    } else if (!::strcmp(key,"orientation")) {
        Quaternion v = check_quat(L,3);
		self->setOrientation(v);
    } else if (!::strcmp(key,"separation")) {
        float v = check_float(L,3);
		self->setSeparation(v);
    } else if (!::strcmp(key,"velocity")) {
        Vector3 v = check_v3(L,3);
		self->setVelocity(v);
	} else if (!::strcmp(key,"pitch")) {
		float f = check_float(L, 3);
		self->setPitch(f);
	} else if (!::strcmp(key,"volume")) {
		float f = check_float(L, 3);
		self->setVolume(f);
	} else if (!::strcmp(key,"referenceDistance")) {
		float f = check_float(L, 3);
		self->setReferenceDistance(f);
	} else if (!::strcmp(key,"rollOff")) {
		float f = check_float(L, 3);
		self->setRollOff(f);
    } else if (!::strcmp(key,"looping")) {
        bool b = check_bool(L,3);
		self->setLooping(b);
    } else {
        my_lua_error(L,"Not a writable AudioSource member: "+std::string(key));
    }
    return 0;
TRY_END
}

EQ_MACRO(AudioSourcePtr,audiosource,AUDIOSOURCE_TAG)

MT_MACRO_NEWINDEX(audiosource);

static int global_audio_source_make (lua_State *L)
{
TRY_START
	check_args(L, 1);
	push_audiosource(L, AudioSource::make(check_string(L, 1),false));
	return 1;
TRY_END
}

static int global_audio_source_make_ambient (lua_State *L)
{
TRY_START
	check_args(L, 1);
	push_audiosource(L, AudioSource::make(check_string(L, 1),true));
	return 1;
TRY_END
}

static int global_audio_play_ambient (lua_State *L)
{
TRY_START
	check_args(L, 6);
    std::string res = check_string(L,1);
    Vector3 pos = check_v3(L,2);
    float volume = check_float(L,3);
    float ref_dist = check_float(L,4);
    float roll_off = check_float(L,5);
    float pitch = check_float(L,6);
	audio_play(res, true, pos, volume, ref_dist, roll_off, pitch);
	return 0;
TRY_END
}

static int global_audio_play (lua_State *L)
{
TRY_START
	check_args(L, 6);
    std::string res = check_string(L,1);
    Vector3 pos = check_v3(L,2);
    float volume = check_float(L,3);
    float ref_dist = check_float(L,4);
    float roll_off = check_float(L,5);
    float pitch = check_float(L,6);
	audio_play(res, false, pos, volume, ref_dist, roll_off, pitch);
	return 0;
TRY_END
}

static int global_audio_update (lua_State *L)
{
TRY_START
	check_args(L, 3);
	audio_update(check_v3(L, 1), check_v3(L, 2), check_quat(L, 3));
	return 0;
TRY_END
}

int global_audio_option (lua_State *L)
{
TRY_START
    if (lua_gettop(L)==2) {
        std::string opt = luaL_checkstring(L,1);
        int t;
        AudioBoolOption o0;
        AudioIntOption o1;
        AudioFloatOption o2;
        audio_option_from_string(opt, t, o0, o1, o2);
        switch (t) {
            case -1: my_lua_error(L,"Unrecognised audio option: \""+opt+"\"");
            case 0: audio_option(o0, check_bool(L,2)); break;
            case 1: audio_option(o1, check_t<int>(L,2)); break;
            case 2: audio_option(o2, check_float(L,2)); break;
            default: my_lua_error(L,"Unrecognised type from audio_option_from_string");
        }
        return 0;
    } else {
        check_args(L,1);
        std::string opt = luaL_checkstring(L,1);
        int t;
        AudioBoolOption o0;
        AudioIntOption o1;
        AudioFloatOption o2;
        audio_option_from_string(opt, t, o0, o1, o2);
        switch (t) {
            case -1: my_lua_error(L,"Unrecognised audio option: \""+opt+"\"");
            case 0: lua_pushboolean(L,audio_option(o0)); break;
            case 1: lua_pushnumber(L,audio_option(o1)); break;
            case 2: lua_pushnumber(L,audio_option(o2)); break;
            default: my_lua_error(L,"Unrecognised type from audio_option_from_string");
        }
        return 1;
    }
TRY_END
}

static const luaL_reg global[] = {
	{"audio_source_make",global_audio_source_make},
	{"audio_source_make_ambient",global_audio_source_make_ambient},
	{"audio_play",global_audio_play},
	{"audio_play_ambient",global_audio_play_ambient},
	{"audio_update",global_audio_update},
	{"audio_option",global_audio_option},
	{NULL,NULL}
};

void audio_lua_init (lua_State *L)
{
	ADD_MT_MACRO(audiosource,AUDIOSOURCE_TAG);
    register_lua_globals(L, global);
}
