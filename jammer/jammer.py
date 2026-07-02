import socket
import random
import sys
import time

TARGET_HOST = "127.0.0.1"
DEFAULT_PORT = 5000       # the jammer attacks one hardcoded port, it does not hop
PACKET_LEN = 52


def sniff(port):
    # bind the target port and print any UDP packets that show up
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.bind((TARGET_HOST, port))
    print("sniffing udp port %d ..." % port)
    while True:
        data, addr = s.recvfrom(2048)
        print("intercepted %d bytes from %s:%d  %s" %
              (len(data), addr[0], addr[1], data[:16].hex()))


def grab_template(port):
    # try to intercept one real packet to corrupt, fall back to random bytes
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        s.bind((TARGET_HOST, port))
    except OSError:
        s.close()
        return None
    s.settimeout(3)
    try:
        data, _ = s.recvfrom(2048)
        return bytearray(data)
    except socket.timeout:
        return None
    finally:
        s.close()


def bitflip(port):
    # flip random bytes in a packet and send the corrupted copy to the port
    base = grab_template(port)
    if base:
        print("captured a %d-byte packet, replaying corrupted copies" % len(base))
    else:
        print("no live packet, using a random template")
        base = bytearray(random.getrandbits(8) for _ in range(PACKET_LEN))

    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print("jamming port %d (bit-flip) ..." % port)
    while True:
        pkt = bytearray(base)
        for _ in range(4):
            i = random.randrange(len(pkt))
            pkt[i] ^= 1 << random.randrange(8)
        s.sendto(pkt, (TARGET_HOST, port))
        time.sleep(0.2)


def noise(port):
    # flood the target port with completely random garbage bytes
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    print("flooding port %d with noise ..." % port)
    while True:
        n = random.randint(20, 80)
        junk = bytes(random.getrandbits(8) for _ in range(n))
        s.sendto(junk, (TARGET_HOST, port))
        time.sleep(0.1)


if __name__ == "__main__":
    mode = sys.argv[1] if len(sys.argv) > 1 else "sniff"
    port = int(sys.argv[2]) if len(sys.argv) > 2 else DEFAULT_PORT
    if mode == "sniff":
        sniff(port)
    elif mode == "bitflip":
        bitflip(port)
    elif mode == "noise":
        noise(port)
    else:
        print("usage: jammer.py [sniff|bitflip|noise] [port]")
