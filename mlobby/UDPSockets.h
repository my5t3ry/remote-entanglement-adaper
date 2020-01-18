#ifndef __UDPSOCKET__
#define __UDPSOCKET__

#include <vector>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <thread>
#include <mutex>
#include "socketHelper.h"

class udpServerSock
{
private:
	uint64_t lastTime;
	uint16_t port;
	SOCKET sock;
	std::vector<client_t> clients;
	std::thread *listenThd;
	std::mutex top_mutex, sock_mutex;
	bool closing, closed;
	void (*messageHandler)(msg_t message);
	static void *listen(void *serv);
public:
	udpServerSock(uint16_t port);
	size_t sendMessage(size_t client = 0xffffffff, msg_t *message = NULL);
	void setMessageHandler(void(*msgHandler)(msg_t message));
	void sendDisconnectMsg(size_t client = 0xffffffff);
	size_t numClients();
	bool isClosed();
	~udpServerSock();
};

class udpClientSock
{
private:
	uint64_t lastTime;
	uint16_t port;
	sockaddr_in host;
	SOCKET sock;
	std::thread *listenThd;
	std::mutex top_mutex, sock_mutex;
	bool closing, closed;
	void (*messageHandler)(msg_t message);
	static void *listen(void *cli);
public:
	udpClientSock(const char *ip, uint16_t port, uint16_t localPort);
	size_t sendMessage(msg_t *message);
	void setMessageHandler(void(*msgHandler)(msg_t message));
	void sendDisconnectMsg();
	bool isDisconnected();
	~udpClientSock();
};

#endif
