include ../Makefile.rules

OBJS-y :=main.o Mngr.o MngrCmdHandler.o MngrEvent.o MngrEventList.o Config.o Utility.o
BIN := appName
MODULES_DIR := modules

LDFLAGS += -Lmodules/module1 -lMngrmodule1
LDFLAGS += -Lmodules/module2 -lMngrmodule2
LDFLAGS += -Lmodules/module3 -lMngrmodule3

CFLAGS += ${CFLAGS-y} ${COMMON_CFLAGS} -I../logger -I../thread -I../timer -I./include -I../include -Imodules/include
LDFLAGS += ${LDFLAGS-y} -L../logger -llogger -L../thread -lthread -L../timer -ltimer -L${TOOLCHAIN_LIB_DIR}

ifneq (${CROSS_PREFIX},)
CFLAGS += -I${QT_PATH}/include
LDFLAGS += -L${QT_PATH}/lib
endif


all: $(BIN)

$(BIN): $(OBJS-y)
	${Q}${MAKE} -C ${MODULES_DIR} all
	$(do_ld)

install: ${BIN} ${INSTALL_DIR}
	$(do_strip)
	$(do_install_upx)
	${Q}${MAKE} -C ${MODULES_DIR} $@

clean: $(BIN).clean $(patsubst %.o,%.o.clean,$(OBJS-y))
	${Q}${MAKE} -C ${MODULES_DIR} $@
