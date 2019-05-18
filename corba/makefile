# Makefile

OMNIORB_HOME=/usr

IDL=$(OMNIORB_HOME)/bin/omniidl
IDLFLAGS=-bcxx

INCLUDES=-I$(OMNIORB_HOME)/include -I$(OMNIORB_HOME)/include/omniORB4

LIBS=-L$(OMNIORB_HOME)/lib -lomnithread -lomniORB4 -lomniDynamic4

IDL_FILE = ./fipa.idl

.PHONY: all
all: server client

server: server.o FipaInterface_impl.o fipaSK.o
	$(CXX) -o $@ $^ $(LIBS)

server.o: server.cpp FipaInterface_impl.h

FipaInterface_impl.h: fipa.hh

client: client.o fipaSK.o
	$(CXX) -o $@ $^ $(LIBS)

FipaInterface_impl.o: FipaInterface_impl.cpp FipaInterface_impl.h fipa.hh

fipaSK.o: fipaSK.cc fipa.hh

fipaSK.cc fipa.hh: $(IDL_FILE)
	$(IDL) $(IDLFLAGS) $<

.PHONY: clean
clean:
	find . -maxdepth 1 -type f -name "*.bak" -exec rm -f {} \;
	find . -maxdepth 1 -type f -name "*.o" -exec rm -f {} \;
	find . -maxdepth 1 -type f -name "*.stackdump" -exec rm -f {} \;
	find . -maxdepth 1 -type f -name "*.exe" -exec rm -f {} \;
	find . -maxdepth 1 -type f -name "fipa.hh" -exec rm -f {} \;
	find . -maxdepth 1 -type f -name "fipaSK.cc" -exec rm -f {} \;

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(INCLUDES)

.cc.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $< $(INCLUDES)

