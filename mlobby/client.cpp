#include <cstdlib>
#include <signal.h>
#include <time.h>
#include "UDPSockets.h"
#include "LobbyHelper.h"

bool quit;

void shutdown(int signal)
{
	printf("Exiting...\n");
	exit(EXIT_SUCCESS);
}

void handleMessage(msg_t message)
{
	switch(message.type)
	{
		case TEXT:
			if(message.data[strlen((char*)message.data)-1] == '\n')
				printf("%s", (char*)message.data);
			else
				printf("%s\n", (char*)message.data);
			fflush(stdout);
			break;
		default:
			printf("Unknown Message Received\n");
			// Do Nothing
			break;
	}
}

void getServer(returnAddress *addr);

int main(int argc, char *argv[])
{
	quit = false;
	signal(SIGINT, shutdown);
	signal(SIGHUP, shutdown);

	returnAddress addr;
	addr.sin_addr = LOCALHOST;
	addr.sin_port = DEFAULT_PORT;

	if(argc > 1)
	{
		if(5 != sscanf(argv[1],
			"%hhu.%hhu.%hhu.%hhu:%hu", 
			&((uint8_t*)&addr.sin_addr)[0], 
			&((uint8_t*)&addr.sin_addr)[1], 
			&((uint8_t*)&addr.sin_addr)[2], 
			&((uint8_t*)&addr.sin_addr)[3], 
			&addr.sin_port))
		{
			printf("Invalid argument: %s\n", argv[1]);
			exit(EXIT_SUCCESS);
		}
	}

	getServer(&addr);
	char addrStr[16];
	sprintaddr(addr.sin_addr, addrStr, 16);
	printf("host port: %d\n", addr.sin_port);
	udpClientSock sock(addrStr, addr.sin_port, getpid());

	sock.setMessageHandler(handleMessage);
	char buffer[256];
	while(!quit)
	{
		fgets(buffer, 256, stdin);
		if(buffer[0] == '/')	// If a command
		{
			if(0 == strcmp("quit\n", &buffer[1]))
			{
				sock.sendDisconnectMsg();
				quit = true;
			}
		}
		else
		{
			msg_t message;
			strncpy((char*)message.data, buffer, MTU - messageHeader);
			message.type = TEXT;
			message.size = messageHeader + strlen(buffer) + 1;
			sock.sendMessage(&message);
		}
		quit |= sock.isDisconnected();
	}

	return EXIT_SUCCESS;
}

void getServer(returnAddress *addr)
{
	printf("TextChatCli(ent)\n");
	int com = 0;
	do
	{
		printf("1> Print list of Lobbies\n");
		printf("2> Join Lobby\n");
		printf("3> Create New Lobby\n");
		char buffer[8];
		fgets(buffer, 8, stdin);
		fflush(stdin);
		com = atoi(buffer);
		returnAddress *list;
		size_t num = 0;
		switch(com)
		{
			case 1:
				if(0 == (num = getLobbyList(*addr, list)))
				{
					printf("\nNo servers available\n\n");
					break;
				}
				printf("%u servers\n", (uint32_t)num);
				printf("#\n\tIP\tport\n");
				for(size_t i = 0; i < num; ++i)
				{
					printf("%u\t%hhu.%hhu.%hhu.%hhu\t%hu\n", 
						(uint32_t)i, ((uint8_t*)&list[i])[0], 
						((uint8_t*)&list[i].sin_addr)[1], ((uint8_t*)&list[i].sin_addr)[2], 
						((uint8_t*)&list[i].sin_addr)[3], list[i].sin_port);
				}
				printf("\n");
				delete [] list;
				break;
			case 2:
				printf("What port? ");
				fgets(buffer, 8, stdin);
				addr->sin_port = (uint16_t)atoi(buffer);
				break;
			case 3:
				getNewLobby(addr);
				break;
			default:
				printf("Invalid Selection\n\n");
				break;
		}
	}while((com != 2)&&(com != 3));
}