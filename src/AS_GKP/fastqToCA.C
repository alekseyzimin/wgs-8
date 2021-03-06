
/**************************************************************************
 * This file is part of Celera Assembler, a software program that
 * assembles whole-genome shotgun reads into contigs and scaffolds.
 * Copyright (C) 2010, J. CRaig Venter Institute.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received (LICENSE.txt) a copy of the GNU General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *************************************************************************/

const char *mainid = "$Id: fastqToCA.C 4479 2013-12-02 20:28:53Z brianwalenz $";

#include "AS_global.H"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "AS_UTL_fileIO.H"
#include "AS_PER_gkpStore.H"
#include "AS_MSG_pmesg.H"

#include <vector>

using namespace std;


void
addFeature(LibraryMesg *libMesg, const char *feature, const char *value) {
  int32 nf = libMesg->num_features;

  libMesg->features[nf] = (char *)safe_malloc(sizeof(char) * (strlen(feature) + 1));
  libMesg->values  [nf] = (char *)safe_malloc(sizeof(char) * (strlen(value) + 1));

  strcpy(libMesg->features[nf], feature);
  strcpy(libMesg->values[nf],   value);

  libMesg->num_features++;
}


int32
checkFiles(char **names, int32 namesLen) {
  int32  errors = 0;

  for (int32 i=0; i<namesLen; i++) {
    char   *f1  = names[i];
    char   *f2  = strrchr(names[i], ',');
    char    cwd[FILENAME_MAX];

    getcwd(cwd, FILENAME_MAX);

    if (f2) {
      *f2 = 0;
      f2++;
    }

    if ((f1) && (AS_UTL_fileExists(f1, FALSE, FALSE) == false))
      fprintf(stderr, "ERROR: fastq file '%s' doesn't exist.\n", f1), errors++;

    if ((f2) && (AS_UTL_fileExists(f2, FALSE, FALSE) == false))
      fprintf(stderr, "ERROR: fastq file '%s' doesn't exist.\n", f2), errors++;

    if ((f1) && (f1[0] != '/')) {
      char *n1 = new char [FILENAME_MAX];
      sprintf(n1, "%s/%s", cwd, f1);
      f1 = n1;

      if (AS_UTL_fileExists(f1, FALSE, FALSE) == false)
        fprintf(stderr, "ERROR: absolute-path fastq file '%s' doesn't exist.\n", f1), errors++;
    }

    if ((f2) && (f2[0] != '/')) {
      char *n2 = new char [FILENAME_MAX];
      sprintf(n2, "%s/%s", cwd, f2);
      f2 = n2;

      if (AS_UTL_fileExists(f2, FALSE, FALSE) == false)
        fprintf(stderr, "ERROR: absolute-path fastq file '%s' doesn't exist.\n", f2), errors++;
    }

    char *n = new char [FILENAME_MAX + FILENAME_MAX];

    if (f2)
      sprintf(n, "%s,%s", f1, f2);
    else
      sprintf(n, "%s", f1);

    names[i] = n;
  }

  return(errors);
}



int
main(int argc, char** argv) {
  int  insertSize         = 0;
  int  insertStdDev       = 0;
  bool constantInsertSize = false;

  const char *libraryName = NULL;

  bool isMated = false;

  const char *type         = "sanger";
  const char *technology   = "illumina";
  const char *orientInnie  = "innie";
  const char *orientOuttie = "outtie";
  const char *orient       = orientInnie;


  char **reads    = new char * [argc];
  int32        readsLen = 0;

  char **mates    = new char * [argc];
  int32        matesLen = 0;

  bool isNonRandom = false;

  vector<const char *> featureKeys;
  vector<const char *> featureVals;

  argc = AS_configure(argc, (const char**)argv);

  int arg = 1;
  int err = 0;
  while (arg < argc) {
    if        (strcmp(argv[arg], "-insertsize") == 0) {
      insertSize   = atoi(argv[++arg]);
      insertStdDev = atoi(argv[++arg]);
      isMated      = true;

      if ((arg+1 < argc) && (strcmp(argv[arg+1], "constant") == 0)) {
        constantInsertSize = true;
        arg++;
      }

    } else if (strcmp(argv[arg], "-libraryname") == 0) {
      libraryName = argv[++arg];

    } else if (strcmp(argv[arg], "-technology") == 0) {
       technology = argv[++arg];
     
    } else if (strcmp(argv[arg], "-type") == 0) {
      type = argv[++arg];

    } else if (strcmp(argv[arg], "-innie") == 0) {
      orient = orientInnie;

    } else if (strcmp(argv[arg], "-outtie") == 0) {
      orient = orientOuttie;

    } else if (strcmp(argv[arg], "-reads") == 0) {
      reads[readsLen++] = argv[++arg];

    } else if (strcmp(argv[arg], "-mates") == 0) {
      mates[matesLen++] = argv[++arg];

    } else if (strcmp(argv[arg], "-nonrandom") == 0) {
      isNonRandom = true;

    } else if (strcmp(argv[arg], "-feature") == 0) {
      featureKeys.push_back(argv[++arg]);
      featureVals.push_back(argv[++arg]);

    } else {
      fprintf(stderr, "ERROR:  Unknown option '%s'\n", argv[arg]);
      exit(1);
      err++;
    }

    arg++;
  }

  if ((insertSize == 0) && (insertStdDev >  0))
    err++;
  if ((insertSize >  0) && (insertStdDev == 0))
    err++;
  if (libraryName == 0L)
    err++;
  if ((strcasecmp(technology, "sanger") != 0) &&
      (strcasecmp(technology, "454") != 0) &&
      (strcasecmp(technology, "illumina") != 0) &&
      (strcasecmp(technology, "illumina-long") != 0) &&
      (strcasecmp(technology, "moleculo") != 0) &&
      (strcasecmp(technology, "pacbio-ccs") != 0) &&
      (strcasecmp(technology, "pacbio-corrected") != 0) &&
      (strcasecmp(technology, "pacbio-raw") != 0) &&
      (strcasecmp(technology, "none") != 0))
     err++;
  if ((strcasecmp(type, "sanger") != 0) &&
      (strcasecmp(type, "solexa") != 0) &&
      (strcasecmp(type, "illumina") != 0))
    err++;
  if (readsLen + matesLen == 0)
    err++;
  if ((isMated == false) && (matesLen > 0))
    err++;
  if (err) {
    fprintf(stderr, "usage: %s [-insertsize <mean> <stddev>] [-libraryname <name>]\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "  -insertsize i d    Mates are on average i +- d bp apart.\n");
    fprintf(stderr, "                     If the word 'constant' follows the insert size, no changes will be\n");
    fprintf(stderr, "                     made to the insert size.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -libraryname n     The UID of the library these reads are added to.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -technology p      What instrument were these reads generated on ('illumina' is the default):\n");
    fprintf(stderr, "                       'none'               -- don't set any features; use -feature to set them manually\n");
    fprintf(stderr, "                       'sanger'             -- reads from dideoxy sequencers\n");
    fprintf(stderr, "                       '454'                -- reads from 454 Life Sciences; FLX, Titanium, FLX+\n");
    fprintf(stderr, "                       'illumina'           -- reads from Illumina; GAIIx, MiSeq, HiSeq; shorter than 160bp\n");
    fprintf(stderr, "                       'illumina-long'      -- reads from Illumina; GAIIx, MiSeq, HiSeq; any length\n");
    fprintf(stderr, "                       'moleculo'           -- reads from Illumina; Moleculo\n");
    fprintf(stderr, "                       'pacbio-ccs'         -- reads from PacBio; Circular Consensus Sequence (CSS)\n");
    fprintf(stderr, "                       'pacbio-corrected'   -- reads from PacBio; corrected reads from pacBioToCA\n");
    fprintf(stderr, "                       'pacbio-raw'         -- reads from PacBio; uncorrected reads\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -type t            What type of fastq ('sanger' is the default):\n");
    fprintf(stderr, "                       'sanger'   -- QV's are PHRED, offset=33 '!', NCBI SRA data.\n");
    fprintf(stderr, "                       'solexa'   -- QV's are Solexa, early Solexa data.\n");
    fprintf(stderr, "                       'illumina' -- QV's are PHRED, offset=64 '@', Illumina reads from version 1.3 on.\n");
    fprintf(stderr, "                     See Cock, et al., 'The Sanger FASTQ file format for sequences with quality scores, and\n");
    fprintf(stderr, "                     the Solexa/Illumina FASTQ variants', doi:10.1093/nar/gkp1137\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -innie             The paired end reads are 5'-3' <-> 3'-5' (the usual case) (default)\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -outtie            The paired end reads are 3'-5' <-> 5'-3' (for Illumina Mate Pair reads)\n");
    fprintf(stderr, "                     This switch will reverse-complement every read, transforming outtie-oriented\n");
    fprintf(stderr, "                     mates into innie-oriented mates.  This trick only works if all reads are the\n");
    fprintf(stderr, "                     same length.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -reads A           Single ended reads, in fastq format.\n");
    fprintf(stderr, "  -mates A           Mated reads, interlaced, in fastq format.\n");
    fprintf(stderr, "  -mates A,B         Mated reads, in fastq format.\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Library Features\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -nonrandom         Mark the library as containing non-random reads.\n");
    fprintf(stderr, "  -feature F V       Set feature F to value V.\n");
    fprintf(stderr, "\n");

    if ((insertSize == 0) && (insertStdDev >  0))
      fprintf(stderr, "ERROR:  Invalid -insertsize.  Both mean and std.dev must be greater than zero.\n");
    if ((insertSize >  0) && (insertStdDev == 0))
      fprintf(stderr, "ERROR:  Invalid -insertsize.  Both mean and std.dev must be greater than zero.\n");

    if (libraryName == 0L)
      fprintf(stderr, "ERROR:  No library name supplied with -libraryname.\n");

    if ((strcasecmp(technology, "sanger") != 0) &&
        (strcasecmp(technology, "454") != 0) &&
        (strcasecmp(technology, "illumina") != 0) &&
        (strcasecmp(technology, "moleculo") != 0) &&
        (strcasecmp(technology, "pacbio-ccs") != 0) &&
        (strcasecmp(technology, "pacbio-corrected") != 0) &&
        (strcasecmp(technology, "pacbio-raw") != 0) &&
        (strcasecmp(technology, "none") != 0))
      fprintf(stderr, "ERROR:  Invalid technology '%s' supplied with -technology.\n", technology);

    if ((strcasecmp(type, "sanger") != 0) &&
        (strcasecmp(type, "solexa") != 0) &&
        (strcasecmp(type, "illumina") != 0))
      fprintf(stderr, "ERROR:  Invalid type '%s' supplied with -type.\n", type);

    if (readsLen + matesLen == 0)
      fprintf(stderr, "ERROR:  No reads supplied with -reads or -mates.\n");

    if ((isMated == false) && (matesLen > 0))
      fprintf(stderr, "ERROR:  Mated reads (-mates) must have am insert size (-insertsize).\n");

    exit(1);
  }

  //  Check that all the read files exist, and that the paths are absolute.

  int32   fastqPathErrors = 0;

  fastqPathErrors += checkFiles(reads, readsLen);
  fastqPathErrors += checkFiles(mates, matesLen);

  if (fastqPathErrors)
    fprintf(stderr, "ERROR: some fastq files not found.\n"), exit(1);

  //  Construct the library

  gkLibrary         gkl;

  gkl.libraryUID                 = AS_UID_load(libraryName);

  strcpy(gkl.libraryName, libraryName);

  gkl.mean                       = insertSize;
  gkl.stddev                     = insertStdDev;

  if        (strcasecmp(technology, "sanger") == 0) {
    gkl.forceBOGunitigger          = 0;
    gkl.doNotTrustHomopolymerRuns  = 0;

    gkl.doTrim_initialNone         = 0;
    gkl.doTrim_initialMerBased     = 0;
    gkl.doTrim_initialFlowBased    = 0;
    gkl.doTrim_initialQualityBased = 1;

    gkl.doRemoveDuplicateReads     = 0;

    gkl.doTrim_finalLargestCovered = 0;
    gkl.doTrim_finalEvidenceBased  = 1;

    gkl.doRemoveSpurReads          = 1;
    gkl.doRemoveChimericReads      = 1;
    gkl.doCheckForSubReads         = 0;

    gkl.doConsensusCorrection      = 0;

    gkl.forceShortReadFormat       = 0;

  } else if (strcasecmp(technology, "454") == 0) {
    gkl.forceBOGunitigger          = 1;
    gkl.doNotTrustHomopolymerRuns  = 1;

    gkl.doTrim_initialNone         = 0;
    gkl.doTrim_initialMerBased     = 0;
    gkl.doTrim_initialFlowBased    = 1;
    gkl.doTrim_initialQualityBased = 0;

    gkl.doRemoveDuplicateReads     = 1;

    gkl.doTrim_finalLargestCovered = 1;
    gkl.doTrim_finalEvidenceBased  = 0;

    gkl.doRemoveSpurReads          = 1;
    gkl.doRemoveChimericReads      = 1;
    gkl.doCheckForSubReads         = 0;

    gkl.doConsensusCorrection      = 0;

    gkl.forceShortReadFormat       = 0;

  } else if (strcasecmp(technology, "illumina") == 0) {
    gkl.forceBOGunitigger          = 1;
    gkl.doNotTrustHomopolymerRuns  = 0;

    gkl.doTrim_initialNone         = 0;
    gkl.doTrim_initialMerBased     = 1;
    gkl.doTrim_initialFlowBased    = 0;
    gkl.doTrim_initialQualityBased = 0;

    gkl.doRemoveDuplicateReads     = 1;

    gkl.doTrim_finalLargestCovered = 1;
    gkl.doTrim_finalEvidenceBased  = 0;

    gkl.doRemoveSpurReads          = 1;
    gkl.doRemoveChimericReads      = 1;
    gkl.doCheckForSubReads         = 0;

    gkl.doConsensusCorrection      = 0;

    gkl.forceShortReadFormat       = 1;

  } else if (strcasecmp(technology, "illumina-long") == 0) {
    gkl.forceBOGunitigger          = 1;
    gkl.doNotTrustHomopolymerRuns  = 0;

    gkl.doTrim_initialNone         = 0;
    gkl.doTrim_initialMerBased     = 1;
    gkl.doTrim_initialFlowBased    = 0;
    gkl.doTrim_initialQualityBased = 0;

    gkl.doRemoveDuplicateReads     = 1;

    gkl.doTrim_finalLargestCovered = 1;
    gkl.doTrim_finalEvidenceBased  = 0;

    gkl.doRemoveSpurReads          = 1;
    gkl.doRemoveChimericReads      = 1;
    gkl.doCheckForSubReads         = 0;

    gkl.doConsensusCorrection      = 0;

    gkl.forceShortReadFormat       = 0;

  } else if (strcasecmp(technology, "moleculo") == 0) {
    gkl.forceBOGunitigger          = 1;
    gkl.doNotTrustHomopolymerRuns  = 0;

    gkl.doTrim_initialNone         = 1;
    gkl.doTrim_initialMerBased     = 0;
    gkl.doTrim_initialFlowBased    = 0;
    gkl.doTrim_initialQualityBased = 0;

    gkl.doRemoveDuplicateReads     = 0;

    gkl.doTrim_finalLargestCovered = 1;
    gkl.doTrim_finalEvidenceBased  = 0;

    gkl.doRemoveSpurReads          = 1;
    gkl.doRemoveChimericReads      = 1;
    gkl.doCheckForSubReads         = 0;

    gkl.doConsensusCorrection      = 0;

    gkl.forceShortReadFormat       = 0;

  } else if (strcasecmp(technology, "pacbio-ccs") == 0) {
    gkl.forceBOGunitigger          = 1;
    gkl.doNotTrustHomopolymerRuns  = 0;

    gkl.doTrim_initialNone         = 1;
    gkl.doTrim_initialMerBased     = 0;
    gkl.doTrim_initialFlowBased    = 0;
    gkl.doTrim_initialQualityBased = 0;

    gkl.doRemoveDuplicateReads     = 1;

    gkl.doTrim_finalLargestCovered = 1;
    gkl.doTrim_finalEvidenceBased  = 0;

    gkl.doRemoveSpurReads          = 1;
    gkl.doRemoveChimericReads      = 1;
    gkl.doCheckForSubReads         = 0;

    gkl.doConsensusCorrection      = 0;
   
    gkl.forceShortReadFormat       = 0;

  } else if (strcasecmp(technology, "pacbio-corrected") == 0) {
    gkl.forceBOGunitigger          = 1;
    gkl.doNotTrustHomopolymerRuns  = 0;

    gkl.doTrim_initialNone         = 1;
    gkl.doTrim_initialMerBased     = 0;
    gkl.doTrim_initialFlowBased    = 0;
    gkl.doTrim_initialQualityBased = 0;

    gkl.doRemoveDuplicateReads     = 1;

    gkl.doTrim_finalLargestCovered = 1;
    gkl.doTrim_finalEvidenceBased  = 0;

    gkl.doRemoveSpurReads          = 1;
    gkl.doRemoveChimericReads      = 1;
    gkl.doCheckForSubReads         = 0;

    gkl.doConsensusCorrection      = 0;
    
    gkl.forceShortReadFormat       = 0;

  } else if (strcasecmp(technology, "pacbio-raw") == 0) {
    gkl.forceBOGunitigger          = 1;
    gkl.doNotTrustHomopolymerRuns  = 0;

    gkl.doTrim_initialNone         = 1;
    gkl.doTrim_initialMerBased     = 0;
    gkl.doTrim_initialFlowBased    = 0;
    gkl.doTrim_initialQualityBased = 0;

    gkl.doRemoveDuplicateReads     = 0;

    gkl.doTrim_finalLargestCovered = 1;
    gkl.doTrim_finalEvidenceBased  = 0;

    gkl.doRemoveSpurReads          = 1;
    gkl.doRemoveChimericReads      = 1;
    gkl.doCheckForSubReads         = 1;

    gkl.doConsensusCorrection      = 0;
    
    gkl.forceShortReadFormat       = 0;

  } else if (strcasecmp(technology, "none") == 0) {
    gkl.forceBOGunitigger          = 0;
    gkl.doNotTrustHomopolymerRuns  = 0;

    gkl.doTrim_initialNone         = 0;
    gkl.doTrim_initialMerBased     = 0;
    gkl.doTrim_initialFlowBased    = 0;
    gkl.doTrim_initialQualityBased = 0;

    gkl.doRemoveDuplicateReads     = 0;

    gkl.doTrim_finalLargestCovered = 0;
    gkl.doTrim_finalEvidenceBased  = 0;

    gkl.doRemoveSpurReads          = 0;
    gkl.doRemoveChimericReads      = 0;
    gkl.doCheckForSubReads         = 0;

    gkl.doConsensusCorrection      = 0;
    
    gkl.forceShortReadFormat       = 0;
  }

  gkl.isNotRandom                = isNonRandom;
  gkl.orientation                = (isMated) ? AS_READ_ORIENT_INNIE : AS_READ_ORIENT_UNKNOWN;

  gkl.constantInsertSize         = constantInsertSize;


  for (uint32 ff=0; ff<featureKeys.size(); ff++)
    gkl.gkLibrary_setFeature(featureKeys[ff], featureVals[ff]);


  //  Construct the messages.

  VersionMesg       vr2Mesg;
  VersionMesg       vr1Mesg;
  LibraryMesg       libMesg;

  vr2Mesg.version      = 2;

  vr1Mesg.version      = 1;

  libMesg.action       = AS_ADD;
  libMesg.eaccession   = gkl.libraryUID;
  libMesg.mean         = gkl.mean;
  libMesg.stddev       = gkl.stddev;
  libMesg.source       = NULL;

  libMesg.link_orient.setIsUnknown();

  switch(gkl.orientation) {
    case AS_READ_ORIENT_INNIE:
      libMesg.link_orient.setIsInnie();
      break;
    case AS_READ_ORIENT_OUTTIE:
      libMesg.link_orient.setIsOuttie();
      break;
    case AS_READ_ORIENT_NORMAL:
      libMesg.link_orient.setIsNormal();
      break;
    case AS_READ_ORIENT_ANTINORMAL:
      libMesg.link_orient.setIsAnti();
      break;
    case AS_READ_ORIENT_UNKNOWN:
      libMesg.link_orient.setIsUnknown();
      break;
    default:
      //  Cannot happen, unless someone adds a new orientation to gkFragment.
      assert(0);
      break;
  }

  gkl.gkLibrary_encodeFeatures(&libMesg);

  //  Add in the non-standard Illumina features.

  libMesg.features = (char **)safe_realloc(libMesg.features, sizeof(char *) * (libMesg.num_features + 2 + readsLen + matesLen));
  libMesg.values   = (char **)safe_realloc(libMesg.values,   sizeof(char *) * (libMesg.num_features + 2 + readsLen + matesLen));

  addFeature(&libMesg, "fastqQualityValues", type);
  addFeature(&libMesg, "fastqOrientation", orient);

  for (int32 i=0; i<readsLen; i++)
    addFeature(&libMesg, "fastqReads", reads[i]);

  for (int32 i=0; i<matesLen; i++)
    addFeature(&libMesg, "fastqMates", mates[i]);

  //  Emit the VER and LIB messages.  Enable version 2, write the LIB, switch back to version 1.

  GenericMesg       pmesg;

  pmesg.m = &vr2Mesg;
  pmesg.t = MESG_VER;
  WriteProtoMesg_AS(stdout, &pmesg);

  pmesg.m = &libMesg;
  pmesg.t = MESG_LIB;
  WriteProtoMesg_AS(stdout, &pmesg);

  pmesg.m = &vr1Mesg;
  pmesg.t = MESG_VER;
  WriteProtoMesg_AS(stdout, &pmesg);

  //  Clean up.

  gkl.gkLibrary_encodeFeaturesCleanup(&libMesg);

  delete [] reads;
  delete [] mates;

  exit(0);
}

