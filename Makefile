CPPFLAGS = -I${ZBX_INCLUDE}
LDFLAGS = -lrt -shared
CFLAGS = -fPIC -std=gnu11 -Wall -Werror

ZBX_INCLUDE = ../zabbix-4.0.0/include
ZBX_CONFIG_H = ${ZBX_INCLUDE}/config.h

.ifdef DEBUG
CFLAGS += -g
.else
CFLAGS += -O2
.endif

all: zbx_templog.so

clean:
	-rm -f zbx_templog.o zbx_templog.so

${ZBX_CONFIG_H}: ${ZBX_CONFIG_H}.in
	cp $> $@

zbx_templog.so: zbx_templog.c ${ZBX_INCLUDE}/config.h
	${CC} ${CPPFLAGS} ${CFLAGS} ${LDFLAGS} -o $@ $<

