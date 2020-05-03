#include "Globals.h"

#include "..\Cryptography\Rc4.h"
#include "XLive\xnet\upnp.h"
#include "XLive\xnet\Sockets\XSocket.h"
#include "XLive\xnet\IpManagement\XnIp.h"
#include "H2MOD\Modules\Config\Config.h"
#include "H2MOD\Modules\Networking\NetworkStats\NetworkStats.h"

int MasterState = 0;
XECRYPT_RC4_STATE Rc4StateRand;
XSocket* game_network_data_gateway_socket_1000; // used for game data
XSocket* game_network_message_gateway_socket_1001; // used for messaging like connection requests

void ForwardPorts()
{
	if (h2mod->Server)
		return;

	ModuleUPnP upnp;

	upnp.UPnPForwardPort(false, H2Config_base_port, H2Config_base_port, "Halo2");
	upnp.UPnPForwardPort(false, (H2Config_base_port + 1), (H2Config_base_port + 1), "Halo2_1");
	//upnp.UPnPForwardPort(false, (H2Config_base_port + 5), (H2Config_base_port + 5), "Halo2_2");
	//upnp.UPnPForwardPort(false, (H2Config_base_port + 6), (H2Config_base_port + 6), "Halo2_3");
	upnp.UPnPForwardPort(true, (H2Config_base_port + 10), (H2Config_base_port + 10), "Halo2_QoS");

	LOG_TRACE_NETWORK("ForwardPorts - Finished forwarding ports.");
}

// #5310: XOnlineStartup
int WINAPI XOnlineStartup()
{
	LOG_TRACE_NETWORK("XOnlineStartup()");
	std::thread(ForwardPorts).detach();

	return ERROR_SUCCESS;
}

// #3: XSocketCreate
SOCKET WINAPI XSocketCreate(int af, int type, int protocol)
{
	LOG_TRACE_NETWORK("XSocketCreate() - af = {0}, type = {1}, protocol = {2}", af, type, protocol);

	if (protocol == IPPROTO_TCP)
		return SOCKET_ERROR; // we dont support TCP yet

	// TODO: support TCP
	XSocket* newXSocket = new XSocket;
	if (protocol == IPPROTO_UDP)
	{
		newXSocket->protocol = IPPROTO_UDP;
	}
	else if (protocol == IPPROTO_VDP)
	{
		protocol = IPPROTO_UDP; // We can't support VDP (Voice / Data Protocol) it's some encrypted crap which isn't standard.
		newXSocket->protocol = IPPROTO_UDP; 
		newXSocket->isVoiceSocket = true;
	}

	SOCKET ret = socket(af, type, protocol);

	if (ret == INVALID_SOCKET)
	{
		LOG_ERROR_NETWORK("XSocketCreate() - Invalid socket, last error: ", WSAGetLastError());
		delete newXSocket;
		return ret;
	}

	newXSocket->WinSockHandle = ret;

	if (newXSocket->isVoiceSocket)
	{
		LOG_TRACE_NETWORK("XSocketCreate() - Socket: {} was VDP", ret);
	}

	ipManager.SocketPtrArray.push_back(newXSocket);

	return (SOCKET)newXSocket;
}

// #4
int WINAPI XSocketClose(SOCKET s)
{
	XSocket* xsocket = (XSocket*)s;
	LOG_TRACE_NETWORK("XSocketClose() - socket: {}", xsocket->WinSockHandle);

	int ret = closesocket(xsocket->WinSockHandle);

	for (auto i = ipManager.SocketPtrArray.begin(); i != ipManager.SocketPtrArray.end(); ++i)
	{
		if (*i == xsocket) 
		{
			ipManager.SocketPtrArray.erase(i);
			break;
		}
	}

	delete xsocket;

	return ret;
}

// #52: XNetCleanup
INT WINAPI XNetCleanup()
{
	LOG_TRACE_NETWORK("XNetCleanup()");
	for (auto xsocket : ipManager.SocketPtrArray)
	{
		XSocketClose((SOCKET)xsocket);
		delete xsocket;
	}
	ipManager.SocketPtrArray.clear();

	return 0;
}

// #11: XSocketBind
SOCKET WINAPI XSocketBind(SOCKET s, const struct sockaddr *name, int namelen)
{
	XSocket* xsocket = (XSocket*)s;
	u_short port = (((struct sockaddr_in*)name)->sin_port);

	// TODO: support TCP
	if (xsocket->protocol == IPPROTO_UDP)
		xsocket->nPort = htons(port);

	if (htons(port) == 1000) {
		game_network_data_gateway_socket_1000 = xsocket;
		(((struct sockaddr_in*)name)->sin_port) = ntohs(H2Config_base_port);
		LOG_TRACE_NETWORK("XSocketBind() - replaced port {} with {}", htons(port), H2Config_base_port);
	}

	if (htons(port) == 1001) {
		game_network_message_gateway_socket_1001 = xsocket;
		(((struct sockaddr_in*)name)->sin_port) = ntohs(H2Config_base_port + 1);
		LOG_TRACE_NETWORK("XSocketBind() - replaced port {} with {}", htons(port), H2Config_base_port + 1);
	}

	if (htons(port) == 1005)
		(((struct sockaddr_in*)name)->sin_port) = ntohs(H2Config_base_port + 5);
	if (htons(port) == 1006)
		(((struct sockaddr_in*)name)->sin_port) = ntohs(H2Config_base_port + 6);

	int ret = bind(xsocket->WinSockHandle, name, namelen);

	if (ret == SOCKET_ERROR)
		LOG_TRACE_NETWORK("XSocketBind() - SOCKET_ERROR");

	return ret;
}

// #53: XNetRandom
INT WINAPI XNetRandom(BYTE * pb, UINT cb)
{
	static bool Rc4CryptInitialized = false;

	LARGE_INTEGER key;

	if (Rc4CryptInitialized == false)
	{
		QueryPerformanceCounter(&key);
		XeCryptRc4Key(&Rc4StateRand, (BYTE*)&key, sizeof(key));
		Rc4CryptInitialized = true;
	}

	XeCryptRc4Ecb(&Rc4StateRand, pb, cb);
	return ERROR_SUCCESS;
}

// #24: XSocketSendTo
int WINAPI XSocketSendTo(SOCKET s, const char *buf, int len, int flags, sockaddr *to, int tolen)
{
	XSocket* xsocket = (XSocket*)s;

	if (((struct sockaddr_in*)to)->sin_addr.s_addr == INADDR_BROADCAST) // handle broadcast
	{
		(((struct sockaddr_in*)to)->sin_addr.s_addr) = H2Config_master_ip;
		((struct sockaddr_in*)to)->sin_port = ntohs(H2Config_master_port_relay);
			
		//LOG_TRACE_NETWORK_N("XSocketSendTo - Broadcast");

		int result = sendto(xsocket->WinSockHandle, buf, len, flags, (sockaddr*)to, sizeof(sockaddr));
		
		return result;
	}

	/*
		Create new SOCKADDR_IN structure,
		If we overwrite the original the game's security functions know it's not a secure address any longer.
		Worst case if this is found to cause performance issues we can handle the send and re-update to secure before return.
	*/

	u_long connectionIndex = ipManager.getConnectionIndex(((struct sockaddr_in*)to)->sin_addr);
	XnIp* xnIp = &ipManager.XnIPs[connectionIndex];

	sockaddr_in sendAddress;
	sendAddress.sin_family = AF_INET;
	sendAddress.sin_addr = xnIp->xnaddr.ina;
	sendAddress.sin_port = ((struct sockaddr_in*)to)->sin_port;

	//LOG_TRACE_XLIVE("SendStruct.sin_addr.s_addr == %08X", sendAddress.sin_addr.s_addr);

	if (sendAddress.sin_addr.s_addr == H2Config_ip_wan)
	{
		sendAddress.sin_addr.s_addr = H2Config_ip_lan;
		//LOG_TRACE_XLIVE("Replaced send struct s_addr with g_lLANIP: %08X", H2Config_ip_lan);
	}

	/*
	 Handle NAT map socket to port
	 Switch on port to determine which port we're intending to send to.
	 1000-> User.pmap_a[secureaddress]
	 1001-> User.pmap_b[secureaddress]
	*/

	/* TODO: handle this dynamically */
	u_short hPort = 0;
	switch (xsocket->nPort)
	{
	case 1000:
		hPort = xnIp->NatAddrSocket1000.sin_port;

		if (hPort != 0)
		{
			//LOG_TRACE_XLIVE("XSocketSendTo() port 1000 nPort: {} secure: %08X", htons(nPort), iplong);
			sendAddress.sin_port = hPort;
		}
		else 
		{
			sendAddress.sin_port = xnIp->xnaddr.wPortOnline;
		}

		break;

	case 1001:
		hPort = xnIp->NatAddrSocket1001.sin_port;

		if (hPort != 0)
		{
			//LOG_TRACE_XLIVE("XSocketSendTo() port 1001 nPort: %i secure: %08X", htons(nPort), iplong);
			sendAddress.sin_port = hPort;
		}
		else
		{
			sendAddress.sin_port = ntohs(htons(xnIp->xnaddr.wPortOnline) + 1);
		}

		break;

	default:
		//LOG_TRACE_XLIVE("XSocketSendTo() port: %i not matched!", htons(port));
		break;
	}

	int result = sendto(xsocket->WinSockHandle, buf, len, flags, (const sockaddr*)&sendAddress, sizeof(sendAddress));

	if (result == SOCKET_ERROR)
	{
		LOG_ERROR_NETWORK("XSocketSendTo() - Socket Error: {}", WSAGetLastError());
		return SOCKET_ERROR;
	}
	else
	{
		updateSendToStatistics(result);
	}

	return result;
}

// #20
int WINAPI XSocketRecvFrom(SOCKET s, char *buf, int len, int flags, sockaddr *from, int *fromlen)
{
	XSocket* xsocket = (XSocket*)s;
	int result = recvfrom(xsocket->WinSockHandle, buf, len, flags, from, fromlen);

	if (result == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSAEWOULDBLOCK)
			LOG_ERROR_NETWORK("XSocketRecvFrom() - Socket Error: {}", WSAGetLastError());
		return SOCKET_ERROR;
	}
	else if (result > 0)
	{
		u_long iplong = ((struct sockaddr_in*)from)->sin_addr.s_addr;

		XNetConnectionReqPacket* connectionPck = reinterpret_cast<XNetConnectionReqPacket*>(buf);
		if (iplong == H2Config_master_ip)
		{
			return result;
		}
		else if (result == sizeof(XNetConnectionReqPacket)
			&& connectionPck->ConnectPacketIdentifier == ipManager.connectPacketIdentifier)
		{
			LOG_TRACE_NETWORK("XSocketRecvFrom() - Received secure packet with ip address {:x}, port: {}", htonl(iplong), htons(((struct sockaddr_in*)from)->sin_port));
			ipManager.HandleConnectionPacket(xsocket, &connectionPck->xnaddr, &connectionPck->xnkid, from); // save NAT info and send back a packet
			return 0;
		}
		else
		{
			IN_ADDR ipIdentifier = ipManager.GetConnectionIdentifierByNat(from);

			if (ipIdentifier.s_addr != 0)
			{
				ipManager.setTimePacketReceived(ipIdentifier, timeGetTime());
			}

			((struct sockaddr_in*)from)->sin_addr = ipIdentifier;	
		}
	}
	

	/*if (ret > 0)
	{
		LOG_TRACE_NETWORK("XSocketRecvFrom() received socket data, total: {}", bytesReceived);
	}*/

	return result;
}

