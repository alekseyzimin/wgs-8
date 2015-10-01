noinst_LIBRARIES += lib/libAS_MSG.a
lib_libAS_MSG_a_SOURCES = %D%/AS_MSG_pmesg.C %D%/AS_MSG_pmesg1.C %D%/AS_MSG_pmesg2.C

libCA_a_SOURCES += $(lib_libAS_MSG_a_SOURCES)

# fastaToCA and tracearchiveToCA are just aliases to other files
%D%/fastaToCA: %D%/convert-fasta-to-v2.pl
	$(AM_V_GEN)cp $< $@
%D%/tracearchiveToCA: %D%/tracedb-to-frg.pl
	$(AM_V_GEN)cp $< $@
dist_bin_SCRIPTS += %D%/convert-fasta-to-v2.pl %D%/tracedb-to-frg.pl %D%/fastaToCA %D%/tracearchiveToCA

bin_PROGRAMS += bin/remove_fragment bin/extractmessages
bin_remove_fragment_SOURCES = %D%/remove_fragment.C
bin_extractmessages_SOURCES = %D%/ExtractMessages.C

noinst_HEADERS += %D%/AS_MSG_types.H %D%/AS_MSG_pmesg.H	\
%D%/AS_MSG_pmesg_internal.H
