#include "../lua_wrappers_common.h"

#include "net.h"

void net_lua_init (lua_State *L);

void push_netaddress (lua_State *L, const NetAddressPtr &self);
void push_netmessage (lua_State *L, const NetMessagePtr &self);