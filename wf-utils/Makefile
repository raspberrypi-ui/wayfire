# Copyright © 2020 Erik Van Hamme

# Project name.
PROJECT := wf-utils

# List of source files.
SOURCES := \
	wayfire/action/action.cpp \
	wayfire/condition/condition.cpp \
	wayfire/condition/logic_condition.cpp \
	wayfire/condition/test_condition.cpp \
	wayfire/lexer/lexer.cpp \
	wayfire/lexer/literal.cpp \
	wayfire/lexer/symbol.cpp \
	wayfire/parser/action_parser.cpp \
	wayfire/parser/condition_parser.cpp \
        wayfire/parser/lambda_rule_parser.cpp \
	wayfire/parser/rule_parser.cpp \
        wayfire/rule/lambda_rule.cpp \
	wayfire/rule/rule.cpp \
	wayfire/variant.cpp \
	main.cpp \

# List of system source files.
SYSTEMSOURCES := \

# List of include directories.
INCLUDES := \
	. \

# List of system include directories.
SYSTEMINCLUDES := \

# IWYU include directives.
IWYUINCLUDES := -isystem /usr/lib/gcc/x86_64-pc-linux-gnu/9.2.0/include

# List of preprocessor defines.
DEFINES := \

# List of static libraries.
SLIBS := \

# Target architecure
ARCH := -march=native

# Build mode/directory.
BUILDMODE ?= debug
BUILDDIR := $(BUILDMODE)

# Output artefacts.
ELFFILE := $(BUILDDIR)/$(PROJECT).elf
BINFILE := $(BUILDDIR)/$(PROJECT)

# Object list and containing folders.
OBJECTS := $(patsubst %,$(BUILDDIR)/%.o,$(SOURCES)) $(patsubst %,$(BUILDDIR)/%sys.o,$(SYSTEMSOURCES))
OBJECTDIRS := $(sort $(dir $(OBJECTS)))

# List of include paths.
INCLUDEPATHS := $(patsubst %,-I %,$(INCLUDES)) $(patsubst %,-isystem %,$(SYSTEMINCLUDES))

# Tools.
CC := gcc
AS := gcc
GPP := g++
LD := g++
OC := objcopy
OS := size
IWYU := include-what-you-use

# Warnings configuration.
WARNCOMMON :=  -Wall -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 \
	-Winit-self -Wlogical-op -Wmissing-include-dirs -Wredundant-decls -Wstrict-overflow=5 \
	-Wno-unused -Wno-variadic-macros -Wno-parentheses -Wshadow
WARNC := $(WARNCOMMON)
WARNCPP := $(WARNCOMMON) -Wnoexcept -Woverloaded-virtual -Wsign-promo -Wstrict-null-sentinel -Wold-style-cast -Wpedantic
WARNAS := -Wall

# Dependency generation.
DEPGEN = -MMD -MP -MF"$(@:%.o=%.d)"
DEPS := $(patsubst %.o,%.d,$(OBJECTS))

# Debug/release setting
ifeq ($(BUILDMODE),debug)
OPT_DEBUG := -Og -g3 -gdwarf-2 -pipe
endif
ifeq ($(BUILDMODE),release)
OPT_DEBUG := -O3 -pipe
endif

# Compiler/assembler flags.
CFLAGSCOMMON = -c $(DEPGEN) -ffunction-sections -fdata-sections -fmessage-length=0 # -Wa,-adhlns="$@.lst"
CFLAGS = $(CFLAGSCOMMON) -std=c18 -fno-builtin $(OPT_DEBUG)
CPPFLAGS = $(CFLAGSCOMMON) -std=c++17 $(OPT_DEBUG)
ASFLAGS = -c $(OPT_DEBUG)
IWYUFLAGS = -Xiwyu --no_comments -Xiwyu --quoted_includes_first -Xiwyu --cxx17ns

# Linker,objcopy flags.
LDFLAGS = -Xlinker --gc-sections -Wl,-Map=$(BUILDDIR)/$(PROJECT).map
OCFLAGS = -O binary

# Include support for project specific targets.
-include targets.mk

# Set default target to all.
.DEFAULT_GOAL := all

# List of phony targets.
.PHONY:: all info info_verbose clean flash

all:: $(OBJECTDIRS) $(BINFILE)

info::
	@echo "project        $(PROJECT)"
	@echo ""
	@echo "arch           $(ARCH)"
	@echo ""
	@echo "builddir:      $(BUILDDIR)"
	@echo ""
	@echo "includepaths:  $(INCLUDEPATHS)"
	@echo ""
	@echo "cflags:        $(CFLAGS)"
	@echo ""
	@echo "cpplags:       $(CPPFLAGS)"
	@echo ""
	@echo "asflags:       $(ASFLAGS)"
	@echo ""
	@echo "ldflags:       $(LDFLAGS)"
	@echo ""
	@echo "defines:       $(DEFINES)"
	@echo ""
	@echo "static libs:   $(SLIBS)"
	@echo ""

info_verbose:: info
	@echo "sources:       $(SOURCES)"
	@echo ""
	@echo "systemsources: $(SYSTEMSOURCES)"
	@echo ""
	@echo "objects:       $(OBJECTS)"
	@echo ""
	@echo "deps:          $(DEPS)"
	@echo ""

clean::
	@echo "Cleaning"
	@rm -rf $(BUILDDIR)

$(OBJECTDIRS):
	@echo "Creating output directories"
	@mkdir -p $(OBJECTDIRS)

$(ELFFILE): $(OBJECTS) $(SLIBS)
	@echo "Linking"
	@$(LD) $(ARCH) $(LDFLAGS) $(OBJECTS) $(SLIBS) -o $(ELFFILE)
	@echo "---------------------------------------"
	@$(OS) $(ELFFILE)
	@echo "---------------------------------------"

$(BINFILE): $(ELFFILE)
	@$(OC) $(OCFLAGS) $(ELFFILE) $(BINFILE)

flash: $(BINFILE)
	$(OOCD) $(OOCDCFGS) $(OOCDCMDS)

oocd: $(BINFILE)
	$(OOCD) $(OOCDCFGS)

serialflash: $(BINFILE)
	@echo 'Flashing bin file to microcontroller.'
	$(STM32FLASH) $(STM32FLASHARGS) $(STM32FLASHPORT)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

$(BUILDDIR)/%.s.o : %.s
	@echo "Assembling $<"
	@$(AS) $(ARCH) $(ASFLAGS) $(DEFINES) $(WARNAS) -o "$@" "$<"

$(BUILDDIR)/%.S.o : %.S
	@echo "Assembling $<"
	@$(AS) $(ARCH) $(ASFLAGS) $(DEFINES) $(WARNAS) -o "$@" "$<"

$(BUILDDIR)/%.c.o : %.c
	@echo "Compiling $<"
	@$(CC) $(ARCH) $(CFLAGS) $(INCLUDEPATHS) $(DEFINES) $(WARNC) -o "$@" "$<"

$(BUILDDIR)/%.cpp.o : %.cpp
	@-$(IWYU) $(ARCH) $(CPPFLAGS) $(IWYUFLAGS) $(INCLUDEPATHS) $(IWYUINCLUDES) $(DEFINES) -o "$@" "$<" &>> $(BUILDMODE)/iwyu.out || true
	@echo "Compiling $<"
	@$(GPP) $(ARCH) $(CPPFLAGS) $(INCLUDEPATHS) $(DEFINES) $(WARNCPP) -o "$@" "$<"


$(BUILDDIR)/%.ssys.o : %.s
	@echo "Assembling $<"
	@$(AS) $(ARCH) $(ASFLAGS) $(DEFINES) -o "$@" "$<"

$(BUILDDIR)/%.Ssys.o : %.S
	@echo "Assembling $<"
	@$(AS) $(ARCH) $(ASFLAGS) $(DEFINES) -o "$@" "$<"

$(BUILDDIR)/%.csys.o : %.c
	@echo "Compiling $<"
	@$(CC) $(ARCH) $(CFLAGS) -w $(INCLUDEPATHS) $(DEFINES) -o "$@" "$<"

$(BUILDDIR)/%.cppsys.o : %.cpp
	@echo "Compiling $<"
	@$(GPP) $(ARCH) $(CPPFLAGS) -w $(INCLUDEPATHS) $(DEFINES) -o "$@" "$<"

