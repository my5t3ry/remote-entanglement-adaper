#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#
# Modified by Patrick Desaulniers
#
# --------------------------------------------------------------
# Project name, used for binaries
#
# Set PLUGIN_NAME in Makefile.mk at the project's root directory
#

NAME = $(PLUGIN_NAME)

# --------------------------------------------------------------
# Plugin types to build

BUILD_LV2 ?= true
BUILD_VST2 ?= true
BUILD_JACK ?= true
BUILD_DSSI ?= true
BUILD_LADSPA ?= false


ROC_PATH=../../roc/build/x86_64-pc-linux-gnu/clang-7.0.1-release/libroc_unstripped.so.0.1
CXXFLAGS = -I../../roc/src/modules/

NANOVG_PATH=../../nanovg/build/libnanovg.so
CXXFLAGS = -I../../nanovg/src/

NANOGUI_PATH=../../nanogui/build/libnanogui.so
LDFLAGS = $(NANOVG_PATH) $(NANOGUI_PATH) $(ROC_PATH)
CXXFLAGS = -I../../nanogui/include/

# --------------------------------------------------------------
# Files to build

OBJS_DSP = \
	Common/Utils/src/Mathf.cpp.o \
	Common/Structures/src/Graph.cpp.o \
	Common/Structures/src/Oversampler.cpp.o \
	Common/Structures/src/ParamSmooth.cpp.o \
	Libs/DSPFilters/source/Butterworth.cpp.o \
	Libs/DSPFilters/source/Biquad.cpp.o \
	Libs/DSPFilters/source/Cascade.cpp.o \
	Libs/DSPFilters/source/Filter.cpp.o \
	Libs/DSPFilters/source/Param.cpp.o \
	Libs/DSPFilters/source/Butterworth.cpp.o \
	Libs/DSPFilters/source/PoleFilter.cpp.o \
	Libs/DSPFilters/source/RBJ.cpp.o \
	Libs/DSPFilters/source/Bessel.cpp.o \
	Libs/DSPFilters/source/RootFinder.cpp.o \
	RemoteEntanglementPlugin.cpp.o

OBJS_UI  = \
	Common/Utils/src/Mathf.cpp.o \
	Common/Structures/src/Graph.cpp.o \
	Common/Structures/src/Margin.cpp.o \
	Common/Structures/src/Animation.cpp.o \
	Common/Widgets/src/WolfWidget.cpp.o \
	Common/Widgets/src/GraphNode.cpp.o \
	Common/Widgets/src/GraphWidget.cpp.o \
	Common/Widgets/src/NanoSwitch.cpp.o \
	Common/Widgets/src/RemoveDCSwitch.cpp.o \
	Common/Widgets/src/NanoButton.cpp.o \
	Common/Widgets/src/ResetGraphButton.cpp.o \
	Common/Widgets/src/NanoWheel.cpp.o \
	Common/Widgets/src/OversampleWheel.cpp.o \
	Common/Widgets/src/NanoKnob.cpp.o \
	Common/Widgets/src/VolumeKnob.cpp.o \
	Common/Widgets/src/ResizeHandle.cpp.o \
	Common/Widgets/src/LabelBox.cpp.o \
	Common/Widgets/src/LabelBoxList.cpp.o \
	Common/Widgets/src/BipolarModeSwitch.cpp.o \
	Common/Widgets/src/LabelContainer.cpp.o \
	Common/Widgets/src/GlowingLabelsBox.cpp.o \
	Common/Widgets/src/NanoLabel.cpp.o \
	Common/Widgets/src/WidgetBar.cpp.o \
	Common/Widgets/src/ArrowButton.cpp.o \
	Common/Widgets/src/RightClickMenu.cpp.o \
	Config/src/Config.cpp.o \
	Libs/inih/ini.c.o \
	Libs/inih/INIReader.cpp.o \
	Resources/Fonts/chivo_bold.cpp.o \
	Resources/Fonts/chivo_italic.cpp.o \
	RemoteEntanglementPluginUi.cpp.o

# --------------------------------------------------------------
# Do some magic

include ../Makefile.mk

# --------------------------------------------------------------
# Additional flags

ifeq ($(PARANOID_MODE),true)
BASE_FLAGS += -Wall -Wextra -Werror
endif

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(BUILD_JACK),true)
ifeq ($(HAVE_JACK),true)
TARGETS += jack
endif
endif

ifeq ($(BUILD_LADSPA),true)
TARGETS += ladspa
endif

ifeq ($(BUILD_DSSI),true)
ifeq ($(HAVE_DGL),true)
ifeq ($(HAVE_LIBLO),true)
TARGETS += dssi
endif
endif
endif

ifeq ($(BUILD_LV2),true)
ifeq ($(HAVE_DGL),true)
TARGETS += lv2_sep
else
TARGETS += lv2_dsp
endif
endif

ifeq ($(BUILD_VST2),true)
TARGETS += vst
endif

all: $(TARGETS)

# --------------------------------------------------------------
