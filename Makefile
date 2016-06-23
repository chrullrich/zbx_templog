CPPFLAGS = -It/zabbix/include
LDFLAGS = -lrt
CFLAGS = -shared -fPIC -std=gnu11

all: zbx_templog.so

zbx_templog.so: zbx_templog
	-mv $< $@


