TARGET=Fuzzer

CLIENTEXE=FuzzClient
SERVEREXE=FuzzServer

INC=-I../src/
LIB=../src/libmodbus.la
EXTCFLAGS= -Wall -Wextra
LIBTOOL=libtool --mode=link --tag=CXX

all: $(TARGET)

CPFILE:
	cp ../tests/unit-test.h .

$(TARGET): CPFILE
	$(CC) $(EXTCFLAGS) $(CFLAGS) $(INC) -c $(CLIENTEXE).c
	$(CC) $(EXTCFLAGS) $(CFLAGS) $(INC) -c $(SERVEREXE).c

	$(LIBTOOL) $(CXX) $(LDFLAGS) $(CFLAGS) $(LIB) $(LIB_FUZZING_ENGINE) $(CLIENTEXE).o -o $(CLIENTEXE)
	$(LIBTOOL) $(CXX) $(LDFLAGS) $(CFLAGS) $(LIB) $(LIB_FUZZING_ENGINE) $(SERVEREXE).o -o $(SERVEREXE)

clean:
	rm $(CLIENTEXE) $(SERVEREXE) *.o

.PHONY: all clean
