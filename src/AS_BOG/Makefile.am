bin_PROGRAMS += bin/buildUnitigs bin/fixUnitigs bin/analyzeBest

bin_buildUnitigs_SOURCES = %D%/BuildUnitigs.C				\
%D%/AS_BOG_BestOverlapGraph.C %D%/AS_BOG_Breaking.C			\
%D%/AS_BOG_ChunkGraph.C %D%/AS_BOG_EvaluateMates.C			\
%D%/AS_BOG_FragmentInfo.C %D%/AS_BOG_Instrumentation.C			\
%D%/AS_BOG_IntersectBubble.C %D%/AS_BOG_IntersectSplit.C		\
%D%/AS_BOG_InsertSizes.C %D%/AS_BOG_Joining.C %D%/AS_BOG_MateBubble.C	\
%D%/AS_BOG_MateChecker.C %D%/AS_BOG_MateLocation.C			\
%D%/AS_BOG_MoveContains.C %D%/AS_BOG_Outputs.C				\
%D%/AS_BOG_OverlapBubble.C %D%/AS_BOG_PlaceContains.C			\
%D%/AS_BOG_PlaceFragUsingOverlaps.C %D%/AS_BOG_PlaceZombies.C		\
%D%/AS_BOG_PopulateUnitig.C %D%/AS_BOG_SetParentAndHang.C		\
%D%/AS_BOG_SplitDiscontinuous.C %D%/AS_BOG_Unitig_AddAndPlaceFrag.C	\
%D%/AS_BOG_Unitig_AddFrag.C %D%/AS_BOG_Unitig_PlaceFragUsingEdges.C	\
%D%/AS_BOG_Unitig.C %D%/AS_BOG_UnitigGraph.C
bin_fixUnitigs_SOURCES = %D%/fixUnitigs.C
bin_analyzeBest_SOURCES = %D%/analyzeBest.C

noinst_HEADERS += %D%/AS_BOG_Unitig.H %D%/AS_BOG_MateLocation.H	\
%D%/AS_BOG_UnitigGraph.H %D%/AS_BOG_Datatypes.H			\
%D%/AS_BOG_ChunkGraph.H %D%/AS_BOG_BestOverlapGraph.H		\
%D%/AS_BOG_InsertSizes.H
