#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "LobbyHelper.h"
#include "LobbyHost.h"
#include <cstdio>
#include <set>
#include <thread>

LobbyHost::LobbyHost():
listenSocket(-1),
myAddress(LOCALHOST)
{
}

void waitForServerClose(LobbyHost* host, pid_t pid)
{
	int status = 0;

	waitpid(pid, &status, 0);		// Wait for the process to end for the thread to continue

	printf("Server closed, removing reference\n");
	host->removeServerRef(pid);
}

void LobbyHost::removeServerRef(const pid_t &pid)
{
	m_mutex.lock();
	servers.erase(pid);
	m_mutex.unlock();
}

void LobbyHost::run(const uint16_t &port)
{
	if(-1 == (listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)))
		return;
	sockaddr_in listenAddr;
	listenAddr.sin_family = AF_INET;
	listenAddr.sin_port = port;
	listenAddr.sin_addr.s_addr = INADDR_ANY;
	if(-1 == bind(listenSocket, (sockaddr*)&listenAddr, sizeof(sockaddr_in)))
		return;
	myAddress = (uint32_t)listenAddr.sin_addr.s_addr;
	printf("My address is: %hhu.%hhu.%hhu.%hhu\n", ((uint8_t*)&myAddress)[0],
		((uint8_t*)&myAddress)[1],((uint8_t*)&myAddress)[2],((uint8_t*)&myAddress)[3]);
	if(-1 == listen(listenSocket, SOMAXCONN))
		return;
	for(;;)
	{
		sockaddr_in cli_addr;
		socklen_t socketLength = sizeof(sockaddr_in);
		int client = accept(listenSocket, (sockaddr*)&cli_addr, &socketLength);
		request R;														// A request struct to hold the user's request
		int bytes = recv(client, (void*)&R, sizeof(request), 0);		// Begin receiving the user's request
		if(bytes < 0)
		{
			printf("Recv Error\n");
			close(client);
			exit(EXIT_SUCCESS);
		}
		while(bytes != sizeof(request))									// Make sure we got all of it before continuing
		{
			int r = recv(client, (void*)((size_t)&R + bytes), sizeof(request)-bytes, 0);
			if(r < 0)
			{
				printf("Recv Error\n");
				close(client);
				exit(EXIT_SUCCESS);
			}
			else
				bytes += r;
		}
		// Full request received. Send response;
		if(R.type == REQ_NEW_LOBBY)
		{
			pid_t pid = fork();
			if(pid < 0)
			{
				perror("Fork error");
				close(client);
				continue;
			}
			else if(pid == 0)
			{
				pid = getpid();
				newLobbyResponse response;	// Send the address and port of the server as the response
				response.addr = 0x0100007F;	// Localhost
				response.port = (uint16_t)pid;		// pid is port
				send(client, (char*)&response, sizeof(newLobbyResponse), 0);
				close(client);
				execl("server", "", NULL);	// Start up the server
			}
			else
			{
				m_mutex.lock();
				servers.insert(pid);
				m_mutex.unlock();
				new std::thread(waitForServerClose, this, pid);
			}
		}
		else if(R.type == REQ_LIST)
		{
			listResponse response;
			response.IP = myAddress;
			response.numResponses = (uint16_t)servers.size();
			if(response.numResponses != 0)
			{
				size_t i = 0;
				for(std::set<pid_t>::iterator it = servers.begin(); it != servers.end(); ++i, ++it)
				{
					response.sequenceID = (uint16_t)i+1;
					response.port = (*it);
					send(client, (char*)&response, sizeof(listResponse), 0);
				}
			}
			else
			{
				response.sequenceID = 0;
				send(client, (char*)&response, sizeof(listResponse), 0);
			}
		}
		else
		{
			printf("Unknown Request\n");
		}
		close(client);
	}
	close(listenSocket);
	listenSocket = -1;
}

LobbyHost::~LobbyHost()
{
	if(listenSocket != -1)
		close(listenSocket);
}
