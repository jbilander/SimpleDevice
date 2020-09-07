FILENAME=simple.device
RELDIR=build-release
DEBUGDIR=build-debug
OBJECTS=device.o

SRCDIRS=.
INCDIRS=.

#TARGET = elf-toolchain
TARGET = hunk-toolchain

ifeq ($(MAKECMDGOALS), debug)
 DIR=$(DEBUGDIR)
 EXTRA_CFLAGS+= -g -O2
 EXTRA_ASFLAGS+= -g
 ifeq ($(TARGET), hunk-toolchain)
  EXTRA_CFLAGS+= -DDEBUG -mcrt=clib2
  EXTRA_LDFLAGS+= -ldebug
 endif
else
 DIR=$(RELDIR)
 EXTRA_CFLAGS+= -s -O2
endif

ifeq ($(TARGET), elf-toolchain)
 CC=m68k-amiga-elf-gcc
 AS=m68k-amiga-elf-as
 ELF_SUFFIX=.elf
 EXTRA_LDFLAGS+= -Wl,--emit-relocs,-Ttext=0,-Map=$(DIR)/$(FILENAME)$(ELF_SUFFIX).map
endif

ifeq ($(TARGET), hunk-toolchain)
 CC=m68k-amigaos-gcc
 AS=m68k-amigaos-as
 EXTRA_LDFLAGS+= -Wl,-Map=$(DIR)/$(FILENAME).map
endif

CFLAGS+= -m68000 -Wall -Wextra -Wno-unused-parameter -fomit-frame-pointer
ASFLAGS+= -m68000
LDFLAGS+= -nostdlib -nostartfiles

OBJS:=$(addprefix $(DIR)/,$(OBJECTS))

CFLAGS+=$(addprefix -I,$(INCDIRS)) $(EXTRA_CFLAGS)
ASFLAGS+=$(EXTRA_ASFLAGS)
LDFLAGS+=$(EXTRA_LDFLAGS)

# Search paths
vpath %.c $(SRCDIRS)
vpath %.s $(SRCDIRS)

release: $(TARGET)
debug: $(TARGET)

elf-toolchain: $(DIR) $(OBJS) $(DIR)/$(FILENAME)
	elf2hunk $(DIR)/$(FILENAME)$(ELF_SUFFIX) $(DIR)/$(FILENAME)
	m68k-amiga-elf-objdump -D $(DIR)/$(FILENAME)$(ELF_SUFFIX) > $(DIR)/$(FILENAME)$(ELF_SUFFIX).s
#	m68k-amigaos-objdump -D $(DIR)/$(FILENAME) > $(DIR)/$(FILENAME).s

$(DIR)/$(FILENAME): #ELF_SUFFIX is empty for hunk-toolchain so this below works for both targets
	$(CC) $(CFLAGS) -o $@$(ELF_SUFFIX) $(OBJS) $(LDFLAGS)

hunk-toolchain: $(DIR) $(OBJS) $(DIR)/$(FILENAME)
	m68k-amigaos-objdump -D $(DIR)/$(FILENAME) > $(DIR)/$(FILENAME).s

$(DIR):
	@mkdir $(DIR)

$(DIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	@$(DEL_EXE)
	
$(DIR)/%.o: %.s
	$(AS) $(ASFLAGS) -o $@ $<
	@$(DEL_EXE)


ifeq '$(findstring ;,$(PATH))' ';'
 UNAME := Windows_Native
endif

ifeq ($(UNAME),Windows_Native)
 RM= rmdir /s /q
 DEVNULL= 2>nul || ver>nul
 DEL_EXE=if exist $(DIR)\$(FILENAME) del /q $(DIR)\$(FILENAME)
else
 RM= rm -rf
 DEL_EXE=test -f && $(RM) $(DIR)/$(FILENAME)
endif

.PHONY: clean

clean: cleandebug
clean: cleanrelease

cleandebug:
	$(info Cleaning debug)
	@$(RM) $(DEBUGDIR) $(DEVNULL)

cleanrelease:
	$(info Cleaning release)
	@$(RM) $(RELDIR) $(DEVNULL)
