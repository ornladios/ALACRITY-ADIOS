#AM_CPPFLAGS = -I$(top_srcdir)/ridcompress
#AM_CFLAGS = -g
#AM_CXXFLAGS = -fno-exceptions -fno-rtti -D__STDC_LIMIT_MACROS
AM_CPPFLAGS = -D__STDC_LIMIT_MACROS

RIDCOMPRESS_INCLUDES = \
	include/uniquery.h \
        include/timer.h \
	uniquery/helpers.h

lib_LIBRARIES = libridcompress.a

libridcompress_a_SOURCES = \
    gen_fixedlengthcode.cpp \
    patchedframeofreference.cpp \
    pfordelta-c-interface.cpp \
    $(RIDCOMPRESS_INCLUDES)
 
#include_HEADERS = 
