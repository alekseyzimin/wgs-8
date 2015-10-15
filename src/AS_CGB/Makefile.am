noinst_LIBRARIES += lib/libAS_CGB.a
lib_libAS_CGB_a_SOURCES = %D%/AS_CGB_histo.C

libCA_a_SOURCES += $(lib_libAS_CGB_a_SOURCES)

bin_PROGRAMS += bin/unitigger
AS_FGB_SRCS = %D%/AS_FGB_main.C %D%/AS_FGB_io.C
AS_OGB_SRCS = %D%/AS_CGB_histo.C %D%/AS_CGB_fga.C %D%/AS_CGB_fgb.C	\
%D%/AS_FGB_hanging_fragment.C %D%/AS_FGB_contained.C			\
%D%/AS_CGB_traversal.C %D%/AS_CGB_walk.C %D%/AS_CGB_edgemate.C
AS_CGB_SRCS = %D%/AS_CGB_main.C %D%/AS_CGB_cgb.C	\
%D%/AS_CGB_classify.C %D%/AS_CGB_chimeras.C
AS_CGA_SRCS = %D%/AS_CGB_cga.C
AS_BUB_SRCS = %D%/AS_CGB_Bubble.C %D%/AS_CGB_Bubble_Graph.C	\
%D%/AS_CGB_Bubble_VertexSet.C %D%/AS_CGB_Bubble_dfs.C		\
%D%/AS_CGB_Bubble_Popper.C %D%/AS_CGB_Bubble_PopperMethods.C
bin_unitigger_SOURCES = %D%/AS_CGB_unitigger.C $(AS_FGB_SRCS)		\
                        $(AS_OGB_SRCS) $(AS_CGB_SRCS) $(AS_CGA_SRCS)	\
                        $(AS_BUB_SRCS)

noinst_HEADERS += %D%/AS_CGB_Bubble_Graph.H %D%/AS_CGB_all.H	\
%D%/AS_CGB_Bubble_PopperMethods.H %D%/AS_CGB_histo.H		\
%D%/AS_CGB_unitigger_globals.H %D%/AS_CGB_Bubble_Popper.H	\
%D%/AS_CGB_Bubble.H %D%/AS_CGB_Bubble_VertexSet.H		\
%D%/AS_CGB_methods.H
