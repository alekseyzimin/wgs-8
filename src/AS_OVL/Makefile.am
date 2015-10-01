# TODO: do we need to support the UMD overlapper?

bin_PROGRAMS += bin/overlap_partition bin/olap-from-seeds		\
                bin/correct-frags bin/show-corrects bin/correct-olaps	\
                bin/cat-corrects bin/cat-erates

bin_show_corrects_SOURCES = %D%/ShowCorrectsOVL.C %D%/AS_OVL_delcher.C
#bin_update_erates_SOURCES = %D%/UpdateEratesOVL.C %D%/AS_OVL_delcher.C
#bin_force_erates_SOURCES = %D%/ForceEratesOVL.C  %D%/AS_OVL_delcher.C
bin_cat_corrects_SOURCES = %D%/CatCorrectsOVL.C  %D%/AS_OVL_delcher.C
bin_cat_erates_SOURCES = %D%/CatEratesOVL.C    %D%/AS_OVL_delcher.C
bin_correct_frags_SOURCES = %D%/FragCorrectOVL.C  %D%/AS_OVL_delcher.C
bin_correct_olaps_SOURCES = %D%/CorrectOlapsOVL.C %D%/AS_OVL_delcher.C
bin_overlap_partition_SOURCES = %D%/overlap_partition.C
bin_olap_from_seeds_SOURCES = %D%/OlapFromSeedsOVL.C %D%/AS_OVL_delcher.C %D%/SharedOVL.C

noinst_HEADERS += %D%/OlapFromSeedsOVL.H %D%/AS_OVL_delcher.H	\
%D%/FragCorrectOVL.H %D%/SharedOVL.H %D%/AS_OVL_olapstats.H
