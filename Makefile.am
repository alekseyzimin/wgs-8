dist_bin_SCRIPTS =
bin_PROGRAMS =
noinst_HEADERS =
AM_CPPFLAGS = -I$(srcdir)/src -I$(srcdir)/src/AS_UTL -I$(srcdir)/src/AS_MSG -I$(srcdir)/src/AS_OVS
AM_CPPFLAGS += -I$(srcdir)/src/AS_PER -I$(srcdir)/src/AS_REZ -I$(srcdir)/src/AS_CNS -I$(srcdir)/src/AS_ALN
AM_CPPFLAGS += -I$(srcdir)/src/AS_CGW -I$(srcdir)/src/AS_CGB -I$(srcdir)/src/AS_UID -I$(srcdir)/src/AS_GKP
AM_CPPFLAGS += -I$(srcdir)/src/AS_MER -I$(srcdir)/src/AS_OBT $(BOOST_CPPFLAGS)
AM_LDFLAGS = -lpthread -fopenmp -lz

# Kmer library includes
AM_CPPFLAGS += -I$(srcdir)/kmer/libutil -I$(srcdir)/kmer/libbio -I$(srcdir)/kmer/libseq -I$(srcdir)/kmer/libmeryl
AM_CPPFLAGS += -I$(srcdir)/kmer/meryl -I$(srcdir)/kmer/libkmer
KMERLIBS = lib/libmerylguts.a lib/libkmer.a lib/libmeryl.a	\
           lib/libseq.a lib/libbio.a lib/libutil.a

noinst_LIBRARIES = libCA.a
libCA_a_SOURCES =
LDADD = libCA.a

# Rule to "build" perl script. It really only remove the extension
# while copying. TODO: should the scripts be renamed in git withouth
# an extension?
%: %.pl
	$(AM_V_GEN)mkdir -p $(dir $@) && cp $< $@
%: %.$(LOCAL_NAME).bin
	$(AM_V_GEN)mkdir -p $(dir $@) && cp $< $@

# Load kmer Makefiles
include $(srcdir)/kmer/libmeryl/Makefile.am
include $(srcdir)/kmer/libkmer/Makefile.am
include $(srcdir)/kmer/libseq/Makefile.am
include $(srcdir)/kmer/libbio/Makefile.am
include $(srcdir)/kmer/libutil/Makefile.am
include $(srcdir)/kmer/meryl/Makefile.am

# Load CA Makefiles
include $(srcdir)/src/Makefile.am
include $(srcdir)/src/AS_RUN/Makefile.am
include $(srcdir)/src/AS_UTL/Makefile.am
include $(srcdir)/src/AS_UID/Makefile.am
include $(srcdir)/src/AS_MSG/Makefile.am
include $(srcdir)/src/AS_PER/Makefile.am
include $(srcdir)/src/AS_GKP/Makefile.am
include $(srcdir)/src/AS_OBT/Makefile.am
include $(srcdir)/src/AS_MER/Makefile.am
include $(srcdir)/src/AS_OVL/Makefile.am
include $(srcdir)/src/AS_OVM/Makefile.am
include $(srcdir)/src/AS_OVS/Makefile.am
include $(srcdir)/src/AS_ALN/Makefile.am
include $(srcdir)/src/AS_CGB/Makefile.am
include $(srcdir)/src/AS_BOG/Makefile.am
include $(srcdir)/src/AS_BAT/Makefile.am
include $(srcdir)/src/AS_PBR/Makefile.am
include $(srcdir)/src/AS_REZ/Makefile.am
include $(srcdir)/src/AS_CNS/Makefile.am
include $(srcdir)/src/AS_LIN/Makefile.am
include $(srcdir)/src/AS_CGW/Makefile.am
include $(srcdir)/src/AS_TER/Makefile.am
include $(srcdir)/src/AS_ENV/Makefile.am
include $(srcdir)/src/AS_REF/Makefile.am
