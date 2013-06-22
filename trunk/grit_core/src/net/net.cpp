#include "../CentralisedLog.h"
#include "../sleep.h"
#include "net.h"

#include "NetManager.h"

static NetManager* netManager;

void net_shutdown();

void net_init()
{
	netManager = new NetManager();

	atexit(net_shutdown);
}

void net_shutdown()
{
	delete netManager;
}

void net_process(lua_State* L)
{
	APP_ASSERT(netManager != NULL);

	netManager->process(L);
}

void net_send(NetChannel channel, NetAddress& address, const char* packet, uint32_t packetLength)
{
	APP_ASSERT(netManager != NULL);

    std::string s = std::string(packet, packetLength);
	netManager->sendPacket(channel, address, s);
}

bool net_get_loopback_packet(NetChannel channel, std::string& packet)
{
	APP_ASSERT(netManager != NULL);

	return netManager->getLoopbackPacket(channel, packet);
}

void net_set_callbacks(ExternalTable& table)
{
	APP_ASSERT(netManager != NULL);

	netManager->setCBTable(table);
}
