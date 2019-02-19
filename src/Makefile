include ./Makefile.rules

DIRS := logger thread timer manager

.PHONY: ${DIRS}

export INSTALL_DIR
export CFLAGS-y += -O2

JOBS := -j

%.build: %
	${Q}${MAKE} -C $< all ${JOBS}

platform.build: platform
	${Q}${MAKE} -C $< all j=1

%.install: %
	${Q}${MAKE} -C $< install ${JOBS}

%.cleandir: %
	${Q}${MAKE} -C $< clean ${JOBS}

all: $(addsuffix .build,$(DIRS))

install: $(addsuffix .install,$(DIRS))

clean: $(addsuffix .cleandir,$(DIRS))

# dependencies
timer.build: logger.build thread.build
manager.build: timer.build logger.build thread.build
