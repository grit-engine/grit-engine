#include <cstdio>

#include "../centralised_log.h"

#include "net.h"

NetAddress::NetAddress()
{
	this->type = NetAddress_Loopback;
}

NetAddress::NetAddress(NetAddressType type)
{
	this->type = type;
}

NetAddress::NetAddress(sockaddr* addr, int length)
{
	switch (addr->sa_family)
	{
		case AF_INET:
			{
				sockaddr_in* in_addr = (sockaddr_in*)addr;
				this->type = NetAddress_IPv4;
				this->port = ntohs(in_addr->sin_port);
				this->ip4 = ntohl(in_addr->sin_addr.s_addr);
			}
			break;
		case AF_INET6:
			{
				sockaddr_in6* in_addr = (sockaddr_in6*)addr;
				this->type = NetAddress_IPv6;
				this->port = ntohs(in_addr->sin6_port);
				memcpy(this->ip6, &in_addr->sin6_addr, sizeof(this->ip6));
			}
			break;
	}
}

NetAddress& NetAddress::getLoopback()
{
	static NetAddress loopbackIP;

	return loopbackIP;
}

NetAddress NetAddress::resolve(const char* host, NetAddressType type)
{
	char hostName[256];
	uint16_t port;

	// [^:] sounds like it'd fail for ipv6?
	if (sscanf(host, "%256[^:]:%hu", hostName, &port) == 2)
	{
		if (!strcmp(hostName, "localhost"))
		{
			return NetAddress(NetAddress_Loopback);
		}

		struct addrinfo* addrInfo;
		if (getaddrinfo(hostName, NULL, NULL, &addrInfo) != 0)
		{
			return NetAddress(NetAddress_Invalid);
		}

		int requestedFamily = (type == NetAddress_IPv6) ? AF_INET6 : AF_INET;

		struct addrinfo* info = addrInfo;
		while (info)
		{
			if (info->ai_family == requestedFamily)
			{
				break;
			}

			info = info->ai_next;
		}

		if (!info)
		{
			freeaddrinfo(addrInfo);
			return NetAddress(NetAddress_Invalid);
		}

		NetAddress retval(info->ai_addr, info->ai_addrlen);
		retval.port = port;

		freeaddrinfo(addrInfo);

		return retval;
	}

	return NetAddress(NetAddress_Invalid);
}

bool NetAddress::operator ==(NetAddress& right)
{
	if (type != right.type)
	{
		return false;
	}

	switch (type)
	{
		case NetAddress_Invalid:
		case NetAddress_Loopback:
			return true;
		case NetAddress_IPv4:
			return (ip4 == right.ip4);
		case NetAddress_IPv6:
			return !memcmp(ip6, right.ip6, sizeof(ip6));
	}

	return false;
}

bool NetAddress::operator !=(NetAddress& right)
{
	return !(*this == right);
}

void NetAddress::getSockAddr(sockaddr_storage* storage, int* length)
{
	memset(storage, 0, sizeof(sockaddr_storage));

	switch (type)
	{
		case NetAddress_Invalid:
		case NetAddress_Loopback:
			GRIT_EXCEPT("can not convert invalid/loopback address to sockaddr");
		case NetAddress_IPv4:
			{
				sockaddr_in* in_addr = (sockaddr_in*)storage;
				in_addr->sin_family = AF_INET;
				in_addr->sin_addr.s_addr = htonl(ip4);
				in_addr->sin_port = htons(port);

				*length = sizeof(sockaddr_in);
			}
			break;
		case NetAddress_IPv6:
			{
				sockaddr_in6* in_addr = (sockaddr_in6*)storage;
				in_addr->sin6_family = AF_INET6;
				memcpy(&in_addr->sin6_addr, ip6, sizeof(ip6));
				in_addr->sin6_port = htons(port);

				*length = sizeof(sockaddr_in6);
			}
			break;
	}
}

std::string NetAddress::toString()
{
	if (type == NetAddress_IPv4) {
		sockaddr_storage storage;
		int storageLen;

		char buffer[16];

		getSockAddr(&storage, &storageLen);

		sockaddr_in* inAddr = (sockaddr_in*)&storage;
		inet_ntop(inAddr->sin_family, &inAddr->sin_addr, buffer, sizeof(buffer));

		std::stringstream ss;
		ss << buffer << ":" << port;

		return ss.str();
	} else if (type == NetAddress_IPv6) {
		GRIT_EXCEPT("IPv6 toString not yet implemented");
	} else if (type == NetAddress_Loopback) {
		return "localhost";
	} else if (type == NetAddress_Invalid) {
		return "INVALID";
	} else {
        return "UNRECOGNISED TYPE";
    }
}
