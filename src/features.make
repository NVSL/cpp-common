# -*- mode: makefile; -*-

CLWB_PRESENT_       := $(shell cat /proc/cpuinfo | grep -o -m1 clwb)
CLFLUSHOPT_PRESENT_ := $(shell cat /proc/cpuinfo | grep -o -m1 clflushopt)
SFENCE_PRESENT_     := $(shell cat /proc/cpuinfo | grep -o -m1 sse2)

ifneq ($(PERFORM_CHECKS),0)
    # Check if /proc/cpuinfo is available to determine available
    # extensions. TODO: Do this in a more portable way (CPUID?)
    ifeq (,$(wildcard /proc/cpuinfo))
        $(error "/proc/cpuinfo not found, can't check for CLWB")
    endif

    ifeq ($(CLWB_PRESENT_),clwb)
        $(info Checking for clwb... supported)
        export CLWB_FLAG :=-DCLWB_AVAIL
    else
        $(info SKIP: clwb not supported)
    endif

    ifeq ($(CLFLUSHOPT_PRESENT_),clflushopt)
        $(info Checking for clflushopt... supported)
        export CLFLUSHOPT_FLAG :=-DCLFLUSHOPT_AVAIL
    else
        $(info SKIP: clflushopt not supported)
    endif

    ifeq ($(SFENCE_PRESENT_),sse2)
        $(info Checking for sfence... supported)
        export SFENCE_FLAG :=-DSFENCE_AVAIL
    else
        $(info SKIP: sfence not supported)
    endif

    PERFORM_CHECKS := 0 
endif
