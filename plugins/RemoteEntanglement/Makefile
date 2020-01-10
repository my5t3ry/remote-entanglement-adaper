#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = RemoteEntanglementPlugin

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	RemoteEntanglementPlugin.cpp

# --------------------------------------------------------------
# Do some magic
ROC_PATH=../../roc/build/x86_64-pc-linux-gnu/gcc-7.4.0-release/libroc_unstripped.so.0.1
LDFLAGS = $(ROC_PATH)
CXXFLAGS = -I../../roc/src/modules/ 

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(HAVE_JACK),true)
TARGETS += jack
endif

TARGETS += ladspa
TARGETS += vst

ifeq ($(HAVE_DGL),true)
ifeq ($(HAVE_LIBLO),true)
TARGETS += dssi
endif
endif

ifeq ($(HAVE_DGL),true)
TARGETS += lv2_sep
else
TARGETS += lv2_dsp
endif

all: $(TARGETS)

# --------------------------------------------------------------
