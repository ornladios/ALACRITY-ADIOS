AM_CPPFLAGS = -I$(top_srcdir)/src/include $(RIDCOMPRESS_CPPFLAGS) $(COMPUTIL_CPPFLAGS)
AM_LDFLAGS = $(TIMER_LDFLAGS) $(RIDCOMPRESS_LDFLAGS) $(COMPUTIL_LDFLAGS)
LDADD = ../src/libalacrity.a $(RIDCOMPRESS_LIBS) $(COMPUTIL_LIBS)

check_PROGRAMS =
check_PROGRAMS += test-timer
check_PROGRAMS += test-encode
#check_PROGRAMS += test-decode
check_PROGRAMS += test-merge
check_PROGRAMS += test-serialize
check_PROGRAMS += test-serialize-ciim
check_PROGRAMS += test-filestore
check_PROGRAMS += test-store
check_PROGRAMS += test-store-multipart
check_PROGRAMS += test-rid-translate
check_PROGRAMS += test-uniquery-correctness
check_PROGRAMS += test-uniquery
check_PROGRAMS += test-uniquery-bi
check_PROGRAMS += bench-encode

test_timer_SOURCES = test-timer.c
test_encode_SOURCES = test-encode.c
#test_decode_SOURCES = test-decode.c
test_merge_SOURCES = test-merge.c
test_serialize_SOURCES = test-serialize.c
test_serialize_ciim_SOURCES = test-serialize-ciim.c
test_filestore_SOURCES = test-filestore.c
test_store_SOURCES = test-store.c
test_store_multipart_SOURCES = test-store-multipart.c
test_rid_translate_SOURCES = test-rid-translate.c
test_uniquery_correctness_SOURCES = test-uniquery-correctness.c
test_uniquery_SOURCES = test-uniquery.c
test_uniquery_bi_SOURCES = test-uniquery-based-indexing.c
bench_encode_SOURCES = bench-encode-phases.c

