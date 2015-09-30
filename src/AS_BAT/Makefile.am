bin_PROGRAMS += bin/bogart bin/bogus bin/bogusness bin/splitUnitigs	\
                bin/computeCoverageStat bin/markRepeatUnique		\
                bin/rewriteCache bin/erate-estimate

# TODO: Need kmer compilation for this last 3 execs
#bin/classifyMates	\
#                bin/classifyMatesApply bin/classifyMatesPairwise

bin_bogart_SOURCES = %D%/bogart.C %D%/AS_BAT_BestOverlapGraph.C		\
%D%/AS_BAT_Breaking.C %D%/AS_BAT_ChunkGraph.C				\
%D%/AS_BAT_EvaluateMates.C %D%/AS_BAT_ExtendByMates.C			\
%D%/AS_BAT_FragmentInfo.C %D%/AS_BAT_Instrumentation.C			\
%D%/AS_BAT_IntersectBubble.C %D%/AS_BAT_IntersectSplit.C		\
%D%/AS_BAT_InsertSizes.C %D%/AS_BAT_Joining.C %D%/AS_BAT_Logging.C	\
%D%/AS_BAT_MateBubble.C %D%/AS_BAT_MateChecker.C			\
%D%/AS_BAT_MateLocation.C %D%/AS_BAT_MergeSplitJoin.C			\
%D%/AS_BAT_MoveContains.C %D%/AS_BAT_Outputs.C				\
%D%/AS_BAT_OverlapCache.C %D%/AS_BAT_PlaceContains.C			\
%D%/AS_BAT_PlaceFragUsingOverlaps.C %D%/AS_BAT_PlaceZombies.C		\
%D%/AS_BAT_PopulateUnitig.C %D%/AS_BAT_PromoteToSingleton.C		\
%D%/AS_BAT_ReconstructRepeats.C %D%/AS_BAT_SetParentAndHang.C		\
%D%/AS_BAT_SplitDiscontinuous.C %D%/AS_BAT_Unitig_AddAndPlaceFrag.C	\
%D%/AS_BAT_Unitig_AddFrag.C %D%/AS_BAT_Unitig_PlaceFragUsingEdges.C	\
%D%/AS_BAT_Unitig.C
bin_bogus_SOURCES = %D%/bogus.C %D%/AS_BAT_bogusUtil.C
bin_bogusness_SOURCES = %D%/bogusness.C %D%/AS_BAT_bogusUtil.C
#bin_classifyMates_SOURCES = %D%/classifyMates.C         $(DNC_OBJECTS)  libutil.a
#bin_classifyMatesApply_SOURCES = %D%/classifyMatesApply.C    $(DNC_OBJECTS)  libutil.a
#bin_classifyMatesPairwise_SOURCES = %D%/classifyMatesPairwise.C $(DNC_OBJECTS)  libutil.a
bin_splitUnitigs_SOURCES = %D%/splitUnitigs.C                         
bin_computeCoverageStat_SOURCES = %D%/computeCoverageStat.C                  
bin_markRepeatUnique_SOURCES = %D%/markRepeatUnique.C                     
bin_rewriteCache_SOURCES = %D%/rewriteCache.C %D%/AS_BAT_FragmentInfo.C %D%/AS_BAT_Logging.C 
bin_erate_estimate_SOURCES = %D%/erate-estimate.C
