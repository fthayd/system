
include ../../../Makefile.rules

OBJS-y := module2.o ../MngrModule.o
BIN := libMngrmodule2.so

CFLAGS += ${CFLAGS-y} ${COMMON_CFLAGS} -fPIC -pipe -I. -I../include -I../../include -I../../../include -I../../../logger -I../../../thread
LDFLAGS += ${LDFLAGS-y} -L../../../logger -llogger


all: $(BIN)

$(BIN): $(OBJS-y)
	$(do_so)

install: $(BIN) $(INSTALL_DIR)/lib
	$(do_strip)
	$(do_install_upx)

clean: $(BIN).clean $(patsubst %.o,%.o.clean,$(OBJS-y))
