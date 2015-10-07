bin_PROGRAMS += bin/greedyFragmentTiling			\
                bin/metagenomics_ovl_analyses bin/chimChe

bin_greedyFragmentTiling_SOURCES = %D%/greedyFragmentTiling.C
bin_metagenomics_ovl_analyses_SOURCES = %D%/metagenomics_ovl_analyses.C
bin_chimChe_SOURCES = %D%/chimChe.C

noinst_HEADERS += %D%/kmer_utils.H

dist_bin_SCRIPTS += %D%/greedy_layout_to_IUM %D%/utg2fasta	\
 %D%/run_greedy.csh

# TODO: do we want to check for seqan and distribute graph_consensus
# here?
