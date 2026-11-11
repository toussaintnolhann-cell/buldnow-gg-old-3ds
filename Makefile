#---------------------------------------------------------------------------------
# BuildNow3DS - Makefile
# Requires: devkitARM + libctru + citro2d (devkitPro toolchain)
# Install : https://devkitpro.org/wiki/Getting_Started
#
# Usage:
#   make          → build BuildNow3DS.3dsx + BuildNow3DS.smdh
#   make clean    → remove build dir
#
# Target: 3DS (ARM11 userland, Old 3DS + New 3DS)
#---------------------------------------------------------------------------------

APP_TITLE    := BuildNow3DS
APP_AUTHOR   := bench/olliE
APP_DESC     := Build Practice Trainer for 3DS
APP_ICON     := icon.png

TARGET       := BuildNow3DS
BUILD        := build
SOURCES      := source
INCLUDES     := include
ROMFS        := romfs

#---------------------------------------------------------------------------------
# devkitPro paths — set DEVKITPRO in your environment
#   export DEVKITPRO=/opt/devkitpro
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITPRO)),)
  $(error "Please set DEVKITPRO in your environment. export DEVKITPRO=/opt/devkitpro")
endif

DEVKITARM    := $(DEVKITPRO)/devkitARM

#---------------------------------------------------------------------------------
# tools
#---------------------------------------------------------------------------------
PREFIX       := $(DEVKITARM)/bin/arm-none-eabi-
CC           := $(PREFIX)gcc
CXX          := $(PREFIX)g++
AR           := $(PREFIX)ar
OBJCOPY      := $(PREFIX)objcopy
3DSXTOOL     := 3dsxtool
SMDHTOOL     := smdhtool

#---------------------------------------------------------------------------------
# flags
#---------------------------------------------------------------------------------
ARCH         := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS       := -g -Wall -Wextra -O2 -mword-relocations \
                -ffunction-sections -fdata-sections \
                $(ARCH) -D__3DS__ \
                $(foreach d,$(INCLUDES), -I$(d)) \
                -I$(DEVKITPRO)/libctru/include \
                -I$(DEVKITPRO)/portlibs/3ds/include

LDFLAGS      := -specs=3dsx.specs $(ARCH) \
                -L$(DEVKITPRO)/libctru/lib \
                -L$(DEVKITPRO)/portlibs/3ds/lib

LIBS         := -lcitro2d -lcitro3d -lctru -lm

#---------------------------------------------------------------------------------
# source → object
#---------------------------------------------------------------------------------
CFILES       := $(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
OFILES       := $(patsubst %.c,$(BUILD)/%.o,$(notdir $(CFILES)))

VPATH        := $(SOURCES)

#---------------------------------------------------------------------------------
# ROMFS
#---------------------------------------------------------------------------------
ifneq ($(strip $(ROMFS)),)
  ROMFS_FLAG := --romfs=$(ROMFS)
endif

#---------------------------------------------------------------------------------
# phony targets
#---------------------------------------------------------------------------------
.PHONY: all clean

all: $(BUILD) $(TARGET).3dsx

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/%.o: %.c
	@echo "  CC  $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OFILES)
	@echo "  LD  $@"
	$(CC) $(LDFLAGS) $(OFILES) $(LIBS) -o $@

$(TARGET).smdh: $(APP_ICON)
	@echo "  SMDH $@"
	$(SMDHTOOL) --create "$(APP_TITLE)" "$(APP_DESC)" "$(APP_AUTHOR)" $(APP_ICON) $@

$(TARGET).3dsx: $(TARGET).elf $(TARGET).smdh
	@echo "  3DSX $@"
	$(3DSXTOOL) $< $@ --smdh=$(TARGET).smdh $(ROMFS_FLAG)

clean:
	@echo "  CLEAN"
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).3dsx $(TARGET).smdh
