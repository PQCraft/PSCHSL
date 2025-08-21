# Targets:
#   'all' which builds 'lib'
#   'lib' or 'library' builds the static and shared libraries
#   'lib.a' or 'static_library' builds only the static library
#   'lib.so' or 'shared_library' builds only the shared library
#   'clean' removes intermediate files
#   'distclean' removes intermediates and outputs
#   'install' copies the headers and outputs to $(SYSINC) ('/usr/include' by default) and $(SYSLIB) ('/usr/lib' by default)
#   'uninstall' removes the headers and outputs from $(SYSINC) and $(SYSLIB)

# Options and vars:
#   'CROSS' sets the Makefile up for cross-compiling; valid values are unset and 'win32' (unset by default)
#   'DEBUG' enables debug mode if set to 'y' (unset by default)
#   'ASAN' enables the address sanitizer if set to 'y' (unset by default)
#   'O' holds the optimization level to use (default is '2' in release and 'g' in debug)
#   'USESTDTHREAD' enables the usage of C11 threads instead of pthreads
#   'USEWINPTHREAD' enables the usage of winpthreads instead of win32 threads when compiling for windows
#   'SRCDIR' holds the path to where the sources are
#   'OBJDIR' holds the path to where the object files are to be written
#   'OUTDIR' holds the paht to where the output files are to be written
#   'BIN' holds the base name to give to the output files
#   'CC' holds the name of the C compiler (defaults to gcc)
#   'LD' holds the name of the linker (defaults to $(CC))
#   'AR' holds the name of the archiver (defaults to ar)
#   'TOOLCHAIN' holds a prefix to add to all the tools (unset by default)
#   'CFLAGS' holds the flags to pass to the C compiler; use +=
#   'CPPFLAGS' holds the flags to pass to the C preprocessor; use +=
#   'LDFLAGS' holds the flags to pass to the linker; use +=
#   'LDLIBS' holds the library flags to pass to the linker; use +=
#   'SYSINC' holds the dir to install headers to
#   'SYSLIB' holds the dir to install libraries to

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
_CC := $(TOOLCHAIN)$(CC)
_LD := $(TOOLCHAIN)$(LD)
_AR := $(TOOLCHAIN)$(AR)

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

install:
	@echo Installing headers...; mkdir '$(SYSINC)/pschsl'; cp -rf '$(SRCDIR)'/*.h '$(SYSINC)/pschsl'
	@if [ -f '$(BIN.a)' ]; then echo Installing static library...; cp -f '$(BIN.a)' '$(SYSLIB)'; fi
	@if [ -f '$(BIN.so)' ]; then echo Installing shared library...; cp -f '$(BIN.so)' '$(SYSLIB)'; fi

uninstall:
	@echo Uninstalling headers...; rm -rf '$(SYSINC)/pschsl'
	@if [ -f '$(SYSLIB)/$(BIN.a)' ]; then echo Uninstalling static library...; rm -f '$(SYSLIB)/$(BIN.a)'; fi
	@if [ -f '$(SYSLIB)/$(BIN.so)' ]; then echo Uninstalling shared library...; rm -f '$(SYSLIB)/$(BIN.so)'; fi

.PHONY: all lib library lib.a static_library lib.so shared_library clean distclean install uninstall
