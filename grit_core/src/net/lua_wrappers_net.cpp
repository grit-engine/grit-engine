#include "../main.h"
#include "../ExternalTable.h"
#include "../LuaPtr.h"
#include "../path_util.h"

#include "net.h"
#include "lua_wrappers_net.h"

#define NETADDRESS_TAG "Grit/NetAddress"

void push_netaddress (lua_State *L, const NetAddressPtr &self)
{
	if (self.isNull())
		lua_pushnil(L);
	else
		push(L,new NetAddressPtr(self), NETADDRESS_TAG);
}

GC_MACRO(NetAddressPtr,netaddress,NETADDRESS_TAG)

static int netaddress_tostring (lua_State *L)
{
TRY_START
	check_args(L,1);
	GET_UD_MACRO(NetAddressPtr,self,1,NETADDRESS_TAG);

	std::string stringAddr = self->toString();

	lua_pushstring(L, stringAddr.c_str());
	return 1;
TRY_END
}

static int netaddress_index (lua_State *L)
{
TRY_START
	check_args(L,2);

	GET_UD_MACRO(NetAddressPtr,self,1,NETADDRESS_TAG);

    (void) self;

	my_lua_error(L,"NetAddress has no properties.");

	return 0;
TRY_END
}

EQ_MACRO(NetAddressPtr, netaddress, NETADDRESS_TAG)

MT_MACRO(netaddress);

// --- NetMessage wrappers ----------------------------------------------------
#define NETMESSAGE_TAG "Grit/NetMessage"

void push_netmessage (lua_State *L, const NetMessagePtr &self)
{
	if (self.isNull())
		lua_pushnil(L);
	else
		push(L,new NetMessagePtr(self), NETMESSAGE_TAG);
}

TOSTRING_SMART_PTR_MACRO(netmessage, NetMessagePtr, NETMESSAGE_TAG)
GC_MACRO(NetMessagePtr, netmessage, NETMESSAGE_TAG)
EQ_MACRO(NetMessagePtr, netmessage, NETMESSAGE_TAG)

static int netmessage_read_bool(lua_State *L)
{
TRY_START
	check_args(L,1);
	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	lua_pushboolean(L, (*self)->readBool());
	
	return 1;
TRY_END
}

static int netmessage_read_integer(lua_State *L)
{
TRY_START
	check_args_min(L, 1);
	check_args_max(L, 2);

	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	int bits = (lua_gettop(L) == 2) ? check_int(L, 2, 1, 32) : 32;

	lua_pushinteger(L, (*self)->readInteger(bits));
	
	return 1;
TRY_END
}

static int netmessage_read_float(lua_State *L)
{
TRY_START
	check_args(L,1);
	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	lua_pushnumber(L, (*self)->readFloat());
	
	return 1;
TRY_END
}

static int netmessage_read_string(lua_State *L)
{
TRY_START
	check_args(L,1);
	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	std::string str = (*self)->readString();

	lua_pushstring(L, str.c_str());
	
	return 1;
TRY_END
}

static int netmessage_read_delta_integer(lua_State *L)
{
TRY_START
	check_args_min(L, 2);
	check_args_max(L, 3);

	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	int old = luaL_checkinteger(L, 2);
	int bits = (lua_gettop(L) == 3) ? check_int(L, 3, 1, 32) : 32;

	lua_pushinteger(L, (*self)->readDeltaInteger(old, bits));
	
	return 1;
TRY_END
}

static int netmessage_read_delta_float(lua_State *L)
{
TRY_START
	check_args(L,2);
	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	float old = luaL_checknumber(L, 2);

	lua_pushnumber(L, (*self)->readDeltaFloat(old));
	
	return 1;
TRY_END
}

static int netmessage_write_bool(lua_State *L)
{
TRY_START
	check_args(L,2);
	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	(*self)->writeBool(check_bool(L, 2));
	
	return 0;
TRY_END
}

static int netmessage_write_integer(lua_State *L)
{
TRY_START
	check_args_min(L, 2);
	check_args_max(L, 3);

	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	int bits = (lua_gettop(L) == 3) ? check_int(L, 3, 1, 32) : 32;

	(*self)->writeInteger(luaL_checkinteger(L, 2), bits);
	
	return 0;
TRY_END
}

static int netmessage_write_float(lua_State *L)
{
TRY_START
	check_args(L,2);
	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	(*self)->writeFloat(check_float(L, 2));
	
	return 0;
TRY_END
}

static int netmessage_write_string(lua_State *L)
{
TRY_START
	check_args(L,2);
	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	std::string str = check_string(L, 2);
	(*self)->writeString(str);
	
	return 0;
TRY_END
}

static int netmessage_write_delta_integer(lua_State *L)
{
TRY_START
	check_args_min(L,3);
	check_args_max(L,4);
	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	int bits = (lua_gettop(L) == 4) ? check_int(L, 4, 1, 32) : 32;
	(*self)->writeDeltaFloat(luaL_checkinteger(L, 2), luaL_checkinteger(L, 3));
	
	return 0;
TRY_END
}

static int netmessage_write_delta_float(lua_State *L)
{
TRY_START
	check_args(L,3);
	GET_UD_MACRO_OFFSET(NetMessagePtr,self,1,NETMESSAGE_TAG,0);

	(*self)->writeDeltaFloat(check_float(L, 2), check_float(L, 3));
	
	return 0;
TRY_END
}

static int netmessage_clone(lua_State *L)
{
TRY_START
	check_args(L, 1);
	GET_UD_MACRO_OFFSET(NetMessagePtr, self, 1, NETMESSAGE_TAG, 0);

	std::string buffer = (*self)->getBuffer();

	push_netmessage(L, NetMessagePtr(new NetMessage(buffer.c_str(), buffer.size())));

	return 1;
TRY_END
}

static int netmessage_index (lua_State *L)
{
TRY_START
	check_args(L,2);
	GET_UD_MACRO(NetMessagePtr,self,1,NETMESSAGE_TAG);
	std::string key = luaL_checkstring(L,2);

	if (key == "read_bool")
	{
		push_cfunction(L, netmessage_read_bool);
	}
	else if (key == "read_int")
	{
		push_cfunction(L, netmessage_read_integer);
	}
	else if (key == "read_float")
	{
		push_cfunction(L, netmessage_read_float);
	}
	else if (key == "read_string")
	{
		push_cfunction(L, netmessage_read_string);
	}
	else if (key == "read_delta_int")
	{
		push_cfunction(L, netmessage_read_delta_integer);
	}
	else if (key == "read_delta_float")
	{
		push_cfunction(L, netmessage_read_delta_float);
	}
	else if (key == "write_bool")
	{
		push_cfunction(L, netmessage_write_bool);
	}
	else if (key == "write_int")
	{
		push_cfunction(L, netmessage_write_integer);
	}
	else if (key == "write_float")
	{
		push_cfunction(L, netmessage_write_float);
	}
	else if (key == "write_string")
	{
		push_cfunction(L, netmessage_write_string);
	}
	else if (key == "write_delta_int")
	{
		push_cfunction(L, netmessage_write_delta_integer);
	}
	else if (key == "write_delta_float")
	{
		push_cfunction(L, netmessage_write_delta_float);
	}
	else if (key == "clone")
	{
		push_cfunction(L, netmessage_clone);
	}
	else if (key == "length")
	{
		lua_pushinteger(L, self->getLength());
	}
	else
	{
		my_lua_error(L, "Not a valid NetMessage field: " + key);
	}
	
	return 1;
TRY_END
}

MT_MACRO(netmessage);

static int global_net_register_callbacks(lua_State *L)
{
TRY_START

	check_args(L, 1);

	if (!lua_istable(L, 1))
		my_lua_error(L, "First parameter should be a table");

	ExternalTable table;

	int index = lua_gettop(L);
	for (lua_pushnil(L) ; lua_next(L,index)!=0 ; lua_pop(L,1)) {
		const char *err = table.luaSet(L);
		if (err) my_lua_error(L, err);
	}
	lua_pop(L,1);

	net_set_callbacks(table);

	return 0;

TRY_END
}

static int global_net_process(lua_State* L)
{
TRY_START

	check_args(L, 0);

	net_process(L);

	return 0;
TRY_END
}

static int global_net_make_message(lua_State* L)
{
TRY_START

	check_args(L, 0);

	push_netmessage(L, NetMessagePtr(new NetMessage()));

	return 1;

TRY_END
}

static int global_net_get_loopback_packet(lua_State* L)
{
TRY_START

	check_args(L, 1);

	std::string str = check_string(L, 1);
	NetChannel channel;

	if (str == "server")
	{
		channel = NetChan_ClientToServer;
	}
	else if (str == "client")
	{
		channel = NetChan_ServerToClient;
	}
	else
	{
		my_lua_error(L, "invalid network channel: " + str);
	}

	std::string packet;
	if (!net_get_loopback_packet(channel, packet))
	{
		lua_pushnil(L);
	}
	else
	{
		push_netmessage(L, NetMessagePtr(new NetMessage(packet.c_str(), packet.size())));
	}

	return 1;

TRY_END
}

static int global_net_send_packet(lua_State* L)
{
TRY_START

	check_args(L, 3);
	std::string str = check_string(L, 1);
	GET_UD_MACRO_OFFSET(NetAddressPtr,address,2,NETADDRESS_TAG,0);
	GET_UD_MACRO_OFFSET(NetMessagePtr,message,3,NETMESSAGE_TAG,0);

	NetChannel channel;

	if (str == "server")
	{
		channel = NetChan_ClientToServer;
	}
	else if (str == "client")
	{
		channel = NetChan_ServerToClient;
	}
	else
	{
		my_lua_error(L, "invalid network channel: " + str);
	}

	std::string buffer = (*message)->getBuffer();

	net_send(channel, **address, buffer.c_str(), buffer.size());

	return 0;

TRY_END
}

static int global_net_send_packet_sequenced(lua_State* L)
{
TRY_START

	check_args(L, 4);
	std::string str = check_string(L, 1);
	GET_UD_MACRO_OFFSET(NetAddressPtr,address,2,NETADDRESS_TAG,0);
	GET_UD_MACRO_OFFSET(NetMessagePtr,message,3,NETMESSAGE_TAG,0);
	int sequenceNum = luaL_checkinteger(L, 4);

	NetChannel channel;

	if (str == "server")
	{
		channel = NetChan_ClientToServer;
	}
	else if (str == "client")
	{
		channel = NetChan_ServerToClient;
	}
	else
	{
		my_lua_error(L, "invalid network channel: " + str);
	}

	std::string buffer = (*message)->getBuffer();
	NetMessage tempMessage;
	tempMessage.writeInteger(sequenceNum);
	tempMessage.writeBits(buffer.size() * 8, buffer.c_str());

	buffer = tempMessage.getBuffer();

	net_send(channel, **address, buffer.c_str(), buffer.size());

	return 0;

TRY_END
}

static int global_net_resolve_address(lua_State* L)
{
TRY_START

	check_args(L, 1);

	std::string str = check_string(L, 1);

	NetAddress address = NetAddress::resolve(str.c_str(), NetAddress_IPv4);
	push_netaddress(L, NetAddressPtr(new NetAddress(address)));

	return 1;

TRY_END
}

static const luaL_reg global[] = {
	{"net_register_callbacks", global_net_register_callbacks},
	{"net_process", global_net_process},
	{"net_make_message", global_net_make_message},
	{"net_get_loopback_packet", global_net_get_loopback_packet},
	{"net_send_packet", global_net_send_packet},
	{"net_send_packet_sequenced", global_net_send_packet_sequenced},
	{"net_resolve_address", global_net_resolve_address},
	{NULL,NULL}
};

void net_lua_init (lua_State *L)
{
	ADD_MT_MACRO(netaddress, NETADDRESS_TAG);
	ADD_MT_MACRO(netmessage, NETMESSAGE_TAG);

	register_lua_globals(L, global);
}
