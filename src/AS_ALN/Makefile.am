noinst_LIBRARIES += lib/libAS_ALN.a
lib_libAS_ALN_a_SOURCES = %D%/AS_ALN_dpaligner.C		\
              %D%/AS_ALN_qvaligner.C %D%/AS_ALN_loverlapper.C	\
              %D%/AS_ALN_pieceOlap.C %D%/AS_ALN_forcns.C	\
              %D%/CA_ALN_local.C %D%/CA_ALN_overlap.C		\
              %D%/AS_ALN_bruteforcedp.C

libCA_a_SOURCES += $(lib_libAS_ALN_a_SOURCES)

noinst_HEADERS += %D%/AS_ALN_aligners.H %D%/AS_ALN_bruteforcedp.H	\
%D%/CA_ALN_local.H
