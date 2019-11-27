CFLAGS += -Wall -Wextra
CFLAGS += -ggdb

all: test

test: main.o tagged_storage.o
	$(CC) $(CFLAGS) -o $@ $(LDFLAGS) $^ $(LDLIBS)

clean:
	$(RM) test *.o
