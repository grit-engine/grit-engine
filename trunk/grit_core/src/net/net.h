#include "../SharedPtr.h"

class NetAddress;
typedef SharedPtr<NetAddress> NetAddressPtr;

class NetMessage;
typedef SharedPtr<NetMessage> NetMessagePtr;

struct lua_State;

#ifndef net_h
#define net_h

#include <string>

class ExternalTable;

#include <stdint.h>

#if defined(WIN32)
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/param.h>
#include <sys/types.h>
#include <netdb.h>
#define SOCKET int
#define INVALID_SOCKET (-1)
#include <sys/ioctl.h>
#endif

enum NetChannel
{
	NetChan_ClientToServer,
	NetChan_ServerToClient
};

enum NetAddressType
{
	NetAddress_Invalid,
	NetAddress_Loopback,
	NetAddress_IPv4,
	NetAddress_IPv6
};

class NetAddress
{
private:
	NetAddressType type;

	union
	{
		uint32_t ip4;
		uint8_t ip6[16];
	};

	uint16_t port;

public:
	NetAddress();
	NetAddress(NetAddressType type);
	NetAddress(sockaddr* addr, int length);

	static NetAddress& getLoopback();
	static NetAddress resolve(const char* host, NetAddressType type);

	bool operator ==(NetAddress& right);
	bool operator !=(NetAddress& right);

	NetAddressType getType() { return type; }
	void getSockAddr(sockaddr_storage* storage, int* length);

	std::string toString();
};

class NetMessage
{
private:
	char* buffer;
	bool bufferManaged;

	int curBit;
	int maxBit;

public:
	NetMessage();
	NetMessage(const char* data, uint32_t length);

	virtual ~NetMessage();
	
	std::string getBuffer();
	int getLength();

	// readers
	bool readBool();
	int32_t readInteger();
	int32_t readInteger(int numBits);

	float readFloat();

	std::string readString();

	// delta readers
	int32_t readDeltaInteger(int32_t old);
	int32_t readDeltaInteger(int32_t old, int numBits);

	float readDeltaFloat(float old);

	// writers
	void writeBool(bool value);
	void writeInteger(int32_t value);
	void writeInteger(int32_t value, int numBits);

	void writeFloat(float value);

	void writeString(std::string& value);

	// delta writers
	void writeDeltaInteger(int32_t value, int32_t old);
	void writeDeltaInteger(int32_t value, int32_t old, int bits);

	void writeDeltaFloat(float value, float old);

	// raw reader/writer
	void readBits(int bits, void* out);
	void writeBits(int bits, const void* data);
};

// initializes the networking system
void net_init();

// processes network packets and sends them off to wherever they need to go
void net_process(lua_State* L);

// sends an out-of-band packet to a specified network address
void net_send_oob(NetChannel channel, NetAddress& address, const char* format, ...);

// sends a packet to a network address
void net_send(NetChannel channel, NetAddress& address, const char* packet, uint32_t packetLength);

// sets the network callbacks
void net_set_callbacks(ExternalTable& table);

// gets a loopback packet
bool net_get_loopback_packet(NetChannel channel, std::string& packet);

#endif
