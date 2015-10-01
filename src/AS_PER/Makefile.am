noinst_LIBRARIES += lib/libAS_PER.a
lib_libAS_PER_a_SOURCES = %D%/AS_PER_genericStore.C			\
                          %D%/AS_PER_gkLibrary.C %D%/AS_PER_gkStore.C	\
                          %D%/AS_PER_gkStore_IID.C			\
                          %D%/AS_PER_gkStore_PLC.C			\
                          %D%/AS_PER_gkStore_UID.C			\
                          %D%/AS_PER_gkStore_clearRange.C		\
                          %D%/AS_PER_gkStore_fragments.C		\
                          %D%/AS_PER_gkStore_load.C			\
                          %D%/AS_PER_gkStore_partition.C		\
                          %D%/AS_PER_gkStore_stats.C			\
                          %D%/AS_PER_gkStream.C				\
                          %D%/AS_PER_encodeSequenceQuality.C

libCA_a_SOURCES += $(lib_libAS_PER_a_SOURCES)

noinst_HEADERS += %D%/AS_PER_encodeSequenceQuality.H		\
%D%/AS_PER_gkpStore.H %D%/gkStore.H %D%/gkClearRange.H		\
%D%/AS_PER_genericStore.H %D%/gkStream.H %D%/gkPlacement.H	\
%D%/gkFragment.H %D%/gkLibrary.H
