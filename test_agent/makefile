CC = g++ -std=c++11
CFLAGS = -Wall  
EXEC_NAME = testAgent
INCLUDES =
LIBS = -lgagent  -pthread -lboost_system  -lconfig++ -lrt -lQt5Widgets -lQt5Gui -lQt5Core -lGL
OBJ_FILES = main.o
INSTALL_DIR = /usr/bin

all : $(EXEC_NAME)

clean :
	rm -f *~
	rm -f $(EXEC_NAME) $(OBJ_FILES)
	rm -f *.o
	rm -f *.gch


$(EXEC_NAME) : $(OBJ_FILES)
	$(CC) -o $(EXEC_NAME)  $(OBJ_FILES)  -I../src_agent  -L../src_agent $(LIBS) 


%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

run :
	./testAgent

