#include "net.h"
#include "../CentralisedLog.h"
#include <stdio.h>

NetMessage::NetMessage()
{
	this->buffer = (char*)malloc(32);
	this->curBit = 0;
	this->maxBit = 32 * 8;
	this->bufferManaged = true;

	memset(this->buffer, 0, 32);
}

NetMessage::NetMessage(const char* data, uint32_t length)
{
	this->buffer = (char*)malloc(length);
	this->curBit = 0;
	this->maxBit = length * 8;
	this->bufferManaged = true;

	memcpy(buffer, data, length);
}

NetMessage::~NetMessage()
{
	if (bufferManaged)
	{
		free(this->buffer);
	}
}

std::string NetMessage::getBuffer()
{
	return std::string(buffer, (curBit + 8) / 8);
}

bool NetMessage::readBool()
{
	bool data = false;
	readBits(1, &data);

	return data;
}

// this is not big endian safe
int32_t NetMessage::readInteger()
{
	return readInteger(32);
}

int32_t NetMessage::readInteger(int numBits)
{
	int32_t data = 0;
	readBits(numBits, &data);

	return data;
}

float NetMessage::readFloat()
{
	float data = 0;
	readBits(32, &data);

	return data;
}

std::string NetMessage::readString()
{
	short length;
	readBits(16, &length);

	char str[4097];
	readBits(length * 8, str);

	str[sizeof(str) - 1] = '\0';

	return std::string(str);
}

int32_t NetMessage::readDeltaInteger(int32_t old)
{
	return readDeltaInteger(old, 32);
}

int32_t NetMessage::readDeltaInteger(int32_t old, int numBits)
{
	if (readBool())
	{
		return readInteger(numBits);
	}

	return old;
}

float NetMessage::readDeltaFloat(float old)
{
	if (readBool())
	{
		return readFloat();
	}

	return old;
}

void NetMessage::writeBool(bool value)
{
	writeBits(1, &value);
}

void NetMessage::writeInteger(int32_t value)
{
	writeInteger(value, 32);
}

void NetMessage::writeInteger(int32_t value, int numBits)
{
	writeBits(numBits, &value);
}

void NetMessage::writeFloat(float value)
{
	writeBits(32, &value);
}

void NetMessage::writeString(std::string& value)
{
	int length = value.length();
	writeBits(16, &length);

	writeBits(length * 8, value.c_str());
}

void NetMessage::writeDeltaInteger(int32_t value, int32_t old, int bits)
{
	writeBool(value != old);

	if (value != old)
	{
		writeInteger(value, bits);
	}
}

void NetMessage::writeDeltaFloat(float value, float old)
{
	writeBool(value != old);

	if (value != old)
	{
		writeFloat(value);
	}
}

int NetMessage::getLength()
{
	return (curBit / 8) + ((curBit % 8) ? 0 : 1);
}

void NetMessage::readBits(int bits, void* out)
{
	if (bits == 0)
	{
		return;
	}

	if ((curBit + bits) > maxBit)
	{
		return;
	}

	int curByte = curBit >> 3;
	int curOut = 0;

	uint8_t* outputStuff = (uint8_t*)out;

	while (bits > 0)
	{
		int minBit = (bits < 8) ? bits : 8;

		int thisByte = (int)buffer[curByte];
		curByte++;

		int remain = curBit & 7;

		if ((minBit + remain) <= 8)
		{
			outputStuff[curOut] = (uint8_t)((0xFF >> (8 - minBit)) & (thisByte >> remain));
		}
		else
		{
			outputStuff[curOut] = (uint8_t)((0xFF >> (8 - minBit)) & (buffer[curByte] << (8 - remain)) | (thisByte >> remain));
		}

		curOut++;
		curBit += minBit;
		bits -= minBit;
	}
}

void NetMessage::writeBits(int bits, const void* data)
{
	if (!bufferManaged)
	{
		GRIT_EXCEPT("tried to write to a read-only NetMessage");
	}

	if (bits == 0)
	{
		return;
	}

	if ((curBit + bits) > maxBit)
	{
		this->buffer = (char*)realloc(this->buffer, (this->maxBit + bits + 8) / 8);
	}

	// this code is really weird
	int bit = bits;

	uint8_t* myData = (uint8_t*)data;

	while (bit > 0)
	{
		int bitPos = curBit & 7;
		int remBit = 8 - bitPos;
		int thisWrite = (bit < remBit) ? bit : remBit;

		uint8_t mask = ((0xFF >> remBit) | (0xFF << (bitPos + thisWrite)));
		int bytePos = curBit >> 3;

		uint8_t tempByte = (mask & buffer[bytePos]);
		uint8_t thisBit = ((bits - bit) & 7);
		int thisByte = (bits - bit) >> 3;

		uint8_t thisData = myData[thisByte];

		int nextByte = (((bits - 1) >> 3) > thisByte) ? myData[thisByte + 1] : 0;

		thisData = ((nextByte << (8 - thisBit)) | (thisData >> thisBit));

		uint8_t outByte = (~mask & (thisData << bitPos) | tempByte);
		buffer[bytePos] = outByte;

		curBit += thisWrite;
		bit -= thisWrite;

		if (maxBit < curBit)
		{
			maxBit = curBit;
		}
	}
}