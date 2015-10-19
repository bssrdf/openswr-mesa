# Copyright (C) 2015 Intel Corporation.   All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

AUTOMAKE_OPTIONS = subdir-objects

include Makefile.sources
include $(top_srcdir)/src/gallium/Automake.inc

AM_CXXFLAGS = \
	$(GALLIUM_DRIVER_CFLAGS) \
	-std=c++11 -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS \
	$(SWR_ARCH_FLAG) \
	$(LLVM_CFLAGS)

noinst_LTLIBRARIES = libmesaswr.la

libmesaswr_la_SOURCES = $(CXX_SOURCES)

libmesaswr_la_LDFLAGS =

if SWR_NATIVE
BUILT_SOURCES = \
	rasterizer/scripts/gen_knobs.cpp \
	rasterizer/scripts/gen_knobs.h \
	rasterizer/jitter/state_llvm.h

rasterizer/scripts/gen_knobs.cpp rasterizer/scripts/gen_knobs.h: rasterizer/scripts/gen_knobs.py rasterizer/scripts/knob_defs.py rasterizer/scripts/templates/knobs.template
	$(PYTHON2) $(PYTHON_FLAGS) \
		$(srcdir)/rasterizer/scripts/gen_knobs.py \
		rasterizer/scripts

rasterizer/jitter/state_llvm.h: rasterizer/jitter/scripts/gen_llvm_types.py rasterizer/core/state.h
	$(PYTHON2) $(PYTHON_FLAGS) \
		$(srcdir)/rasterizer/jitter/scripts/gen_llvm_types.py \
		--input $(srcdir)/rasterizer/core/state.h \
		--output rasterizer/jitter/state_llvm.h

libmesaswr_la_SOURCES += \
	$(COMMON_CXX_SOURCES) \
	$(CORE_CXX_SOURCES) \
	$(JITTER_CXX_SOURCES) \
	$(MEMORY_CXX_SOURCES) \
	rasterizer/scripts/gen_knobs.cpp \
	rasterizer/scripts/gen_knobs.h
AM_CXXFLAGS += \
	-I$(srcdir)/rasterizer \
	-I$(srcdir)/rasterizer/core \
	-I$(srcdir)/rasterizer/jitter \
	-I$(builddir)/rasterizer/scripts \
	-I$(builddir)/rasterizer/jitter
else
libmesaswr_la_LDFLAGS += -L$(SWR_LIBDIR) -lSWR
AM_CXXFLAGS += \
	-I$(SWR_INCLUDEDIR) \
	-I$(SWR_INCLUDEDIR)/core \
	-I$(SWR_INCLUDEDIR)/jitter \
	-I$(SWR_INCLUDEDIR)/build/jitter \
	-I$(SWR_INCLUDEDIR)/build/scripts
endif

libmesaswr_la_LDFLAGS += -lnuma


EXTRA_DIST = SConscript
