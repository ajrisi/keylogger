CC=gcc
CFLAGS=-s -O3
TARGET=keylogger

all:
	$(CC) $(CFLAGS) -o $(TARGET) keylogger.c

install:
	install -m 755 $(TARGET) /usr/sbin/
	install -m 644 keylogger.8 /usr/share/man/man8/

clean:
	rm keylogger
