SRCDIR := src/pschsl
OBJDIR := obj/pschsl
OUTDIR := .

ifeq ($(OS),Windows_NT)
    CROSS := win32
endif

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS := $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SOURCES))

BIN := pschsl

BIN := $(OUTDIR)/lib$(BIN)
BIN.a := $(BIN).a
ifneq ($(CROSS),win32)
    BIN.so := $(BIN).so
else
    BIN.so := $(BIN).dll
endif

CC ?= gcc
LD := $(CC)
AR := ar
STRIP ?= strip
_CC := $(TOOLCHAIN)$(CC)
_LD := $(TOOLCHAIN)$(LD)
_AR := $(TOOLCHAIN)$(AR)
_STRIP := $(TOOLCHAIN)$(STRIP)

CFLAGS += -Wall -Wextra -Wuninitialized -Wundef
CPPFLAGS += -D_DEFAULT_SOURCE -D_GNU_SOURCE
LDLIBS += -lz-ng
ifneq ($(DEBUG),y)
    CPPFLAGS += -DNDEBUG
    O := 2
else
    CFLAGS += -g -Wdouble-promotion -fno-omit-frame-pointer -std=c99 -pedantic
    O := g
endif
CFLAGS += -O$(O)
ifeq ($(ASAN),y)
    CFLAGS += -fsanitize=address
    LDFLAGS += -fsanitize=address
endif
ifneq ($(USESTDTHREAD),y)
    ifneq ($(CROSS),win32)
        CFLAGS += -pthread
        LDLIBS += -lpthread
    else
        ifeq ($(USEWINPTHREAD),y)
            CFLAGS += -pthread
            CPPFLAGS += -DPSCHSL_THREADING_USEWINPTHREAD
            LDLIBS += -l:libwinpthread.a
        endif
    endif
else
    CPPFLAGS += -DPSCHSL_THREADING_USESTDTHREAD
endif

.SECONDEXPANSION:

define mkdir
if [ ! -d '$(1)' ]; then echo 'Creating $(1)/...'; mkdir -p '$(1)'; fi; true
endef
define rm
if [ -f '$(1)' ]; then echo 'Removing $(1)...'; rm -f '$(1)'; fi; true
endef
define rmdir
if [ -d '$(1)' ]; then echo 'Removing $(1)/...'; rm -rf '$(1)'; fi; true
endef

deps.filter := %.c %.h
deps.option := -MM
define deps
$$(filter $$(deps.filter),,$$(shell $(_CC) $(CFLAGS) $(CPPFLAGS) -E $(deps.option) $(1)))
endef

all: lib

$(OUTDIR):
	@$(call mkdir,$@)

$(OBJDIR):
	@$(call mkdir,$@)

$(OBJDIR)/%.o: $(SRCDIR)/%.c $(call deps,$(SRCDIR)/%.c) | $(OBJDIR)
	@echo Compiling $<...
	@$(_CC) $(CFLAGS) $(CPPFLAGS) $< -c -o $@
	@echo Compiled $<

$(BIN.a): $(OBJECTS) | $(OUTDIR)
	@echo Linking $@...
	@$(_AR) rcs $@ $^
	@echo Linked $@

$(BIN.so): $(OBJECTS) | $(OUTDIR)
	@echo Linking $@...
	@$(_LD) -shared $(LDFLAGS) $^ $(LDLIBS) -o $@
	@echo Linked $@

lib library: $(BIN.a) $(BIN.so)
	@:

lib.a static_library: $(BIN.a)
	@:

lib.so shared_library: $(BIN.so)
	@:

clean:
	@$(call rmdir,$(OBJDIR))

distclean: clean
	@$(call rm,$(BIN.a))
	@$(call rm,$(BIN.so))

SYSINC := /usr/include
SYSLIB := /usr/lib
SYSBIN := /usr/bin

install:
	@echo Installing headers...; mkdir '$(SYSINC)/pschsl'; cp -rf '$(SRCDIR)'/*.h '$(SYSINC)/pschsl'
	@if [ -f '$(BIN.a)' ]; then echo Installing static library...; cp -f '$(BIN.a)' '$(SYSLIB)'; fi
	@if [ -f '$(BIN.so)' ]; then echo Installing shared library...; cp -f '$(BIN.so)' '$(SYSLIB)'; fi

uninstall:
	@echo Uninstalling headers...; rm -rf '$(SYSINC)/pschsl'
	@if [ -f '$(SYSLIB)/$(BIN.a)' ]; then echo Uninstalling static library...; rm -f '$(SYSLIB)/$(BIN.a)'; fi
	@if [ -f '$(SYSLIB)/$(BIN.so)' ]; then echo Uninstalling shared library...; rm -f '$(SYSLIB)/$(BIN.so)'; fi

.PHONY: all lib library lib.a static_library lib.so shared_library clean distclean install uninstall
