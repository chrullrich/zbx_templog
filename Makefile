CPPFLAGS = -It/zabbix/include
LDFLAGS = -lrt -shared
CFLAGS = -fPIC -std=gnu11 -Wall -Werror

ifdef DEBUG
CFLAGS += -g
else
CFLAGS += -O2
endif

SOURCES = zbx_templog.c
OBJECTS = $(SOURCES:.c=.o)

TARGET = zbx_templog.so

all: $(TARGET)

clean:
	-rm -f $(OBJECTS) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

