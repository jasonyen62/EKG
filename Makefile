# Make file for Test
all: MAIN
CC = g++
DRIVER_PATH = IPMS_Driver
SQLLIB = /usr/local/lib
SQLINC = /usr/local/include

CFLAGS = -O2 -Wall -march=i486
LIBS   = -lpthread -lSDL -ldl -lsqlite3
#ALLEGRO = -L/usr/local/lib -L/usr/X11R6/lib -Wl,--export-dynamic -lalleg
MAIN: rs232.o MF700.o HF320.o CRT350.o Eltra1000.o ServerTalk.o Voice.o Datafile.o Display.o Reader.o test.o Network.o IC8255.o Dio.o ra8822.o D1000.o D3000.o traceLog.o LED888.o BarCode.o Tup500.o PassingIniFile.o MCP210.o main.o
	$(CC) -O3 -lstdc++ $(LIBS) -march=i486 -o main rs232.o MF700.o HF320.o CRT350.o Eltra1000.o  Dio.o Network.o ServerTalk.o Voice.o Datafile.o Display.o Reader.o test.o IC8255.o ra8822.o Tup500.o D1000.o D3000.o traceLog.o LED888.o BarCode.o PassingIniFile.o MCP210.o main.o
#	$(CC) -O3 -g -lstdc++ -lpthread -lsqlite3 -ldl -march=i486 -o main main.o rs232.o MF700.o CRT350.o Eltra1000.o Dio.o ServerTalk.o Voice.o Datafile.o Display.o Reader.o test.o Network.o IC8255.o   #$(SQLLIB)/libsqlite3.a
Network.o: $(DRIVER_PATH)/Network.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/Network.cpp
rs232.o: $(DRIVER_PATH)/rs232.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/rs232.cpp
MF700.o: $(DRIVER_PATH)/MF700.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/MF700.cpp
HF320.o: $(DRIVER_PATH)/HF320.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/HF320.cpp
D1000.o: $(DRIVER_PATH)/D1000.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/D1000.cpp
D3000.o: $(DRIVER_PATH)/D3000.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/D3000.cpp
CRT350.o: $(DRIVER_PATH)/CRT350.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/CRT350.cpp
Tup500.o: $(DRIVER_PATH)/Tup500.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/Tup500.cpp
Eltra1000.o: $(DRIVER_PATH)/Eltra1000.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/Eltra1000.cpp
IC8255.o: $(DRIVER_PATH)/IC8255.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/IC8255.cpp
LED888.o: $(DRIVER_PATH)/LED888.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/LED888.cpp
ra8822.o: $(DRIVER_PATH)/ra8822.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/ra8822.cpp
BarCode.o: $(DRIVER_PATH)/BarCode.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/BarCode.cpp
MCP210.o: $(DRIVER_PATH)/MCP210.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c $(DRIVER_PATH)/MCP210.cpp
Dio.o: Dio.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c Dio.cpp
ServerTalk.o: ServerTalk.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -lstdc++ -c ServerTalk.cpp
Voice.o: Voice.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c Voice.cpp
Datafile.o: Datafile.cpp
	$(CC) -I$(DRIVER_PATH) -I$(SQLINC) $(CFLAGS) -c Datafile.cpp
Display.o: Display.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c Display.cpp
Reader.o: Reader.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c Reader.cpp
test.o: test.cpp
	$(CC) -I$(DRIVER_PATH) -I$(SQLINC) $(CFLAGS) $(LIBS) -c test.cpp
traceLog.o: traceLog.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) -c traceLog.cpp
PassingIniFile.o: PassingIniFile.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) $(LIBS) -c PassingIniFile.cpp
main.o: main.cpp
	$(CC) -I$(DRIVER_PATH) $(CFLAGS) $(LIBS) -c main.cpp
clean: 
	-rm main.o ra8822.o Tup500.o BarCode.o Network.o Voice.o Display.o Reader.o rs232.o MF700.o HF320.o CRT350.o Eltra1000.o Dio.o ServerTalk.o Datafile.o test.o IC8255.o D1000.o D3000.o LED888.o traceLog.o PassingIniFile.o MCP210.o  main
