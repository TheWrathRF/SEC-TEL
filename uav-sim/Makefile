CC = gcc
CFLAGS = -Wall -g
LDLIBS = -lws2_32

uav-sim.exe: main.c
	$(CC) $(CFLAGS) main.c -o uav-sim.exe $(LDLIBS)

clean:
	rm -f uav-sim.exe
