noinst_LIBRARIES += lib/libAS_CGW.a
lib_libAS_CGW_a_SOURCES = %D%/Globals_CGW.C %D%/CIEdgeT_CGW.C			\
                          %D%/CIScaffoldT_Analysis.C			\
                          %D%/CIScaffoldT_Biconnected_CGW.C			\
                          %D%/CIScaffoldT_CGW.C %D%/CIScaffoldT_Cleanup_CGW.C	\
                          %D%/CIScaffoldT_Merge_CGW.C			\
                          %D%/CIScaffoldT_Merge_Interleaved.C		\
                          %D%/CIScaffoldT_Merge_AlignScaffold.C		\
                          %D%/CIScaffoldT_MergeScaffolds.C %D%/Celamy_CGW.C	\
                          %D%/ChunkOverlap_CGW.C %D%/ContigT_CGW.C		\
                          %D%/DemoteUnitigsWithRBP_CGW.C			\
                          %D%/fragmentPlacement.C %D%/GraphCGW_T.C		\
                          %D%/GraphEdgeIterator.C %D%/Input_CGW.C		\
                          %D%/Instrument_CGW.C %D%/InterleavedMerging.C		\
                          %D%/LeastSquaresGaps_CGW.C			\
                          %D%/MarkInternalEdgeStatus.C %D%/MergeEdges_CGW.C	\
                          %D%/Output_CGW.C %D%/SEdgeT_CGW.C			\
                          %D%/ScaffoldGraph_CGW.C %D%/SplitScaffolds_CGW.C	\
                          %D%/Stats_CGW.C %D%/TransitiveReduction_CGW.C		\
                          %D%/ShatterScaffolds_CGW.C

libCA_a_SOURCES += $(lib_libAS_CGW_a_SOURCES)

bin_PROGRAMS += bin/cgw bin/cgwDump bin/analyzeScaffolds		\
                bin/extendClearRanges bin/extendClearRangesPartition	\
                bin/resolveSurrogates bin/dumpCloneMiddles		\
                bin/dumpSingletons bin/frgs2clones

bin_cgw_SOURCES = %D%/AS_CGW_main.C
bin_analyzeScaffolds_SOURCES = %D%/analyzeScaffolds.C
bin_extendClearRanges_SOURCES = %D%/eCR.C %D%/eCR-examineGap.C %D%/eCR-diagnostic.C
bin_extendClearRangesPartition_SOURCES = %D%/eCR-partition.C
bin_resolveSurrogates_SOURCES = %D%/resolveSurrogates.C
bin_dumpCloneMiddles_SOURCES = %D%/dumpCloneMiddles.C
bin_dumpSingletons_SOURCES = %D%/dumpSingletons.C
bin_frgs2clones_SOURCES = %D%/frgs2clones.C
bin_cgwDump_SOURCES = %D%/cgwDump.C

noinst_HEADERS += %D%/Output_CGW.H %D%/fixZLFContigs.H			\
%D%/fragmentPlacement.H %D%/GraphCGW_T.H %D%/Instrument_CGW.H		\
%D%/CIScaffoldT_Merge_AlignScaffold.H %D%/ChunkOverlap_CGW.H		\
%D%/GraphEdgeIterator.H %D%/InputDataTypes_CGW.H			\
%D%/ScaffoldGraphIterator_CGW.H %D%/CIScaffoldT_Analysis.H		\
%D%/ScaffoldGraph_CGW.H %D%/Stats_CGW.H %D%/AS_CGW_dataTypes.H		\
%D%/InterleavedMerging.H %D%/ChiSquareTest_CGW.H %D%/AS_CGW_histo.H	\
%D%/Globals_CGW.H %D%/CIScaffoldT_Merge_CGW.H %D%/Input_CGW.H		\
%D%/CIScaffoldT_MergeScaffolds.H %D%/eCR.H
