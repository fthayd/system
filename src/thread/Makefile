include ../Makefile.rules

OBJS-y := thread.o
BIN := libthread.so

CFLAGS += ${CFLAGS-y} ${COMMON_CFLAGS} -fPIC -pipe -I.
LDFLAGS += ${LDFLAGS-y} -lpthread

all: $(BIN)

$(BIN): $(OBJS-y)
	$(do_so)

install: $(BIN) $(INSTALL_DIR)/lib
	$(do_strip)
	$(do_install_upx)

clean: $(BIN).clean $(patsubst %.o,%.o.clean,$(OBJS-y))
