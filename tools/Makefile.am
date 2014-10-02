AM_CFLAGS = -g
AM_LDFLAGS = -static-libstdc++
#AM_CPPFLAGS =  -I$(top_srcdir)/src/include
#AM_CPPFLAGS = 

bin_PROGRAMS = alac uniquery

alac_SOURCES = alacrity.c
alac_LDFLAGS = $(RIDCOMPRESS_LDFLAGS) $(TIMER_LDFLAGS) -static-libstdc++
alac_LDADD = ../src/libalacrity.a $(RIDCOMPRESS_LIBS) $(TIMER_LIBS) 
alac_CPPFLAGS =  -I$(top_srcdir)/src/include $(TIMER_CPPFLAGS) $(RIDCOMPRESS_CPPFLAGS)

uniquery_SOURCES = uniquery.c
uniquery_LDFLAGS = $(RIDCOMPRESS_LDFLAGS) $(TIMER_LDFLAGS) -static-libstdc++
uniquery_LDADD = ../src/libalacrity.a $(RIDCOMPRESS_LIBS) $(TIMER_LIBS) 
uniquery_CPPFLAGS =  -I$(top_srcdir)/src/include $(TIMER_CPPFLAGS) $(RIDCOMPRESS_CPPFLAGS)
