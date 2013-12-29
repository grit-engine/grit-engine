class NetManager;

#ifndef NetManager_h
#define NetManager_h

#include <cstdlib>

#include <queue>

#include "../ExternalTable.h"

#include "net.h"

class NetManager
{
private:
	struct NetPacket
	{
		NetAddress addr;
		std::string data;
		uint64_t time;

		NetPacket(NetAddress& from, std::string& data_);
	};

	SOCKET netSocket;

	int forcedLatency;

	std::queue<NetPacket> receiveQueue;
	std::queue<NetPacket> sendQueue;

	std::queue<std::string> clientLoopQueue;
	std::queue<std::string> serverLoopQueue;

	ExternalTable netCBTable;

	void sendLoopbackPacket(NetChannel channel, std::string& packet);
	void sendPacketInternal(NetAddress& netAddress, std::string& packet);

public:
	NetManager();
	virtual ~NetManager();

	void process(lua_State* L);
	void processPacket(lua_State* L, NetAddress& from, std::string& data);

	void sendPacket(NetChannel channel, NetAddress& address, std::string& data);

	void setCBTable(ExternalTable& table);
	ExternalTable& getCBTable();

	bool getLoopbackPacket(NetChannel channel, std::string& packet);
};

#endif
