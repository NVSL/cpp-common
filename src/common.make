SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
DEPS_DIR := .deps

include $(SELF_DIR)colors.make

ifdef RELEASE
CXXFLAGS    :=-std=gnu++20 -ggdb3 -O3 -march=native -fPIC -Wall -DNDEBUG
CXXFLAGS    +=-fno-omit-frame-pointer
else
CXXFLAGS    :=-std=gnu++20 -ggdb3 -O0 -fPIC -Wall
CXXFLAGS    += -fno-omit-frame-pointer -Werror
endif

ifdef DEBUG_BUILD
CXXFLAGS    += -v
endif

CXXFLAGS    += -Wpedantic
CXXFLAGS    += $(EXTRA_CXXFLAGS) $(PUDDLE_CXXFLAGS)
CXXFLAGS    +=$(CLWB_FLAG) $(CLFLUSHOPT_FLAG) $(SFENCE_FLAG)
LDFLAGS     :=$(EXTRA_LDFLAGS)
LINKFLAGS   :=$(EXTRA_LINKFLAGS)
INCLUDE     +=-iquote$(SELF_DIR)include
INCLUDE     +=-iquote$(SELF_DIR)
INCLUDE     +=-I$(SELF_DIR)../vendor/boost_intrusive_containers/include
INCLUDE     +=-I$(SELF_DIR)../vendor/nlohmann/include/
INCLUDE     +=-I$(SELF_DIR)../vendor/interval-tree

ifdef RELEASE
CXXFLAGS    +=-DRELEASE
endif

PUDDLES_CC      =$(QUIET_CC)$(CC)
PUDDLES_CXX     =$(QUIET_CXX)$(CXX)
PUDDLES_LN      =$(QUIET_LINK)ln
PUDDLES_MAKE    =+$(QUIET_MAKE)
PUDDLES_AR      =+$(QUIET_AR)$(AR)
PUDDLES_INSTALL =+$(QUIET_INSTALL)install

ifndef V
QUIET_CC      = @printf '     %b %b\n' $(CCCOLOR)CC$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_CXX     = @printf '    %b %b\n'  $(CCCOLOR)CXX$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_LINK    = @printf '   %b %b\n' $(LINKCOLOR)LINK$(ENDCOLOR) $(BINCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_MAKE    = @printf '   %b %b\n'   $(MAKECOLOR)MAKE$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;$(MAKE) -s
QUIET_AR      = @printf '     %b %b\n'   $(ARCOLOR)AR$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
QUIET_INSTALL = @printf '%b %b\n'   $(INSTALLCOLOR)INSTALL$(ENDCOLOR) $(SRCCOLOR)$@$(ENDCOLOR) 1>&2;
else
QUIET_MAKE    = $(MAKE)
endif
