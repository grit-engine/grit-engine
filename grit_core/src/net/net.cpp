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
	APP_ASSERT(netManager != nullptr);

	netManager->process(L);
}

void net_send(NetChannel channel, NetAddress& address, const char* packet, uint32_t packetLength)
{
	APP_ASSERT(netManager != nullptr);

	netManager->sendPacket(channel, address, std::string(packet, packetLength));
}

bool net_get_loopback_packet(NetChannel channel, std::string& packet)
{
	APP_ASSERT(netManager != nullptr);

	return netManager->getLoopbackPacket(channel, packet);
}

void net_set_callbacks(ExternalTable& table)
{
	APP_ASSERT(netManager != nullptr);

	netManager->setCBTable(table);
}