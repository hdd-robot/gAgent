CC = g++
CFLAGS = -Wall -D_REENTRANT
LIB_NAME = libgagentlib.so 
INCLUDES = 
LIBS =    -pthread -lboost_system
OBJ_FILES = Behaviour.o Agent.o AgentCore.o udp_client_server.o AgentID.o ACLMessage.o DFService.o DFAgentDescription.o 

all : $(LIB_NAME)
	
clean :
	rm -f *~
	rm -f $(LIB_NAME) $(OBJ_FILES)
	rm -f *.o
	rm -f *.gch
	rm -f *.so*

$(LIB_NAME) : $(OBJ_FILES)
	$(CC) -std=c++11  -shared -fPIC -o $(LIB_NAME) $(OBJ_FILES) $(LIBS)
	
%.o: %.cpp
	$(CC) $(CFLAGS) -std=c++11  $(INCLUDES) -fPIC -c $<




