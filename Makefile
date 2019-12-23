CPPFLAGS = -I${ZBX_INCLUDE}
LDFLAGS = -lrt -rpath /usr/local/lib -shared
CFLAGS = -fPIC -std=gnu11 -Wall -Werror

ZBX_INCLUDE = ../zabbix-4.0.0/include
ZBX_CONFIG_H = ${ZBX_INCLUDE}/config.h

.ifdef DEBUG
CFLAGS += -g
.else
CFLAGS += -O2
.endif

all: libzbx_templog.so

clean:
	-rm -f zbx_templog.o libzbx_templog.so

${ZBX_CONFIG_H}: ${ZBX_CONFIG_H}.in
	cp $> $@

libzbx_templog.so: zbx_templog.c ${ZBX_INCLUDE}/config.h
	${CC} ${CPPFLAGS} ${CFLAGS} -o zbx_templog.o -c zbx_templog.c
	${CC} ${LDFLAGS} -o libzbx_templog.so zbx_templog.o -lrt

