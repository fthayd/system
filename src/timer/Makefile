include ../Makefile.rules

OBJS-y := timer.o
BIN := libtimer.so

CFLAGS += ${CFLAGS-y} ${COMMON_CFLAGS} -fPIC -pipe -I. -I../logger -I../thread
LDFLAGS += ${LDFLAGS-y} -lpthread -L../logger -llogger -L../thread -lthread


all: $(BIN)

$(BIN): $(OBJS-y)
	$(do_so)

install: $(BIN) $(INSTALL_DIR)/lib
	$(do_strip)
	$(do_install_upx)

clean: $(BIN).clean $(patsubst %.o,%.o.clean,$(OBJS-y))
