# ---------------------------------------------------------------------
# SETUP
# ---------------------------------------------------------------------

# --- No implicit rules ---
.SUFFIXES:

# --- Tonc paths ---
# If not defined as environment var, assumed to be 2 dirs up
export TONCCODE	?= ~/code/tonc/code

include $(TONCCODE)/tonc_rules

# --- Main path ---
export PATH	:=	$(DEVKITARM)/bin:$(PATH)


# ---------------------------------------------------------------------
# PROJECT DETAILS
# ---------------------------------------------------------------------

# PROJ		: Base project name
# TITLE		: Title for ROM header (12 characters)
# LIBS		: Libraries to use, formatted as list for linker flags
# BUILD		: Directory for build process temporaries. Should NOT be empty!
# SRCDIRS	: List of source file directories
# INCDIRS	: List of header file directories
# LIBDIRS	: List of library directories
# General note: use . for the current dir, don't leave them empty.

export PROJ	?= $(notdir $(CURDIR))
TITLE			:= $(PROJ)
LIBS			:= -ltonc
BUILD			:= build
SRCDIRS		:= source
INCDIRS		:= include
LIBDIRS		:= $(DEVKITARM)/../libtonc


# --- switches ---

bMB		:= 0	# Multiboot build
bTEMPS	:= 0	# Save gcc temporaries (.i and .s files)
bDEBUG2	:= 0	# Generate debug info (bDEBUG2? Not a full DEBUG flag. Yet)


# ---------------------------------------------------------------------
# BUILD FLAGS
# ---------------------------------------------------------------------

# This is probably where you can stop editing

# --- Architecture ---

ARCH    := -mthumb-interwork -mthumb
RARCH   := -mthumb-interwork -mthumb
IARCH   := -mthumb-interwork -marm -mlong-calls

# --- Main flags ---

CFLAGS	:= -mcpu=arm7tdmi -mtune=arm7tdmi $(ARCH) -O2
CFLAGS	+= -Wall
CFLAGS	+= $(INCLUDE)
CFLAGS	+= -ffast-math -fno-strict-aliasing

CXXFLAGS	:= $(CFLAGS) -fno-rtti -fno-exceptions

ASFLAGS	:= $(ARCH)
LDFLAGS := $(ARCH) -Wl,-Map,$(PROJ).map

# --- switched additions ----------------------------------------------

# --- Multiboot ? ---
ifeq ($(strip $(bMB)), 1)
	TARGET	:= $(PROJ).mb
else
	TARGET	:= $(PROJ)
endif

# --- Save temporary files ? ---
ifeq ($(strip $(bTEMPS)), 1)
	CFLAGS	+= -save-temps
endif

# --- Debug info ? ---

ifeq ($(strip $(bDEBUG2)), 1)
	CFLAGS	+= -g
	LDFLAGS	+= -g
endif


# ---------------------------------------------------------------------
# BUILD PROCEDURE
# ---------------------------------------------------------------------

ifneq ($(BUILD),$(notdir $(CURDIR)))

# Still in main dir: 
# * Define/export some extra variables
# * Invoke this file again from the build dir
# PONDER: what happens if BUILD == "" ?

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export VPATH	:=									\
	$(foreach dir, $(SRCDIRS) , $(CURDIR)/$(dir))	\
	$(foreach dir, $(DATADIRS), $(CURDIR)/$(dir))

export DEPSDIR	:=	$(CURDIR)/$(BUILD)

# --- List source and data files ---

CFILES		:=	$(foreach dir, $(SRCDIRS) , $(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir, $(SRCDIRS) , $(notdir $(wildcard $(dir)/*.cpp)))
SFILES		:=	$(foreach dir, $(SRCDIRS) , $(notdir $(wildcard $(dir)/*.s)))
BINFILES	:=	$(foreach dir, $(DATADIRS), $(notdir $(wildcard $(dir)/*.*)))

# --- Set linker depending on C++ file existence ---
ifeq ($(strip $(CPPFILES)),)
	export LD	:= $(CC)
else
	export LD	:= $(CXX)
endif

# --- Define object file list ---
export OFILES	:=									\
	$(addsuffix .o, $(BINFILES))					\
	$(CFILES:.c=.o) $(CPPFILES:.cpp=.o)				\
	$(SFILES:.s=.o)

# --- Create include and library search paths ---
export INCLUDE	:=									\
	$(foreach dir,$(INCDIRS),-I$(CURDIR)/$(dir))	\
	$(foreach dir,$(LIBDIRS),-I$(dir)/include)		\
	-I$(CURDIR)/$(BUILD)

export LIBPATHS	:=	-L$(CURDIR) $(foreach dir,$(LIBDIRS),-L$(dir)/lib)


# --- More targets ----------------------------------------------------

.PHONY: $(BUILD) clean

# --- Create $(BUILD) if necessary, and run this makefile from there ---

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

all	: $(BUILD)

clean:
	@echo clean ...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).gba


else		# If we're here, we should be in the BUILD dir

DEPENDS	:=	$(OFILES:.o=.d)

# --- Main targets ----

$(OUTPUT).gba	:	$(OUTPUT).elf

$(OUTPUT).elf	:	$(OFILES)


-include $(DEPENDS)


endif		# End BUILD switch

# EOF
