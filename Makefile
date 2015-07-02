################################################################################
## Copyright (c) Jonathan(Jon) DuBois 2015. This file is part of LNZ.         ##
################################################################################


.PHONY: all
all: depend debug



# Toolchain.
CC=gcc
CCFLAGS=-std=c11 -Wall -fno-exceptions -pedantic -Wextra -Werror -c 
CCINCFLAG=
LD=gcc
LDFLAGS=


TARGET=lnz.exe
TARGETDEFINE=-DWINDOWS
OBJS:=$(OBJS) windowsResource.o
STATE:=octree

# Actual build rules.
# These are supposed everything that might be edited.
TXTS:=$(TXTS) $(wildcard ./*.txt) ./Makefile ./README.md ./windowsResource.rc
SRCS:=$(SRCS) $(wildcard ./*.h) $(wildcard ./*.c)
CS:=$(CS) $(wildcard ./*.c)
OBJS:=$(OBJS) $(CS:.c=.o)
$(OBJS): Makefile

include deps

# windres
windowsResource.o: windowsResource.rc bcj.ico
	windres $< -o $@

# Override defaults
%.o: %.c
	$(CC) $(CCINCFLAG) $(CCFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(LD) $^ -o $@ $(LDFLAGS)
	$(STRIP)
	$(PACK)

TAGS: $(SRCS)
	etags --declarations --ignore-indentation $^


.PHONY: release 
release: $(TARGET)
release: CCFLAGS:=-O4 $(TARGETDEFINE) -flto $(CCFLAGS)
release: LDFLAGS:=-O4 -flto $(LDFLAGS)
release: STRIP:=strip -p $(TARGET)
release: PACK:=upx --best $(TARGET)

.PHONY: debug 
debug: $(TARGET)
debug: CCFLAGS:=$(TARGETDEFINE) -ggdb -DDEBUG $(CCFLAGS)


.PHONY: clean
clean:
	rm -f ./*.o ./$(TARGET) ./$(STATE)

.PHONY: backup
backup: clean release
	git add -A
	git commit -a -m "$(shell cat ~/lnz4/workingon.txt)" || true

.PHONY: depend
depend:
	gcc $(CCINCFLAG) $(TARGETDEFINE) -MM $(CS) > ./deps

.PHONY: run
run: all
	./$(TARGET)

.PHONY: unixify
unixify:
	dos2unix -U $(TXTS) $(SRCS)

