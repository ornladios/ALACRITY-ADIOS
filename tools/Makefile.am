noinst_bin_PROGRAMS = alac uniquery multiquery
noinst_bindir =

alac_SOURCES = alacrity.c
alac_LDFLAGS = $(RIDCOMPRESS_LDFLAGS) 
alac_LDADD = ../src/libalacrity.a $(RIDCOMPRESS_LIBS) 
alac_CPPFLAGS =  -I$(top_srcdir)/src/include $(RIDCOMPRESS_CPPFLAGS)

uniquery_SOURCES = uniquery.c trycatch.h
uniquery_LDFLAGS = $(RIDCOMPRESS_LDFLAGS) 
uniquery_LDADD = ../src/libalacrity.a $(RIDCOMPRESS_LIBS) 
uniquery_CPPFLAGS =  -I$(top_srcdir)/src/include  $(RIDCOMPRESS_CPPFLAGS)

multiquery_SOURCES = multiquery.c trycatch.h
multiquery_LDFLAGS = $(RIDCOMPRESS_LDFLAGS) 
multiquery_LDADD = ../src/libalacrity.a $(RIDCOMPRESS_LIBS)
multiquery_CPPFLAGS =  -I$(top_srcdir)/src/include  $(RIDCOMPRESS_CPPFLAGS)
