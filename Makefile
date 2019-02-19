include src/Makefile.rules

TOOLCHAIN_BASE_PATH := /usr/local/arm/gcc-4.7-linaro-rpi-gnueabihf/arm-linux-gnueabihf
TOOLCHAIN_CCACHE_PATH := ${TOOLCHAIN_BASE_PATH}/ccache -C
TOOLCHAIN_PATH := ${TOOLCHAIN_BASE_PATH}/bin
QT_PATH := /usr/local/arm/qt4.8.5
INSTALL_DIR ?= /nfs
CROSS_PREFIX ?=arm-linux-

export LD	:= ${CROSS_PREFIX}ld
export AR	:= ${CROSS_PREFIX}ar
export RANLIB	:= ${CROSS_PREFIX}ranlib
export CC	:= ${CROSS_PREFIX}gcc
export CPP	:= ${CROSS_PREFIX}c++
export STRIP	:= ${CROSS_PREFIX}strip
export UPX_BIN	:= upx
ifeq ($(CROSS_PREFIX),)
export QMAKE	:= qmake
else
export QMAKE	:= ${QT_PATH}/bin/qmake
endif

export PATH := ${PWD}/tools:${TOOLCHAIN_CCACHE_PATH}:${TOOLCHAIN_PATH}:${PATH}
export CFLAGS-y := -Wall -Werror
export QT_PATH CROSS_PREFIX
export TOOLCHAIN_LIB_DIR := ${TOOLCHAIN_BASE_PATH}

BIN := logger thread timer manager

.PHONY: ${BIN} install clean

JOBS := -j

all: clean install package

${BIN}:
	$(Q)$(MAKE) -C src $@.build

install:
	$(Q)mkdir -p ${INSTALL_DIR}/lib
	$(Q)$(MAKE) -C src install ${JOBS} INSTALL_DIR=${INSTALL_DIR}

package:
	$(Q)mkdir -p $(shell pwd)/_install/lib
	$(Q)${MAKE} install INSTALL_DIR=$(shell pwd)/_install
	$(Q)${MAKE} -C dist clean all

clean:
	$(Q)$(MAKE) -C src clean ${JOBS}
	$(Q)$(MAKE) -C dist clean ${JOBS}
	$(Q)rm -rf $(shell pwd)/_install

run:
	LD_LIBRARY_PATH=_install/lib _install/appName ${LOGLEVEL}
