all: load shared load2

load: load.cpp
	g++ load.cpp -o load -ldl -lrt -lcurl

load2: load2.cpp
	g++ load2.cpp -o load2 -ldl -lrt

shared:	shared.c
	gcc shared.c -shared -fPIC -o shared.so
	xxd -i shared.so > sharedbin.h

.PHONY clean:
	rm -f load load2 shared.so sharedbin.h
