#ifndef __LobbyHelper__H_
#define __LobbyHelper__H_

#include <stdint.h>
#include <stdio.h>

enum LobbyCommand
{
	REQ_LIST = 0xffff0000, REQ_NEW_LOBBY
};

/*typedef struct
{
	uint64_t timestamp;
	uint32_t type;
	uint32_t size;
	uint8_t data[MTU - messageHeader];
}msg_t;*/

struct request
{
	uint32_t type;
};

struct returnAddress 		// Message Body for all requests
{
	uint32_t sin_addr;		// IPv4 Address
	uint16_t sin_port;		// Port
};

struct listResponse			// Holds information about the listEntry
{
	// Data Header
	uint16_t sequenceID;	// ID of this server from 0 to numResponses
	uint16_t numResponses;	// Number of these responses that are being sent
	// Data Body
	uint32_t IP;			// IP Address
	uint16_t port;			// Port
};

struct newLobbyResponse
{
	uint32_t addr;
	uint16_t port;
};

inline size_t getLobbyList(returnAddress addr, returnAddress *&out)
{
	int sock = -1;
	out = nullptr;
	if(-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
	{
		perror("Socket Error");
		out = nullptr;
		return 0;
	}
	sockaddr_in host;
	host.sin_family = AF_INET;
	host.sin_port = addr.sin_port;
	host.sin_addr.s_addr = addr.sin_addr;
	socklen_t hostlen = sizeof(sockaddr_in);
	if(-1 == connect(sock, (sockaddr*)&host, hostlen))
	{
		perror("Connection Error");
		close(sock);
		return 0;
	}
	request R;
	R.type = REQ_LIST;
	int bytes = send(sock, (char*)&R, sizeof(request), 0);
	if(bytes < 0)
	{
		perror("Send error");
		close(sock);
		return 0;
	}
	listResponse response;
	bytes = recv(sock, (void*)&response, sizeof(listResponse), 0);
	while(bytes != sizeof(listResponse))
	{
		int r = recv(sock, (void*)((size_t)&response + bytes), sizeof(listResponse) - bytes, 0);
		if(r < 0)
		{
			perror("Recv Error");
			close(sock);
			return 0;
		}
		bytes += r;
	}
	if(response.numResponses == 0)
	{
		close(sock);
		return 0;
	}
	
	size_t num = response.numResponses;
	out = new returnAddress[num];

	for(size_t i = 0; i < num; ++i)
	{
		out[i].sin_addr = response.IP;
		out[i].sin_port = response.port;
		printf("%d\n", response.port);
		if(i+1 < num)
		{
			bytes = recv(sock, (void*)&response, sizeof(listResponse), 0);
			while(bytes != sizeof(listResponse))
			{
				int r = recv(sock, (void*)((size_t)&response + bytes), sizeof(listResponse) - bytes, 0);
				if(r < 0)
				{
					perror("Recv Error");
					close(sock);
					delete [] out;
					out = nullptr;
					return num;
				}
				bytes += r;
			}
		}
	}
	close(sock);
	return num;
}

inline void getNewLobby(returnAddress *addr)
{
	int sock = -1;
	if(-1 == (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
	{
		perror("Socket Error");
		return;
	}
	sockaddr_in host;
	host.sin_family = AF_INET;
	host.sin_port = addr->sin_port;
	host.sin_addr.s_addr = addr->sin_addr;
	socklen_t hostlen = sizeof(sockaddr_in);
	if(-1 == connect(sock, (sockaddr*)&host, hostlen))
	{
		perror("Connection Error");
		close(sock);
		return;
	}
	request R;
	R.type = REQ_NEW_LOBBY;
	int bytes = send(sock, (char*)&R, sizeof(request), 0);
	if(bytes < 0)
	{
		perror("Send error");
		close(sock);
		return;
	}
	newLobbyResponse response;
	bytes = recv(sock, (void*)&response, sizeof(newLobbyResponse), 0);
	while(bytes != sizeof(newLobbyResponse))
	{
		int i = recv(sock, (void*)((size_t)&response + bytes), sizeof(newLobbyResponse) - bytes, 0);
		if(i < 0)
		{
			perror("Recv Error");
			close(sock);
			return;
		}
	}
	printf("response.port: %d\n", response.port);
	addr->sin_port = response.port;
	close(sock);
}

#endif