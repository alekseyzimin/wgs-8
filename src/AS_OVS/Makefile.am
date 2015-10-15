noinst_LIBRARIES += lib/libAS_OVS.a
lib_libAS_OVS_a_SOURCES = %D%/AS_OVS_overlap.C		\
                          %D%/AS_OVS_overlapFile.C	\
                          %D%/AS_OVS_overlapStore.C

libCA_a_SOURCES += $(lib_libAS_OVS_a_SOURCES)

bin_PROGRAMS += bin/overlapStore bin/overlapStats bin/convertOverlap	\
                bin/filterOverlap bin/overlapStoreBucketizer		\
                bin/overlapStoreSorter bin/overlapStoreIndexer		\
                bin/overlapStoreBuild

bin_overlapStore_SOURCES = %D%/overlapStore.C %D%/overlapStore_dump.C	\
%D%/overlapStore_erates.C %D%/overlapStore_genomeLength.C
bin_overlapStats_SOURCES = %D%/overlapStats.C
bin_convertOverlap_SOURCES = %D%/convertOverlap.C
bin_filterOverlap_SOURCES = %D%/filterOverlap.C
bin_overlapStoreBucketizer_SOURCES = %D%/overlapStoreBucketizer.C
bin_overlapStoreSorter_SOURCES = %D%/overlapStoreSorter.C
bin_overlapStoreIndexer_SOURCES = %D%/overlapStoreIndexer.C
bin_overlapStoreBuild_SOURCES = %D%/overlapStoreBuild.C

noinst_HEADERS += %D%/AS_OVS_overlapFile.H %D%/AS_OVS_overlap.H	\
%D%/overlapStore.H %D%/overlapStatsBoringStuff.H		\
%D%/AS_OVS_overlapStore.H %D%/AS_UTL_histogram.H
