CXX = g++
FLAGS = -g -Wall -O1 -std=c++11
CFLAGS = $(FLAGS) -c

OBJS = UDPSockets.o LobbyHost.o
LIBS = -lpthread

target: server client host

host: $(OBJS) LobbyHost.cpp makefile
	$(CXX) $(FLAGS) $(OBJS) LobbyHostMain.cpp -o LobbyHost $(LIBS)

client: $(OBJS) client.cpp makefile
	$(CXX) $(FLAGS) $(OBJS) client.cpp -o client $(LIBS)

server: $(OBJS) server.cpp makefile
	$(CXX) $(FLAGS) $(OBJS) server.cpp -o server $(LIBS)

%.o : %.cpp UDPSockets.h socketHelper.h makefile
	 $(CXX) $(CFLAGS) $(FLAGS) $< -o $@

clean:
	rm -f $(OBJS) server client LobbyHost
