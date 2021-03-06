
/**************************************************************************
 * This file is part of Celera Assembler, a software program that
 * assembles whole-genome shotgun reads into contigs and scaffolds.
 * Copyright (C) 1999-2004, Applera Corporation. All rights reserved.
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

const char *mainid = "$Id: dumpCloneMiddles.C 4371 2013-08-01 17:19:47Z brianwalenz $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "AS_global.H"
#include "Instrument_CGW.H"

#define CMDIR "CloneMiddles"

static ScaffoldInstrumenter *si;

extern int do_draw_frags_in_CelamyScaffold;
extern int do_compute_missing_overlaps;
extern int do_surrogate_tracking;


void
dumpCloneMiddle(int isScaffold, int id) {
  char          camname[1000];
  FILE         *camfile = NULL;
  struct stat   sb;

  assert(isScaffold);

  CIScaffoldT *scaffold = GetGraphNode(ScaffoldGraph->ScaffoldGraph, id);

  if ((isDeadCIScaffoldT(scaffold)) ||
      (scaffold->type != REAL_SCAFFOLD))
    return;

  sprintf(camname, "%s/scf%d_cm.cam", CMDIR, scaffold->id);

  if (stat(camname, &sb) == 0) {
    fprintf(stderr, "Dumping clone middle for scaffold %d -- file exists, skipped!\n", id);
    return;
  }

  errno = 0;
  camfile = fopen(camname,"w");
  if (errno)
    fprintf(stderr, "Failed to open '%s': %s\n", camname, strerror(errno)), exit(1);

  fprintf(stderr, "Dumping clone middle for scaffold %d\n", id);

  DumpCelamyColors(camfile);
  DumpCelamyMateColors(camfile);

  if(do_draw_frags_in_CelamyScaffold)
    DumpCelamyFragColors(camfile);

  CelamyScaffold(camfile,scaffold,0,scaffold->bpLength.mean);

  InstrumentScaffold(ScaffoldGraph,
		     scaffold,
		     si,
		     InstrumenterVerbose2,
		     stderr);

  PrintScaffoldInstrumenterMateDetails(si,camfile,PRINTCELAMY);
  PrintExternalMateDetailsAndDists(ScaffoldGraph,si->bookkeeping.wExtMates,"\t",camfile,PRINTCELAMY);
  PrintUnmatedDetails(si,camfile,PRINTCELAMY);

  fclose(camfile);
}


void
usage(const char *pgm) {
  fprintf(stderr, "usage: %s -g <gkpStore> -o <ovlStore> -c <ckpName> -n <ckpNum> [other options]\n", pgm);
  fprintf(stderr, "  META OPTION\n");
  fprintf(stderr, "    -p <prefix>          -- attempt to guess all the required options, if your assembly\n");
  fprintf(stderr, "                            follows runCA-OBT naming conventions.\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "    -ctg                 -- dump contigs\n");
  fprintf(stderr, "    -scf                 -- dump scaffolds\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  REQUIRED OPTIONS\n");
  fprintf(stderr, "    -g <GatekeeperStoreName>\n");
  fprintf(stderr, "    -o <OVLStoreName>\n");
  fprintf(stderr, "    -c <CkptFileName>\n");
  fprintf(stderr, "    -n <CkpPtNum>\n");
  fprintf(stderr, "\n");
  fprintf(stderr, "  OPTIONAL OPTIONS\n");
  fprintf(stderr, "    -i <single IID>      -- generate a single contig or scaffold\n");
  fprintf(stderr, "    -l <min length>      -- generate only scaffolds larger than min length\n");
  fprintf(stderr, "    -S                   -- suppress surrogate fragment placement (possibly multiple placements per frg)\n");
}


int
main(int argc, const char** argv) {
  int ckptNum     = NULLINDEX;
  int id          = NULLINDEX;
  int specificScf = NULLINDEX;
  int minLen      = 0;
  int arg         = 1;
  int err         = 0;
  int firstScfArg = 0;

  argc = AS_configure(argc, argv);

  GlobalData = new Globals_CGW();

  while (arg < argc) {
    if        (strcmp(argv[arg], "-c") == 0) {
      strcpy(GlobalData->outputPrefix, argv[++arg]);

    } else if (strcmp(argv[arg], "-g") == 0) {
      strcpy(GlobalData->gkpStoreName, argv[++arg]);

    } else if (strcmp(argv[arg], "-l") == 0) {
      minLen = atoi(argv[++arg]);
      if (minLen <= 0) {
        fprintf(stderr, "error: min length -l must be greater than zero.\n");
        err = 1;
      }

    } else if (strcmp(argv[arg], "-n") == 0) {
      ckptNum = atoi(argv[++arg]);
      if (ckptNum <= 0) {
        fprintf(stderr, "error: checkpoint number -n must be greater than zero.\n");
        err = 1;
      }

    } else if (strcmp(argv[arg], "-o") == 0) {
      strcpy(GlobalData->ovlStoreName, argv[++arg]);

    } else if (strcmp(argv[arg], "-p") == 0) {
      ckptNum = GlobalData->setPrefix(argv[++arg]);

    } else if (strcmp(argv[arg], "-s") == 0) {
      specificScf = atoi(argv[++arg]);

    } else if (strcmp(argv[arg], "-S") == 0) {
      do_surrogate_tracking = 0;

    } else {
      if (atoi(argv[arg]) > 0) {
        firstScfArg = arg;
        break;
      } else {
        err = 1;
      }
    }

    arg++;
  }

  if ((GlobalData->outputPrefix[0] == 0) ||
      (GlobalData->gkpStoreName[0] == 0) ||
      (GlobalData->ovlStoreName[0] == 0)) {
    fprintf(stderr, "At least one of -c, -f, -g, -o not supplied.\n");
    fprintf(stderr, "'%s'\n", GlobalData->outputPrefix);
    fprintf(stderr, "'%s'\n", GlobalData->gkpStoreName);
    fprintf(stderr, "'%s'\n", GlobalData->ovlStoreName);
    err = 1;
  }

  if (err) {
    usage(argv[0]);
    exit(1);
  }

  errno=0;
  mkdir(CMDIR, S_IRWXU | S_IRWXG | S_IRWXO);

  LoadScaffoldGraphFromCheckpoint(GlobalData->outputPrefix, ckptNum, FALSE);

  ScaffoldGraph->frgOvlStore = AS_OVS_openOverlapStore(GlobalData->ovlStoreName);

  si = CreateScaffoldInstrumenter(ScaffoldGraph, INST_OPT_ALL);
  if (si == NULL) {
    fprintf(stderr, "Failed to CreateScaffoldInstrumenter().\n");
    exit(1);
  }

  do_draw_frags_in_CelamyScaffold = 0;
  do_compute_missing_overlaps     = 0;

  // over all scfs in graph

  if        (specificScf != NULLINDEX){
    dumpCloneMiddle(1, specificScf);

  } else if (firstScfArg > 0) {
    while (firstScfArg < argc) {
      id = atoi(argv[firstScfArg]);

      if (id > 0)
        dumpCloneMiddle(1, id);
      else
        fprintf(stderr, "WARNING: scaffold arg %d '%s' isn't numeric!\n", firstScfArg, argv[firstScfArg]);

      firstScfArg++;
    }

  } else {
    for (id = 0; id<GetNumGraphNodes(ScaffoldGraph->ScaffoldGraph); id++) {
      if (GetGraphNode(ScaffoldGraph->ScaffoldGraph, id)->bpLength.mean >= minLen)
        dumpCloneMiddle(1, id);
    }
  }

  DestroyScaffoldInstrumenter(si);
  AS_OVS_closeOverlapStore(ScaffoldGraph->frgOvlStore);

  delete GlobalData;

  exit(0);
}

