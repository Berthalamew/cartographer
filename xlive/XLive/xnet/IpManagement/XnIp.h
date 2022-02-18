#pragma once 

#include "..\xnet.h"
#include "..\Sockets\XSocket.h"

#define MAX_HDR_STR 32

extern h2log* critical_network_errors_log;

// undefine LOG_CRITICAL_NETWORK for now, to implment it using another h2log that always gets created
// TODO: disable if all network problems are addressed
#undef LOG_CRITICAL_NETWORK

#define LOG_CRITICAL_NETWORK(msg, ...)   LOG_CRITICAL  ((network_log != nullptr ? network_log : critical_network_errors_log), msg, __VA_ARGS__)

const char requestStrHdr[MAX_HDR_STR] = "XNetBrOadPack";
const char broadcastStrHdr[MAX_HDR_STR] = "XNetReqPack";

#define XnIp_ConnectionIndexMask 0xFF000000

#define XnIp_ConnectionTimeOut (15 * 1000) // msec

#define IPADDR_LOOPBACK (htonl(0x7F000001)) // 127.0.0.1

#define XNIP_FLAG(_bit) (1<<(_bit))
#define XNIP_SET_BIT(_val, _bit) ((_val) |= XNIP_FLAG((_bit)))
#define XNIP_TEST_BIT(_val, _bit) (((_val) & XNIP_FLAG((_bit))) != 0)

enum eXnip_ConnectRequestType : int
{
	XnIp_ConnectionRequestInvalid = -1,

	XnIp_ConnectionPing,
	XnIp_ConnectionPong,
	XnIp_ConnectionUpdateNAT,
	XnIp_ConnectionEstablishSecure,
	XnIp_ConnectionDeclareConnected,
	XnIp_ConnectionCloseSecure
};

enum eXnIp_ConnectionRequestBitFlags
{
	XnIp_HasEndpointNATData = 0,

};

enum class H2v_sockets : int
{
	Sock1000 = 0,
	Sock1001
};

struct XNetPacketHeader
{
	DWORD intHdr;
	char HdrStr[MAX_HDR_STR];
};

struct XBroadcastPacket
{
	XBroadcastPacket::XBroadcastPacket()
	{
		pckHeader.intHdr = 'BrOd';
		strncpy(pckHeader.HdrStr, broadcastStrHdr, MAX_HDR_STR);
		ZeroMemory(&data, sizeof(data));
		data.name.sin_addr.s_addr = INADDR_BROADCAST;
	};

	XNetPacketHeader pckHeader;
	struct
	{
		sockaddr_in name;
	} data;
};

struct XNetRequestPacket
{
	XNetRequestPacket()
	{
		pckHeader.intHdr = 'XNeT';
		memset(pckHeader.HdrStr, 0, sizeof(pckHeader.HdrStr));
		strncpy(pckHeader.HdrStr, requestStrHdr, MAX_HDR_STR);
		SecureZeroMemory(&data, sizeof(data));
	}

	XNetPacketHeader pckHeader;
	struct
	{
		XNADDR xnaddr;
		XNKID xnkid;
		BYTE nonceKey[8];
		eXnip_ConnectRequestType reqType;
		union
		{
			struct // XnIp_ConnectionUpdateNAT XnIp_ConnectEstablishSecure
			{
				DWORD flags;
				bool connectionInitiator;
			};
		};
	} data;
};

struct XnKeyPair
{
	bool bValid;
	XNKID xnkid;
	XNKEY xnkey;
};

struct XnIp
{
	IN_ADDR connectionIdentifier;
	XNADDR xnaddr;

	// key we connected with
	XnKeyPair* keyPair;

	bool bValid;
	int connectStatus;
	int connectionPacketsSentCount;
	DWORD lastConnectionInteractionTime;
	DWORD lastPacketReceivedTime;

	BYTE connectionNonce[8];
	BYTE connectionNonceOtherSide[8];
	bool otherSideNonceKeyReceived;

	bool connectionInitiator;

	bool logErrorOnce;

	enum eXnIp_Flags
	{
		XnIp_ConnectDeclareConnectedRequestSent
	};

	DWORD flags;

#pragma region Nat

	struct NatTranslation
	{
		enum class eNatDataState : unsigned int
		{
			natUnavailable,
			natAvailable,
		};

		eNatDataState state;
		sockaddr_in natAddress;
	};
	
	// TODO: add single async socket implementation or figure out another way
	NatTranslation natTranslation[2];

	sockaddr_in* getNatAddr(H2v_sockets natIndex)
	{
		int index = (int)natIndex;
		return &natTranslation[index].natAddress;
	}

	void updateNat(H2v_sockets natIndex, const sockaddr_in* addr)
	{
		int index = (int)natIndex;
		natTranslation[index].natAddress = *addr;
		natTranslation[index].state = NatTranslation::eNatDataState::natAvailable;
	}

	void natDiscard()
	{
		for (int i = 0; i < ARRAYSIZE(natTranslation); i++)
		{
			memset(&natTranslation[i], 0, sizeof(*natTranslation));
			natTranslation[i].state = NatTranslation::eNatDataState::natUnavailable;
		}
	}

	bool natIsUpdated(int natIndex) const
	{
		return natTranslation[natIndex].state == NatTranslation::eNatDataState::natAvailable;
	}

	bool natIsUpdated() const
	{
		for (int i = 0; i < ARRAYSIZE(natTranslation); i++)
		{
			if (natTranslation[i].state != NatTranslation::eNatDataState::natAvailable)
				return false;
		}
		return true;
	}
#pragma endregion

	unsigned int pckSent;
	unsigned int pckRecvd;

	unsigned int bytesSent;
	unsigned int bytesRecvd;

	IN_ADDR getOnlineIpAddress() const
	{
		return xnaddr.inaOnline;
	}

	IN_ADDR getLanIpAddr() const
	{
		return xnaddr.ina;
	}

	bool XnIp::isValid(IN_ADDR identifier) const
	{
		if (identifier.s_addr != this->connectionIdentifier.s_addr)
		{
			LOG_CRITICAL_NETWORK("{} - connection identifier different {:X} != {:X}", __FUNCTION__, identifier.s_addr, connectionIdentifier.s_addr);
			return false;
		}

		return bValid && identifier.s_addr == this->connectionIdentifier.s_addr;
	}
};

class CXnIp
{
public:

	CXnIp()
	{
		memset(&startupParams, 0, sizeof(startupParams));
	}

	~CXnIp()
	{
		// TODO maybe terminate all connections
	}

	CXnIp::CXnIp(const CXnIp& other) = delete;
	CXnIp::CXnIp(CXnIp&& other) = delete;

	// gets the actual connection index from a connection identifier
	static int getConnectionIndex(IN_ADDR connectionIdentifier)
	{
		return (int)(connectionIdentifier.s_addr >> 24);
	}

	XnIp* getConnection(const IN_ADDR ina) const
	{
		XnIp* xnIp = &XnIPs[getConnectionIndex(ina)];
		return xnIp->isValid(ina) ? xnIp : nullptr;
	}

	void setTimeConnectionInteractionHappened(IN_ADDR ina)
	{
		XnIp* xnIp = getConnection(ina);
		if (xnIp != nullptr)
			xnIp->lastConnectionInteractionTime = timeGetTime();
	}

	void updatePacketReceivedCounters(IN_ADDR ipIdentifier, int bytesRecvdCount)
	{
		setTimeConnectionInteractionHappened(ipIdentifier);
		XnIp* xnIp = getConnection(ipIdentifier);
		if (xnIp != nullptr)
		{
			xnIp->pckRecvd++;
			xnIp->bytesRecvd += bytesRecvdCount;
			xnIp->lastPacketReceivedTime = timeGetTime();
		}
	}

	void clearLostConnections();
	void UnregisterXnIpIdentifier(const IN_ADDR ina);

	static void UnregisterLocalConnectionInfo()
	{
		ZeroMemory(&ipLocal, sizeof(ipLocal));
	}

	static XnIp* GetLocalUserXn()
	{
		return ipLocal.bValid ? &ipLocal : nullptr;
	}

	static void SetupLocalConnectionInfo(unsigned long xnaddr, unsigned long lanaddr, unsigned short baseport, const char* abEnet, const char* abOnline);

	int handleRecvdPacket(XSocket* xsocket, sockaddr_in* lpFrom, WSABUF* lpBuffers, LPDWORD bytesRecvdCount);

	void Initialize(const XNetStartupParams* netStartupParams);
	
	void LogConnectionsToConsole();
	void LogConnectionsErrorDetails(const sockaddr_in* address, int errorCode, const XNKID* receivedKey);
	int GetEstablishedConnectionIdentifierByRecvAddr(XSocket* xsocket, const sockaddr_in* addr, IN_ADDR* outConnectionIdentifier);
	
	void SaveNatInfo(XSocket* xsocket, IN_ADDR ipIdentifier, const sockaddr_in* addr);

	int CreateXnIpIdentifierFromPacket(const XNADDR* pxna, const XNKID* xnkid, const XNetRequestPacket* reqPacket, IN_ADDR* outIpIdentifier);
	void HandleXNetRequestPacket(XSocket* xsocket, const XNetRequestPacket* reqPaket, const sockaddr_in* recvAddr, LPDWORD lpBytesRecvdCount);
	void HandleConnectionPacket(XSocket* xsocket, XnIp* xnIp, const XNetRequestPacket* conReqPacket, const sockaddr_in* recvAddr, LPDWORD lpBytesRecvdCount);
	void HandleDisconnectPacket(XSocket* xsocket, const XNetRequestPacket* disconnectReqPck, const sockaddr_in* recvAddr);

	XnIp* XnIpLookUp(const XNADDR* pxna, const XNKID* xnkid, bool* firstUnusedIndexFound = nullptr, int* firstUnusedIndex = nullptr);
	int registerNewXnIp(int connectionIndex, const XNADDR* pxna, const XNKID* pxnkid, IN_ADDR* outIpIdentifier);
	
	int RegisterKey(XNKID*, XNKEY*);
	void UnregisterKey(const XNKID* xnkid);
	XnKeyPair* getKeyPair(const XNKID* xnkid);
	
	XNetStartupParams startupParams;
	int GetMaxXnConnections() { return startupParams.cfgSecRegMax; }
	int GetReqQoSBufferSize() { return startupParams.cfgQosDataLimitDiv4 * 4; }
	int GetMaxXnKeyPairs() { return startupParams.cfgKeyRegMax; }

	int GetMinSockRecvBufferSizeInBytes() { return startupParams.cfgSockDefaultRecvBufsizeInK * SOCK_K_UNIT; }
	int GetMinSockSendBufferSizeInBytes() { return startupParams.cfgSockDefaultSendBufsizeInK * SOCK_K_UNIT; }

	int GetRegisteredKeyCount()
	{
		int keysCount = 0;
		for (int i = 0; i < GetMaxXnKeyPairs(); i++)
		{
			if (XnKeyPairs[i].bValid)
			{
				keysCount++;
			}
		}

		return keysCount;
	}

	/*
		Sends a request over the socket to the other socket end, with the same identifier
	*/
	void SendXNetRequest(XSocket* xsocket, IN_ADDR connectionIdentifier, eXnip_ConnectRequestType reqType);

	/*
		Sends a request to all open sockets
	*/
	void SendXNetRequestAllSockets(IN_ADDR connectionIdentifier, eXnip_ConnectRequestType reqType);

	// Data
	XnIp* XnIPs = nullptr;
	XnKeyPair* XnKeyPairs = nullptr;

private:

	static XnIp ipLocal;
};

extern CXnIp gXnIp;

int WINAPI XNetRegisterKey(XNKID *pxnkid, XNKEY *pxnkey);