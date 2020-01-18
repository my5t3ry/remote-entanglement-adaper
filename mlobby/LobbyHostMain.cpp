#include <cstdlib>
#include <signal.h>
#include <time.h>
#include "LobbyHost.h"

#define DEFAULT_PORT 16567

int main(int argc, char *argv[])
{
	uint16_t port = DEFAULT_PORT;
	if(argc > 1)
	{
		port = (uint16_t)atoi(argv[1]);
	}
	LobbyHost host;
	host.run(port);
	exit(EXIT_SUCCESS);
}