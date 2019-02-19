
include ../../Makefile.rules

OBJS-y := MngrModule.o
MODULE_DIRS := module1 module2 module3

CFLAGS += ${CFLAGS-y} ${COMMON_CFLAGS} -fPIC -pipe -I../../logger -I../../thread -I. -I../include -I../../include -Iinclude 

.PHONY: ${MODULE_DIRS}


${MODULE_DIRS}: $(OBJS-y)
	${Q}${MAKE} -C $@ all

all: $(OBJS-y) ${MODULE_DIRS}

install:
	@for D in $(MODULE_DIRS); do $(MAKE) -C $$D $@; done

clean: $(patsubst %.o,%.o.clean,$(OBJS-y))
	@for D in $(MODULE_DIRS); do $(MAKE) -C $$D $@; done

