
# Perl scrip are simply copied!
%: %.pl
	$(AM_V_GEN)mkdir -p `dirname $@` && cp $< $@

# TODO: Missing the TIGR perl library
dist_bin_SCRIPTS += %D%/runCA %D%/runCA-dedupe				\
                    %D%/runCA-overlapStoreBuild %D%/asmQC/mergeqc.pl	\
                    %D%/asmQC/caqc.pl %D%/asmQC/caqc_help.ini		\
                    %D%/asmQC/ca2ace.pl %D%/asmQC/asmToAGP.pl		\
                    %D%/replaceUIDwithName-fastq				\
                    %D%/replaceUIDwithName-posmap

bin_PROGRAMS += bin/fragmentDepth bin/markUniqueUnique
bin_fragmentDepth_SOURCES = %D%/fragmentDepth.C
bin_markUniqueUnique_SOURCES = %D%/markUniqueUnique.C
