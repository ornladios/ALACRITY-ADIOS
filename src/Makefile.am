AM_CPPFLAGS = -I$(srcdir) -I$(srcdir)/include $(TIMER_CPPFLAGS) $(RIDCOMPRESS_CPPFLAGS)
AM_CFLAGS = -g 

ALACRITY_EXT_INCLUDES = \
	include/alacrity.h \
	include/alacrity-core.h \
	include/alacrity-filestore.h \
	include/alacrity-io-backend.h \
	include/alacrity-part-manip.h \
	include/alacrity-rid-compress.h \
	include/alacrity-serialization.h \
	include/alacrity-serialization-debug.h \
	include/alacrity-serialization-legacy.h \
	include/alacrity-store.h \
	include/alacrity-types.h \
	include/alacrity-datatype.h \
	include/ALUtil.h \
	include/memstream.h \
	include/uniquery.h \
	uniquery/helpers.h

lib_LIBRARIES = libalacrity.a
libalacrity_a_SOURCES = \
    ALConfigure.c ALEncode.c ALDecode.c ALConvert.c ALMerge.c \
    ALFileStore.c ALStore.c  ALSerialization.c ALSerializationLegacy.c \
    ALDataDestroy.c ALDatatype.c ALUtil.c \
    ALRIDCompression.c ALRIDTranslate.c \
    memstream/memstream.c \
    uniquery/uniquery.c uniquery/findbinrange.templates.cc \
    uniquery/readreconst.templates.cc uniquery/trimbins.templates.cc \
    io/ALStorePOSIX.c \
    $(ALACRITY_INT_INCLUDES) \
    $(ALACRITY_EXT_INCLUDES)
 
include_HEADERS = $(ALACRITY_EXT_INCLUDES)
