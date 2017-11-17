TARGET = sparsetest
CC = gcc
CFLAGS = -O3 -Wall -Werror -std=c99

.PHONY: default all clean install uninstall

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

install: $(TARGET)
	mkdir -p "$(DESTDIR)$(PREFIX)/bin"
	cp "$(TARGET)" "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"

uninstall:
	-rm -f "$(DESTDIR)$(PREFIX)/bin/$(TARGET)"