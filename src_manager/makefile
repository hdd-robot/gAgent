CC = g++
CFLAGS = -Wall
EXEC_NAME = gAgent
INCLUDES =
LIBS = -lgagentlib -lboost_program_options -lboost_system -lboost_filesystem -lconfig++ 

OBJ_FILES = main.o Args.o AgentPlatform.o AgentManager.o 

all : $(EXEC_NAME)

clean :
	rm -f *~
	rm -f $(EXEC_NAME) $(OBJ_FILES)
	rm -f *.o
	rm -f *.gch
	rm -f ./agentmanager
	rm -f ./agentplatform
	rm -f ./agentmonitor

$(EXEC_NAME) : $(OBJ_FILES)
	$(CC) -o $(EXEC_NAME) $(OBJ_FILES)  -I../src_agent  -L../src_agent $(LIBS)
	ln -sf gAgent agentmanager
	ln -sf gAgent agentplatform
	ln -sf gAgent agentmonitor

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -std=c++11 -o $@ -c $<

run :
	./gAgent

