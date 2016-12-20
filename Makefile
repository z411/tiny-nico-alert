SOURCES=core.c net.c http.c nico.c main.c
CFLAGS=-D_GNU_SOURCE
LDFLAGS=-ltls

OBJECTS=$(SOURCES:.c=.o)
OUT=nico

$(OUT): $(OBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

clean:
	rm *.o $(OUT)
