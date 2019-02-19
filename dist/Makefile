include ../src/Makefile.rules

MAJOR=$(shell sed -n 's/.*PROGRAM_VERSIYON_MAJOR.*\"\(.*\)\"/\1/p' ../src/include/common.h | head -n1)
MINOR=$(shell sed -n 's/.*PROGRAM_VERSIYON_MINOR.*\"\(.*\)\"/\1/p' ../src/include/common.h | head -n1)
BUILD=$(shell sed -n 's/.*PROGRAM_VERSIYON_BUILD.*\"\(.*\)\"/\1/p' ../src/include/common.h | head -n1)
APP_NAME=$(shell sed -n 's/.*APP_NAME.*\"\(.*\)\"/\1/p' ../src/include/common.h | head -n1)
VERSION=${MAJOR}.${MINOR}.${BUILD}
LZMA_NAME=${APP_NAME}"_"${VERSION}

.PHONY: init package

Q ?= @
DATE := '1970-01-01 02:00:00'

all: package

init:
	${Q}mkdir -p files
	${Q}cp -R ../_install/* files

package: init
	${Q}echo -n "Packing: ${LZMA_NAME}.tar.lzma "
	${Q}(tar -cvf ${LZMA_NAME}.tar files/ ; rm -R ${LZMA_NAME}.tar.lzma ; lzma ${LZMA_NAME}.tar ) 1>/dev/null 2>&1
	${Q}echo -e "\tFile Size:" `stat -c '%s' ${LZMA_NAME}.tar.lzma` " OK!"

clean:
	${Q}rm -rf files/*
	${Q}rm -rf *.lzma
