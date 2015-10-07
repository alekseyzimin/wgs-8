noinst_LIBRARIES += lib/libAS_OBT.a
lib_libAS_OBT_a_SOURCES = %D%/AS_OBT_acceptableOverlap.C \
                          %D%/AS_OBT_overlaps.C

libCA_a_SOURCES += $(lib_libAS_OBT_a_SOURCES)

bin_PROGRAMS += bin/initialTrim bin/deduplicate bin/finalTrim	\
                bin/chimera bin/removeMateOverlap
bin_initialTrim_SOURCES = %D%/initialTrim.C %D%/trim.C
bin_deduplicate_SOURCES = %D%/deduplicate.C
bin_finalTrim_SOURCES = %D%/finalTrim.C %D%/trim.C %D%/finalTrim-largestCovered.C %D%/finalTrim-evidenceBased.C %D%/finalTrim-bestEdge.C
bin_chimera_SOURCES = %D%/chimera.C
bin_removeMateOverlap_SOURCES = %D%/removeMateOverlap.C

noinst_HEADERS += %D%/trim.H %D%/finalTrim.H		\
%D%/AS_OBT_acceptableOverlap.H %D%/AS_OBT_overlaps.H	\
%D%/finalTrim-consolidate.H

# TODO: is Figaro still supported?
