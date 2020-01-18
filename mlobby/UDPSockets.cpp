#include "UDPSockets.h"
#include <cstring>
#include <cstdlib>
#include <arpa/inet.h>
#include <errno.h>

udpServerSock::udpServerSock(uint16_t port):
lastTime(0),
port(port),
closing(false),
closed(false),
messageHandler(NULL)
{
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock == INVALID_SOCKET)
	{
		perror("socket error");
		return;
	}
	sockaddr_in me;
	me.sin_family = AF_INET;
	me.sin_port = port;
	me.sin_addr.s_addr = INADDR_ANY;
	if(SOCKET_ERROR == bind(sock, (sockaddr*)&me, sizeof(sockaddr_in)))
	{
		perror("Bind error");
		return;
	}
	printf("Socket Bound to port %d\n", me.sin_port);
	listenThd = new std::thread(listen, (void*)this);
}

size_t udpServerSock::sendMessage(size_t client, msg_t *message)
{
	if(message == NULL)
		return 0;
	if(message->size == 0)
		return 0;
	sock_mutex.lock();
	size_t bytes_sent = 0;
	top_mutex.lock();
	message->timestamp = lastTime++;
	top_mutex.unlock();
	if(message->timestamp == 0)
	{
		message->timestamp = lastTime++;
		msg_t ping;
		ping.type = PING;
		ping.timestamp = 0;
		ping.size = messageHeader;
		for(size_t i = 0; i < clients.size(); ++i)
		{
			sendto(sock, (char*)&ping, ping.size, 0, (sockaddr*)&clients[i].addr,
				sizeof(sockaddr_in));
		}
	}
	if(client == 0xffffffff)
	{
		for(size_t i = 0; i < clients.size(); ++i)
		{
			bytes_sent += sendto(sock, (char*)message, message->size, 0, (sockaddr*)&clients[i].addr,
				sizeof(sockaddr_in));
		}
	}
	else if(client < clients.size())
		bytes_sent += sendto(sock, (char*)message, message->size, 0, (sockaddr*)&clients[client].addr,
			sizeof(sockaddr_in));
	sock_mutex.unlock();
	return bytes_sent;
}

void udpServerSock::sendDisconnectMsg(size_t client)
{
	msg_t message;
	message.type = DISCONNECT_MSG;
	message.size = messageHeader;
	top_mutex.lock();
	message.timestamp = lastTime++;
	top_mutex.unlock();
	sock_mutex.lock();
	if(client == 0xffffffff)
	{
		for(size_t i = 0; i < clients.size(); ++i)
		{
			sendto(sock, (char*)&message, messageHeader, 0, (sockaddr*)&clients[i].addr,
				sizeof(sockaddr_in));
		}
	}
	else if(client < clients.size())
		sendto(sock, (char*)&message, messageHeader, 0, (sockaddr*)&clients[client].addr,
			sizeof(sockaddr_in));
	sock_mutex.unlock();
}

bool udpServerSock::isClosed()
{
	top_mutex.lock();
	bool out = closed;
	top_mutex.unlock();
	return out;
}

size_t udpServerSock::numClients()
{
	top_mutex.lock();
	size_t out = clients.size();
	top_mutex.unlock();
	return out;
}

void *udpServerSock::listen(void *serv)
{
	udpServerSock *server = (udpServerSock*)serv;
	sockaddr_in src;
	socklen_t addr_len;
	msg_t buffer;
	server->top_mutex.lock();
	while(!server->closing)
	{
		server->top_mutex.unlock();
		memset((void*)&src, 0, sizeof(sockaddr_in));
		addr_len = sizeof(sockaddr_in);
		server->sock_mutex.lock();
		int received = recvfrom(server->sock, (void*)&buffer, sizeof(msg_t), MSG_DONTWAIT,
			(sockaddr*)&src, &addr_len);
		server->sock_mutex.unlock();
		if(received > 0)
		{
			std::vector<client_t>::iterator i = server->clients.end();
			for(i = server->clients.begin(); i != server->clients.end(); ++i)
			{
				if((src.sin_port == (*i).addr.sin_port) &&
				(src.sin_addr.s_addr == (*i).addr.sin_addr.s_addr))
					break;
			}
			if(i == server->clients.end()) // New Client
			{
				if(buffer.type == CONNECT_MSG)
				{
					char addr_s[32];
					sprintaddr(src, addr_s, 32);
					printf("Connection from: %s\n", addr_s);
					client_t conn;
					conn.addr = sockaddr_in(src);
					conn.lastMessage = buffer.timestamp;
					server->clients.push_back(conn);
					msg_t ack;
					ack.size = messageHeader;
					ack.type = ACK_MSG;
					server->sock_mutex.lock();
					sendto(server->sock, (char*)&ack, ack.size, 0, (sockaddr*)&src, sizeof(src));
					server->sock_mutex.unlock();
				}
				else
				{
					server->top_mutex.lock();
					continue;
				}
			}
			else if((buffer.timestamp <= (*i).lastMessage)&&(buffer.timestamp != 0))
			{
				server->top_mutex.lock();
				continue;
			}
			else
			{
				(*i).lastMessage = buffer.timestamp;
				if(buffer.type == PING)
				{
					buffer.type = PING_RESPONSE;
					server->top_mutex.lock();
					buffer.timestamp = server->lastTime++;
					server->sock_mutex.lock();
					sendto(server->sock, (char*)&buffer, buffer.size, 0, (sockaddr*)&src, sizeof(src));
					server->sock_mutex.unlock();
					continue;
				}
			}
			//printf("Bytes Received:%d\n", received);
			//printf("Command Received: %s\n", getComm(buffer.type));
			size_t bytesSent = 0;
			switch(buffer.type)
			{
				case CONNECT_MSG:
					buffer.size = messageHeader;
					buffer.type = ACK_MSG;
					server->sock_mutex.lock();
					sendto(server->sock, (char*)&buffer, buffer.size, 0, (sockaddr*)&src, sizeof(src));
					msg_t notify;
					notify.type = TEXT;
					notify.timestamp = ++server->lastTime;
					snprintf((char*)notify.data, MTU - messageHeader, "%d.%d.%d.%d:%d connected",
						((unsigned char*)&src.sin_addr.s_addr)[0],
						((unsigned char*)&src.sin_addr.s_addr)[1],
						((unsigned char*)&src.sin_addr.s_addr)[2],
						((unsigned char*)&src.sin_addr.s_addr)[3],
						src.sin_port);
					notify.size = messageHeader + strlen((char*)notify.data) +1;
					for(std::vector<client_t>::iterator j = server->clients.begin(); j != server->clients.end(); ++j)
					{
						if(!sameClient((*j).addr, src))
							bytesSent += sendto(server->sock, (void*)&notify, notify.size, 0, (sockaddr*)&((*j).addr), sizeof(sockaddr_in));
					}
					server->sock_mutex.unlock();
					if(bytesSent != 0)
						printf("Bytes sent: %lu\n", bytesSent);
					server->top_mutex.lock();
					continue;
					break;
				case TEXT:
					printf("Message Received: %s\n", (char*)buffer.data);
					msg_t pipe;
					pipe.type = TEXT;
					pipe.timestamp = ++server->lastTime;
					snprintf((char*)pipe.data, MTU - messageHeader, "%d.%d.%d.%d:%d says: %s",
						((unsigned char*)&src.sin_addr.s_addr)[0],
						((unsigned char*)&src.sin_addr.s_addr)[1],
						((unsigned char*)&src.sin_addr.s_addr)[2],
						((unsigned char*)&src.sin_addr.s_addr)[3],
						src.sin_port, (char*)buffer.data);
					pipe.size = messageHeader+strlen((char*)pipe.data)+1;
					server->sock_mutex.lock();
					for(std::vector<client_t>::iterator j = server->clients.begin(); j != server->clients.end(); ++j)
					{
						if(j != i)
							bytesSent += sendto(server->sock, (void*)&pipe, pipe.size, 0, (sockaddr*)&((*j).addr), sizeof(sockaddr_in));
					}
					server->sock_mutex.unlock();
					if(0 != bytesSent)
						printf("Bytes sent: %lu\n", bytesSent);
					break;
				case DISCONNECT_MSG:
					if(i != server->clients.end())
					{
						char addr_s[32];
						sprintaddr(src, addr_s, 32);
						printf("%s Disconnected\n", addr_s);
						server->clients.erase(i);
						msg_t notify;
						notify.type = TEXT;
						notify.timestamp = ++server->lastTime;
						snprintf((char*)notify.data, MTU - messageHeader, "%s disconnected", addr_s);
						notify.size = messageHeader + strlen((char*)notify.data) +1;
						for(std::vector<client_t>::iterator j = server->clients.begin(); j != server->clients.end(); ++j)
						{
							bytesSent += sendto(server->sock, (void*)&notify, notify.size, 0, (sockaddr*)&((*j).addr), sizeof(sockaddr_in));
						}
						server->sock_mutex.unlock();
						if(bytesSent != 0)
							printf("Bytes sent: %lu\n", bytesSent);
					}
					break;
				case PING:
					buffer.type = PING_RESPONSE;
					server->top_mutex.lock();
					buffer.timestamp = server->lastTime++;
					server->sock_mutex.lock();
					sendto(server->sock, (char*)&buffer, buffer.size, 0, (sockaddr*)&src, sizeof(src));
					server->sock_mutex.unlock();
					server->top_mutex.unlock();
					break;
				default:
					server->top_mutex.lock();
					if(server->messageHandler != NULL)
						server->messageHandler(buffer);
					server->top_mutex.unlock();
					break;
			}
		}
		else if((received < 0)&&(errno != EAGAIN))
		{
			perror("Recvfrom error");
			server->closing = true;
		}
		server->top_mutex.lock();
	}
	server->closed = true;
	server->listenThd = 0;
	server->top_mutex.unlock();
	return NULL;
}

void udpServerSock::setMessageHandler(void (*msgHandler)(msg_t message))
{
	top_mutex.lock();
	messageHandler = msgHandler;
	top_mutex.unlock();
}

udpServerSock::~udpServerSock()
{
	top_mutex.lock();
	closing = true;
	top_mutex.unlock();
}

udpClientSock::udpClientSock(const char *ip, uint16_t port, uint16_t localPort): 
lastTime(0),
port(port),
closing(false),
closed(false),
messageHandler(NULL)
{
	host.sin_family = AF_INET;
	host.sin_port = port;
	inet_pton(AF_INET, ip, (void*)&host.sin_addr);
	sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sock == INVALID_SOCKET)
		return;
	sockaddr_in me;
	me.sin_family = AF_INET;
	me.sin_port = localPort;
	me.sin_addr.s_addr = INADDR_ANY;
	if(SOCKET_ERROR == bind(sock, (sockaddr*)&me, sizeof(sockaddr_in)))
		return;
	printf("Socket Bound to port: %d\n", localPort);
	msg_t connectMessage;
	connectMessage.size = messageHeader;
	connectMessage.type = CONNECT_MSG;
	if(messageHeader == sendMessage(&connectMessage))
	{
		char hostAddr[32];
		sprintaddr(host, hostAddr, 32);
		printf("CONNECTING to %s\n", hostAddr);
		sockaddr_in src;
		socklen_t socklen = sizeof(sockaddr_in);
		bool connecting = true;
		int received = 0;
		timespec wait;
		wait.tv_sec = 1;
		wait.tv_nsec = 0;
		while(connecting)
		{
			nanosleep(&wait, &wait);
			received = recvfrom(sock, (void*)&connectMessage, sizeof(msg_t), MSG_DONTWAIT,
				(sockaddr*)&src, &socklen);
			if((received < 0) && (errno != EAGAIN))
				connecting = false;
			else if((received > 0)&&(connectMessage.type == ACK_MSG))
				connecting = false;
			else
				sendMessage(&connectMessage);
		}
		if(received > 0)
		{
			if(connectMessage.type == ACK_MSG)
				printf("CONNECTED\n");
		}
		else if(received < 0)
		{
			perror("Connection Error");
			closing = true;
		}
	}
	else
	{
		perror("Connection Error");
		closing = true;
	}
	listenThd = new std::thread(listen, (void*)this);
}

size_t udpClientSock::sendMessage(msg_t *message)
{
	if(message == NULL)
		return 0;
	if(message->size == 0)
		return 0;
	top_mutex.lock();
	message->timestamp = lastTime++;
	top_mutex.unlock();
	sock_mutex.lock();
	size_t bytes_sent = sendto(sock, (char*)message, message->size, 0, (sockaddr*)&host, 
		sizeof(host));
	sock_mutex.unlock();
	return bytes_sent;
}

void udpClientSock::sendDisconnectMsg()
{
	msg_t message;
	message.type = DISCONNECT_MSG;
	message.size = messageHeader;
	top_mutex.lock();
	message.timestamp = lastTime++;
	top_mutex.unlock();
	sock_mutex.lock();
	sendto(sock, (char*)&message, messageHeader, 0, (sockaddr*)&host, 
		sizeof(host));
	sock_mutex.unlock();
}

bool udpClientSock::isDisconnected()
{
	top_mutex.lock();
	bool out = closed;
	top_mutex.unlock();
	return out;
}

void *udpClientSock::listen(void *cli)
{
	udpClientSock *client = (udpClientSock*)cli;
	client->top_mutex.lock();
	sockaddr_in src;
	socklen_t socklen;
	msg_t buffer;
	while(!client->closing)
	{
		client->top_mutex.unlock();
		memset((void*)&buffer, 0, sizeof(msg_t));
		socklen = sizeof(sockaddr_in);
		client->sock_mutex.lock();
		int received = recvfrom(client->sock, (void*)&buffer, sizeof(msg_t), MSG_DONTWAIT,
			(sockaddr*)&src, &socklen);
		client->sock_mutex.unlock();
		if(received > 0)
		{
			switch(buffer.type)
			{
				case ACK_MSG:
					break;
				case DISCONNECT_MSG:
					printf("Server Shutting Down\n");
					client->closing = true;
					break;
				case PING:
					printf("PING\n");
					buffer.type = PING_RESPONSE;
					client->sock_mutex.lock();
					client->top_mutex.lock();
					buffer.timestamp = client->lastTime++;
					sendto(client->sock, (char*)&buffer, buffer.size, 0, (sockaddr*)&src, socklen);
					client->sock_mutex.unlock();
					continue;
					break;
				default:
					client->top_mutex.lock();
					if(client->messageHandler != NULL)
						client->messageHandler(buffer);
					continue;
					break;
			}
		}
		else if((received < 0)&&(errno != EAGAIN))
		{
			printf("Socket Error: %d, Terminating\n", errno);
			client->top_mutex.lock();
			client->closing = true;
			client->top_mutex.unlock();
		}
		client->top_mutex.lock();
	}
	client->closed = true;
	client->listenThd = 0;
	client->top_mutex.unlock();
	return NULL;
}

void udpClientSock::setMessageHandler(void (*msgHandler)(msg_t message))
{
	top_mutex.lock();
	messageHandler = msgHandler;
	top_mutex.unlock();
}

udpClientSock::~udpClientSock()
{
	top_mutex.lock();
	closing = true;
	while(listenThd != 0)
	{
		top_mutex.unlock();
		top_mutex.lock();
	}
	top_mutex.unlock();
}
