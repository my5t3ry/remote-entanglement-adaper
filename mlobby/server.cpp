#include <cstdlib>
#include <signal.h>
#include <time.h>
#include "UDPSockets.h"
#include "LobbyHelper.h"

bool quit;

void shutdown(int signal)
{
	printf("Exiting...\n");
	quit = true;
}

void handleMessage(msg_t message)
{
	switch(message.type)
	{
		default:
			printf("Default Message Handler\n");
			// Do Nothing
			break;
	}
}

int main(int argc, char *argv[])
{
	uint32_t lastClientDisconnected = 0xffffffff;
	bool emptyServer = true;
	quit = false;
	signal(SIGINT, shutdown);
	signal(SIGHUP, shutdown);
	udpServerSock sock(getpid());
	while(!quit)
	{
		uint32_t now = time(NULL);
		if((now > lastClientDisconnected + 60)&&(lastClientDisconnected != 0xffffffff))
		{
			quit = true;
			continue;
		}
		if(0 == sock.numClients() && !emptyServer)
		{
			emptyServer = true;
			lastClientDisconnected = (uint32_t)time(NULL);
		}
		else if(sock.numClients() != 0)
			emptyServer = false;

		quit |= sock.isClosed();
	}
	sock.sendDisconnectMsg();
	printf("Exiting\n");
	return EXIT_SUCCESS;
}
