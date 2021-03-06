
/**************************************************************************
 * This file is part of Celera Assembler, a software program that
 * assembles whole-genome shotgun reads into contigs and scaffolds.
 * Copyright (C) 2007, J. Craig Venter Institute. All rights reserved.
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

static const char *rcsid = "$Id: overlapStore_erates.C 4371 2013-08-01 17:19:47Z brianwalenz $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#include "AS_global.H"
#include "AS_UTL_fileIO.H"
#include "AS_OVS_overlap.H"
#include "AS_OVS_overlapFile.H"
#include "AS_OVS_overlapStore.H"

#include "overlapStore.H"



void
updateErates(const char *storeName, const char *eratesName) {
  int             i;
  char            name[FILENAME_MAX];

  OverlapStore   *store = NULL;
  OverlapStore   *orig  = NULL;
  OVSoverlap      ovl;

  int32           iFirst;
  int32           iLast;
  uint64          iNum;

  FILE           *eF   = NULL;
  int             eLen = 0;
  int             eNum = 0;
  int             eMax = 32;
  uint16         *e    = NULL;

  //  Open the erates, read in the header ('defined' in CatEratesOVL.c
  //  and CorrectOlapsOVL.c).
  //
  errno = 0;
  eF = fopen(eratesName, "r");
  if (errno) {
    fprintf(stderr, "failed to open erates file '%s': %s\n", eratesName, strerror(errno));
    exit(1);
  }
  AS_UTL_safeRead(eF, &iFirst, "updateErates read header 0", sizeof(int32), 1);
  AS_UTL_safeRead(eF, &iLast,  "updateErates read header 1", sizeof(int32), 1);
  AS_UTL_safeRead(eF, &iNum,   "updateErates read header 2", sizeof(uint64), 1);

  //  Open the two stores so we can read overlaps from them.  The
  //  store we're merging into is opened as a "copy".  The store we
  //  merge from is opened first, so that if it fails, we haven't
  //  turned the original store into a backup.
  //
  orig = AS_OVS_openOverlapStorePrivate(storeName, TRUE,  TRUE);

  if (iNum != orig->ovs.numOverlapsTotal) {
    fprintf(stderr, "ERROR: iNum " F_U64 " != orig->ovs.numOverlapsTotal " F_U64 "\n",
            iNum, orig->ovs.numOverlapsTotal);

    // The backup gets nuked by the close call, restore to avoid
    // exiting with only backup store
    //
    AS_OVS_restoreBackup(orig);
    AS_OVS_closeOverlapStore(orig);

    exit(1);
  }

  //  Recreate a store in the same place as the original store.
  //
  store = AS_OVS_createOverlapStore(storeName, FALSE);

  //  Grab some space for our cache of erates
  e = (uint16 *)safe_malloc(sizeof(uint16) * eMax);

  while (AS_OVS_readOverlapFromStore(orig, &ovl, AS_OVS_TYPE_OVL)) {
    if (eLen >= eNum) {
      eLen = 0;
      eNum = AS_UTL_safeRead(eF, e, "updateErates read erates", sizeof(uint16), eMax);
      if (eNum == 0) {
        fprintf(stderr, "failed to read more erates from '%s'.\n", eratesName);
        exit(1);
      }
    }

    ovl.dat.ovl.corr_erate = e[eLen++];
    iNum--;

#if 0
    fprintf(stderr, "%d-vs-%d from %d(%6.3f) to %d(%6.3f) [eLen=%4d eNum=%4d eMax=%4d]\n",
            (int)ovl.a_iid,
            (int)ovl.b_iid,
            (int)ovl.dat.ovl.orig_erate, AS_OVS_decodeQuality(ovl.dat.ovl.orig_erate) * 100.0,
            (int)ovl.dat.ovl.corr_erate, AS_OVS_decodeQuality(ovl.dat.ovl.corr_erate) * 100.0,
            eLen, eNum, eMax);
#endif

    AS_OVS_writeOverlapToStore(store, &ovl);
  }

  //  check that both files are empty now

  assert(iNum == 0);


  //  ALL DONE!  Close the stores, nuke the backups and get outta here.
  //
  AS_OVS_closeOverlapStore(orig);
  AS_OVS_closeOverlapStore(store);

  exit(0);
}
