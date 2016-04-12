//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

// This is a modified version of SampleInterfaces.cpp from Recast Demo

#define _USE_MATH_DEFINES
#include <math.h>
#include <stdio.h>
#include <stdarg.h>
#include "navigation_interfaces.h"
#include "Recast.h"
#include "RecastDebugDraw.h"
#include "DetourDebugDraw.h"

#include"navigation_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

BuildContext::BuildContext() :
	m_messageCount(0),
	m_textPoolSize(0)
{
	resetTimers();
}

BuildContext::~BuildContext()
{
}

// Virtual functions for custom implementations.
void BuildContext::doResetLog()
{
	m_messageCount = 0;
	m_textPoolSize = 0;
}

void BuildContext::doLog(const rcLogCategory category, const char* msg, const int len)
{
	if (!len) return;
	if (m_messageCount >= MAX_MESSAGES)
		return;
	char* dst = &m_textPool[m_textPoolSize];
	int n = TEXT_POOL_SIZE - m_textPoolSize;
	if (n < 2)
		return;
	char* cat = dst;
	char* text = dst+1;
	const int maxtext = n-1;
	// Store category
	*cat = (char)category;
	// Store message
	const int count = rcMin(len+1, maxtext);
	memcpy(text, msg, count);
	text[count-1] = '\0';
	m_textPoolSize += 1 + count;
	m_messages[m_messageCount++] = dst;
}

void BuildContext::dumpLog(const char* format, ...)
{
	// Print header.
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
	printf("\n");
	
	// Print messages
	const int TAB_STOPS[4] = { 28, 36, 44, 52 };
	for (int i = 0; i < m_messageCount; ++i)
	{
		const char* msg = m_messages[i]+1;
		int n = 0;
		while (*msg)
		{
			if (*msg == '\t')
			{
				int count = 1;
				for (int j = 0; j < 4; ++j)
				{
					if (n < TAB_STOPS[j])
					{
						count = TAB_STOPS[j] - n;
						break;
					}
				}
				while (--count)
				{
					putchar(' ');
					n++;
				}
			}
			else
			{
				putchar(*msg);
				n++;
			}
			msg++;
		}
		putchar('\n');
	}
}

int BuildContext::getLogCount() const
{
	return m_messageCount;
}

const char* BuildContext::getLogText(const int i) const
{
	return m_messages[i]+1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//++
Ogre::ColourValue uintColorToOgre(unsigned int colour)
{
	int r = colour & 0xff;
	int g = (colour >> 8) & 0xff;
	int b = (colour >> 16) & 0xff;
	int a = (colour >> 24) & 0xff;

	float fr = (float)r / 255;
	float fg = (float)g / 255;
	float fb = (float)b / 255;
	float fa = (float)a / 255;

	return Ogre::ColourValue(fr, fg, fb, fa);
}

void DebugDrawGL::depthMask(bool state)
{
    (void) state;
}

void DebugDrawGL::texture(bool state)
{
    (void) state;
}

void DebugDrawGL::begin(duDebugDrawPrimitives prim, float size)
{
    (void)size;
	switch (prim)
	{
		case DU_DRAW_POINTS:
			NavSysDebug::DebugObject->begin("debugobjwireframe", Ogre::RenderOperation::OT_POINT_LIST);
			break;
		case DU_DRAW_LINES:
			NavSysDebug::DebugObject->begin("debugobjwireframe", Ogre::RenderOperation::OT_LINE_LIST);
			break;
		case DU_DRAW_TRIS:
			NavSysDebug::DebugObject->begin("debugobj", Ogre::RenderOperation::OT_TRIANGLE_LIST);
			break;
		case DU_DRAW_QUADS:
			NavSysDebug::DebugObject->begin("debugobjwireframe", Ogre::RenderOperation::OT_POINT_LIST);
			break;
	};
}

void DebugDrawGL::vertex(const float* pos, unsigned int color)
{
	NavSysDebug::DebugObject->position(-pos[0], pos[2], pos[1]);
	NavSysDebug::DebugObject->colour(uintColorToOgre(color));
}

void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color)
{
	NavSysDebug::DebugObject->position(-x, z, y);
	NavSysDebug::DebugObject->colour(uintColorToOgre(color));
}

void DebugDrawGL::vertex(const float* pos, unsigned int color, const float* uv)
{
    (void) uv;
	NavSysDebug::DebugObject->position(-pos[0], pos[2], pos[1]);
	NavSysDebug::DebugObject->colour(uintColorToOgre(color));
}
void DebugDrawGL::vertex(const float x, const float y, const float z, unsigned int color, const float u, const float v)
{
    (void) u;
    (void) v;
	NavSysDebug::DebugObject->position(-x, z, y);
	NavSysDebug::DebugObject->colour(uintColorToOgre(color));
}

void DebugDrawGL::end()
{
	NavSysDebug::DebugObject->end();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

FileIO::FileIO() :
	m_fp(0),
	m_mode(-1)
{
}

FileIO::~FileIO()
{
	if (m_fp) fclose(m_fp);
}

bool FileIO::openForWrite(const char* path)
{
	if (m_fp) return false;
	m_fp = fopen(path, "wb");
	if (!m_fp) return false;
	m_mode = 1;
	return true;
}

bool FileIO::openForRead(const char* path)
{
	if (m_fp) return false;
	m_fp = fopen(path, "rb");
	if (!m_fp) return false;
	m_mode = 2;
	return true;
}

bool FileIO::isWriting() const
{
	return m_mode == 1;
}

bool FileIO::isReading() const
{
	return m_mode == 2;
}

bool FileIO::write(const void* ptr, const size_t size)
{
	if (!m_fp || m_mode != 1) return false;
	fwrite(ptr, size, 1, m_fp);
	return true;
}

bool FileIO::read(void* ptr, const size_t size)
{
	if (!m_fp || m_mode != 2) return false;
	size_t readLen = fread(ptr, size, 1, m_fp);
	return readLen == 1;
}


