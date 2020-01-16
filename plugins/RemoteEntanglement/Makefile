#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = RemoteEntanglement

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	RemoteEntanglementPlugin.cpp

FILES_UI  = \
	RemoteEntanglementPluginUi.cpp

# --------------------------------------------------------------
# Do some magic

ROC_PATH=../../roc/build/x86_64-pc-linux-gnu/gcc-7.4.0-release/libroc_unstripped.so.0.1
LDFLAGS = $(ROC_PATH)
CXXFLAGS = -I../../roc/src/modules/

include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(HAVE_JACK),true)
ifeq ($(HAVE_OPENGL),true)
TARGETS += jack
endif
endif

ifneq ($(MACOS_OR_WINDOWS),true)
ifeq ($(HAVE_LIBLO),true)
ifeq ($(HAVE_OPENGL),true)
TARGETS += dssi
endif
endif
endif

ifeq ($(HAVE_OPENGL),true)
TARGETS += lv2_sep
else
TARGETS += lv2_dsp
endif

TARGETS += vst

all: $(TARGETS)

# --------------------------------------------------------------
