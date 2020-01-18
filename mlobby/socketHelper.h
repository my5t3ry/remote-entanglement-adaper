#ifndef __SOCKET_HELP_H_
#define __SOCKET_HELP_H_

#include <cstring>
#include <arpa/inet.h>

inline void itoa(int n, char *str, int radix)
{
	sprintf(str, "%d", n);
}

typedef int SOCKET;

#define DEFAULT_PORT 16567
#define LOCALHOST 0x0100007F

#define SOCKET_ERROR	-1
#define INVALID_SOCKET 	-1
#define MTU 1500

enum command
{
	CONNECT_MSG, ACK_MSG, DISCONNECT_MSG, TEXT, TEXT_ACK, PING, PING_RESPONSE, DATA
};

inline const char *getComm(uint32_t msg)
{
	switch(msg)
	{
		case CONNECT_MSG:
			return "CONNECT_MSG";
			break;
		case ACK_MSG:
			return "ACK_MSG";
			break;
		case DISCONNECT_MSG:
			return "DISCONNECT_MSG";
			break;
		case TEXT:
			return "TEXT";
			break;
		case PING:
			return "PING";
			break;
		case PING_RESPONSE:
			return "PING_RESPONSE";
			break;
		case DATA:
			return "DATA";
			break;
		default:
			break;
	}
	return "UNKNOWN_MSG";
}

const size_t messageHeader = sizeof(uint64_t) + 2*sizeof(uint32_t);

typedef struct
{
	uint64_t timestamp;
	uint32_t type;
	uint32_t size;
	uint8_t data[MTU - messageHeader];
}msg_t;

msg_t makeMessage(uint32_t type, void* data = NULL, size_t bytes = 0);

typedef struct
{
	uint64_t id;
	sockaddr_in addr;
	SOCKET tcpSocket;
	uint64_t lastMessage;	
}client_t;

inline void sprintaddr(const sockaddr_in &addr, char *buffer, size_t bufferlen)
{
	snprintf(buffer, bufferlen, "%d.%d.%d.%d:%d", 
						((unsigned char*)&addr.sin_addr.s_addr)[0],
						((unsigned char*)&addr.sin_addr.s_addr)[1],
						((unsigned char*)&addr.sin_addr.s_addr)[2],
						((unsigned char*)&addr.sin_addr.s_addr)[3],
						addr.sin_port);
}

inline void sprintaddr(const uint32_t &addr, char *buffer, size_t bufferlen)
{
	snprintf(buffer, bufferlen, "%d.%d.%d.%d", ((unsigned char*)&addr)[0],
						((unsigned char*)&addr)[1], ((unsigned char*)&addr)[2],
						((unsigned char*)&addr)[3]);
}

inline bool sameClient(const client_t &a, const client_t &b)
{
	return ((a.addr.sin_addr.s_addr == b.addr.sin_addr.s_addr)&&(a.addr.sin_port == b.addr.sin_port));
}

inline bool sameClient(const sockaddr_in &a, const sockaddr_in &b)
{
	return ((a.sin_addr.s_addr == b.sin_addr.s_addr)&&(a.sin_port == b.sin_port));
}

inline msg_t makeMessage(uint32_t type, void* data, size_t bytes)
{
	msg_t out;
	memset((void*)&out, 0, sizeof(msg_t));
	out.type = type;
	out.size = messageHeader + bytes;
	memcpy(out.data, data, (((MTU - messageHeader) > bytes)?bytes:(MTU-messageHeader)));
	return out;

}


#endif