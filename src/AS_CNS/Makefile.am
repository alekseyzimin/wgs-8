noinst_LIBRARIES += lib/libAS_CNS.a
lib_libAS_CNS_a_SOURCES = %D%/MultiAlign.C				\
                          %D%/MultiAlignMatePairAnalysis.C		\
                          %D%/MultiAlignSizeAnalysis.C			\
                          %D%/MultiAlignPrint.C %D%/MultiAlignStore.C	\
                          %D%/MultiAlignment_CNS.C %D%/RefreshMANode.C	\
                          %D%/AbacusRefine.C %D%/ApplyAlignment.C	\
                          %D%/BaseCall.C %D%/GetAlignmentTrace.C	\
                          %D%/MergeMultiAligns.C %D%/MergeRefine.C	\
                          %D%/MultiAlignContig.C			\
                          %D%/MultiAlignUnitig.C %D%/PrintAlignment.C	\
                          %D%/ReplaceEndUnitigInContig.C

libCA_a_SOURCES += $(lib_libAS_CNS_a_SOURCES)

# TODO: do we need to support SEQ_AN
bin_PROGRAMS += bin/utgcns bin/utgcnsfix bin/ctgcns bin/tigStore bin/addReadsToUnitigs

bin_utgcns_SOURCES = %D%/utgcns.C
bin_utgcnsfix_SOURCES = %D%/utgcnsfix.C
bin_ctgcns_SOURCES = %D%/ctgcns.C
bin_tigStore_SOURCES = %D%/tigStore.C
bin_addReadsToUnitigs_SOURCES = %D%/addReadsToUnitigs.C
