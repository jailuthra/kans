CC=gcc
CFLAGS=-O2

all: compress decompress

compress: compress.o kans.o
	$(CC) $(CFLAGS) kans.o compress.o -o compress

decompress: decompress.o kans.o
	$(CC) $(CFLAGS) kans.o decompress.o -o decompress

kans.o: kans.h

%.s: %.c
	$(CC) $(CFLAGS) -S -o $@ $<

lib%.so: %.c
	$(CC) $(CFLAGS) -shared -o $@ -fPIC $<

clean:
	rm -rf *.o
	rm -f compress
	rm -f decompress
