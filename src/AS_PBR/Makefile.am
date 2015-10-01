noinst_LIBRARIES += lib/libAS_PBR.a
lib_libAS_PBR_a_SOURCES = %D%/AS_PBR_util.cc %D%/AS_PBR_store.cc	\
                          %D%/AS_PBR_correct.cc %D%/AS_PBR_filter.cc	\
                          %D%/AS_PBR_mates.cc %D%/AS_PBR_output.cc

libCA_a_SOURCES += $(lib_libAS_PBR_a_SOURCES)

bin_PROGRAMS += bin/outputLayout bin/trimFastqByQVWindow bin/correctPacBio bin/dumpPBRLayoutStore
# Todo: Need to add support for SAMTOOLS

bin_outputLayout_SOURCES = %D%/OutputLayout.cc
bin_trimFastqByQVWindow_SOURCES = %D%/TrimFastqByQVWindow.cc
# convertSamToCA:		%D%/ConvertSamToCA.cc $(SAM_LIBS)
bin_correctPacBio_SOURCES = %D%/CorrectPacBio.cc
bin_dumpPBRLayoutStore_SOURCES = %D%/AS_PBR_dumpStore.cc
