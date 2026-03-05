CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -flto
LDFLAGS ?= -flto

TARGET = rg35xxh-controller
SOURCES = main.c controller.c input_reader.c usb_gadget.c bluetooth_hid.c
OBJECTS = $(SOURCES:.c=.o)
DEPS = controller.h input_reader.h usb_gadget.h bluetooth_hid.h

LIBS = -levdev -lbluetooth -lusb-1.0 -lasound -lpthread -lrt

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS) $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJECTS) $(TARGET)

install: $(TARGET)
	install -m 755 $(TARGET) $(DESTDIR)/usr/bin/

.PHONY: all clean install