CC=gcc
CFLAGS=-Wall -Wextra -pedantic -Wno-pointer-arith -std=c17
LDFLAGS=
OBJFILES=$(wildcard src/*.c)
TARGET=pageboy

DEST=/usr/local/bin

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

debug: CFLAGS += -D debug
debug: $(TARGET)

clean:
	rm $(TARGET)

install: $(TARGET)
	install -m 0777 $(TARGET) $(DEST)/$(TARGET)

run: $(TARGET)
	./pageboy test.db

test: $(TARGET)
	shpec t/*_shpec.bash

test_watch: $(TARGET)
	ls t/*_shpec.bash | entr shpec

# Install test dependencies
install_test_deps:
	./scripts/install_test_deps.bash
