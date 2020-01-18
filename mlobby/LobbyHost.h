#ifndef __LOBBYHOST_H__
#define __LOBBYHOST_H__

#include <cstdlib>
#include <set>
#include <stdint.h>
#include <mutex>
#include "socketHelper.h"

class LobbyHost
{
private:
	/// Stores the PIDs of all of the currently running servers
	std::set<pid_t> servers;

	/// TCP Listening Socket for listening to new connections
	int listenSocket;

	// my IPv4 Address (initialized when run is called)
	uint32_t myAddress;

	// mutex to handle asynchronous access to the server reference set
	std::mutex m_mutex;
public:
	/// Constructor
	LobbyHost();

	/// Destructor
	~LobbyHost();

	// This function removes the server reference from the set
	void removeServerRef(const pid_t &pid);

	///	This method runs the entire program
	void run(const uint16_t &port);
};

#endif