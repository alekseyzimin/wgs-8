# Don't need AS_PBR in MaSuRCA

#noinst_LIBRARIES += lib/libAS_PBR.a
#lib_libAS_PBR_a_SOURCES = %D%/AS_PBR_util.cc %D%/AS_PBR_store.cc	\
#                           %D%/AS_PBR_correct.cc %D%/AS_PBR_filter.cc	\
#                           %D%/AS_PBR_mates.cc %D%/AS_PBR_output.cc

#libCA_a_SOURCES += $(lib_libAS_PBR_a_SOURCES)

#bin_PROGRAMS += bin/outputLayout bin/trimFastqByQVWindow bin/correctPacBio bin/dumpPBRLayoutStore
#bin_outputLayout_SOURCES = %D%/OutputLayout.cc
#bin_trimFastqByQVWindow_SOURCES = %D%/TrimFastqByQVWindow.cc 
#bin_correctPacBio_SOURCES = %D%/CorrectPacBio.cc 
#bin_dumpPBRLayoutStore_SOURCES = %D%/AS_PBR_dumpStore.cc 
# TODO: Need to add support for SAMTOOLS. Maybe not!
# convertSamToCA:		%D%/ConvertSamToCA.cc $(SAM_LIBS)
bin/PBcR: %D%/PBcR.pl
	$(AM_V_GEN)cp $< $@
bin/convertToPBCNS: %D%/convertToPBCNS.pl
	$(AM_V_GEN)cp $< $@
bin/addCNSToStore: %D%/addCNSToStore.pl
	$(AM_V_GEN)cp $< $@
bin/pbdagcon: %D%/pbdagcon.Linux-amd64.bin
	$(AM_V_GEN)cp $< $@
bin/pbutgcns: %D%/pbutgcns.Linux-amd64.bin
	$(AM_V_GEN)cp $< $@
bin/falcon_sense: %D%/falcon_sense.Linux-amd64.bin
	$(AM_V_GEN)cp $< $@
bin/blasr: %D%/blasr.Linux-amd64.bin
	$(AM_V_GEN)cp $< $@

dist_bin_SCRIPTS +=  %D%/blasr.Linux-amd64.bin %D%/PBcR.pl  %D%/convertToPBCNS.pl %D%/addCNSToStore.pl %D%/pbdagcon.Linux-amd64.bin %D%/pbutgcns.Linux-amd64.bin %D%/falcon_sense.Linux-amd64.bin bin/blasr bin/PBcR bin/convertToPBCNS bin/addCNSToStore bin/pbdagcon bin/pbutgcns bin/falcon_sense

#noinst_HEADERS += %D%/AS_PBR_output.hh %D%/AS_PBR_mates.hh	\
#%D%/AS_PBR_util.hh %D%/AS_PBR_store.hh %D%/AS_PBR_correct.hh	\
#%D%/AS_PBR_filter.hh 
