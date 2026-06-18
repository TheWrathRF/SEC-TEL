CC = gcc
CFLAGS = -Wall -g
LDLIBS = -lws2_32
SRC = main.c packet.c crc32.c aes.c

uav-sim.exe: $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o uav-sim.exe $(LDLIBS)

clean:
	rm -f uav-sim.exe
