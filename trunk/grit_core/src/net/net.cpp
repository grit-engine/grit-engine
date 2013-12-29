#include <sleep.h>

#include "../CentralisedLog.h"
#include "net.h"

#include "NetManager.h"

static NetManager* netManager;

void net_init()
{
	netManager = new NetManager();
}

void net_shutdown(lua_State *L)
{
	netManager->getCBTable().clear(L); //closing LuaPtrs
	
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

void net_set_callbacks(ExternalTable& table, lua_State* L)
{
	APP_ASSERT(netManager != NULL);

	netManager->setCBTable(table);
	
	table.clear(L); //ensure setNil() is called so we don't get LuaPtr shutdown error
}
