
/**************************************************************************
 * This file is part of Celera Assembler, a software program that
 * assembles whole-genome shotgun reads into contigs and scaffolds.
 * Copyright (C) 2006-2007, J. Craig Venter Institute
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

const char *mainid = "$Id: overmerry.C 4523 2014-04-11 20:07:55Z brianwalenz $";

#include "AS_global.H"
#include "AS_PER_gkpStore.H"
#include "AS_OVS_overlapStore.H"

#include <algorithm>

#include "AS_MER_gkpStore_to_FastABase.H"

#include "bio++.H"
#include "sweatShop.H"
#include "positionDB.H"
#include "libmeryl.H"


#define MAX_COUNT_GLOBAL     0x01
#define MAX_COUNT_FRAG_MEAN  0x02
#define MAX_COUNT_FRAG_MODE  0x03



//  Instead of using the intern<al overlap, which has enough extra stuff in it that we cannot store a
//  sequence iid for the table sequence and have it be small, we make our own overlap structure.

class kmerhit {
public:
  union {
    uint64  num;

    struct {
      uint64   tseq:30;              //  sequence in the table
      uint64   tpos:AS_OVS_POSBITS;  //  position in that sequence
      uint64   qpos:AS_OVS_POSBITS;  //  position in the query sequence
      uint64   cnt:8;                //  count of the kmer
      uint64   pal:1;                //  palindromic ; 0 = nope,    1 = yup
      uint64   fwd:1;                //  orientation ; 0 = reverse, 1 = forward
      uint64   pad:2;
    } val;
  } dat;

  void  setInteger(uint64 tseq, uint64 cnt, uint64 qpos, uint64 tpos, uint64 pal, uint64 fwd) {

    //  Threshold cnt to the maximum allowed.
    if (cnt > 255)
      cnt = 255;

    dat.val.tseq = tseq;
    dat.val.cnt  = cnt;
    dat.val.qpos = qpos;
    dat.val.tpos = tpos;
    dat.val.pal  = pal;
    dat.val.fwd  = fwd;
  };

  void  unpackInteger(void) {
  };


  bool  operator<(kmerhit const that) const {
    if (dat.val.tseq != that.dat.val.tseq)
      return(dat.val.tseq < that.dat.val.tseq);
    if (dat.val.cnt  != that.dat.val.cnt)
      return(dat.val.cnt  < that.dat.val.cnt);
    if (dat.val.qpos != that.dat.val.qpos)
      return(dat.val.qpos < that.dat.val.qpos);
    if (dat.val.tpos != that.dat.val.tpos)
      return(dat.val.tpos < that.dat.val.tpos);
    return(false);
  };
};



class ovmGlobalData {
public:
  ovmGlobalData() {
    gkpPath        = 0L;
    merCountsFile  = 0L;
    merSize        = 23;
    compression    = 1;
    maxCountGlobal = 1024 * 1024 * 1024;
    maxCountType   = MAX_COUNT_GLOBAL;
    numThreads     = 4;
    beVerbose      = false;

    qGK  = 0L;
    qFS  = 0L;
    qBeg = 1;
    qEnd = 0;

    tSS  = 0L;
    tMS  = 0L;
    tPS  = 0L;
    tBeg = 1;
    tEnd = 0;

    table_clrBeg       = 0L;
    table_untrimLength = 0L;

    outputName = 0L;
    outputFile = 0L;
  };

  ~ovmGlobalData() {

    //fprintf(stderr, "Found " F_U64 " mer hits.\n", merfound);
    //fprintf(stderr, "Found " F_U64 " overlaps.\n", ovlfound);

    AS_OVS_closeBinaryOverlapFile(outputFile);

    delete qFS;
    delete qGK;

    delete [] table_clrBeg;
    delete [] table_untrimLength;

    delete tPS;
    delete tMS;
    delete tSS;
  };

  void  build(void) {

    //
    //  Open the output file -- checks that we can actually create the
    //  output before we do any work.
    //
    outputFile = AS_OVS_createBinaryOverlapFile(outputName, FALSE);

    //
    //  Open inputs for the reader
    //
    qGK = new gkStore(gkpPath, FALSE, FALSE);

    //  Use that gkpStore to check and reset the desired ranges
    //
    uint32  mIID = qGK->gkStore_getNumFragments();

    if (qBeg < 1)     qBeg = 1;
    if (tBeg < 1)     tBeg = 1;

    if (qEnd == 0)    qEnd = mIID;
    if (qEnd > mIID)  qEnd = mIID;

    if (tEnd == 0)    tEnd = mIID;
    if (tEnd > mIID)  tEnd = mIID;

    if (qBeg < tBeg) {
      fprintf(stderr, "WARNING: reset -qb to -tb=" F_U32 "\n",
              tBeg);
      qBeg = tBeg;
    }

    if (qBeg >= qEnd) {
      fprintf(stderr, "ERROR: -qb=" F_U32 " and -qe=" F_U32 " are invalid (" F_U32 " frags in the store)\n",
              qBeg, qEnd, mIID);
      exit(1);
    }

    if (tBeg >= tEnd) {
      fprintf(stderr, "ERROR: -tb=" F_U32 " and -te=" F_U32 " are invalid (" F_U32 " frags in the store)\n",
              tBeg, tEnd, mIID);
      exit(1);
    }

    //  Use that gkpStore to quickly build a list of the clear ranges
    //  and full sequence length for reads in the table.

    {
      gkFragment   fr;
      gkStream    *fs = new gkStream(qGK, tBeg, tEnd, GKFRAGMENT_INF);

      table_clrBeg       = new uint32 [tEnd - tBeg + 1];
      table_untrimLength = new uint32 [tEnd - tBeg + 1];

      while (fs->next(&fr)) {
        AS_IID iid = fr.gkFragment_getReadIID();

        assert((tBeg <= iid) && (iid <= tEnd));

        table_clrBeg      [iid - tBeg] = fr.gkFragment_getClearRegionBegin();
        table_untrimLength[iid - tBeg] = fr.gkFragment_getSequenceLength();
      }

      delete fs;
    }

    //
    //  Open another fragStream for the query fragments.
    //

    qFS = new gkStream(qGK, qBeg, qEnd, GKFRAGMENT_SEQ);

    //
    //  Build state for the workers
    //

    merylStreamReader *MF = 0L;
    if (merCountsFile)
      MF = new merylStreamReader(merCountsFile);

    char     gkpName[FILENAME_MAX + 64] = {0};
    sprintf(gkpName, "%s:%u-%u:latest", gkpPath, tBeg, tEnd);

    tSS = new seqStream(gkpName);

    tSS->tradeSpaceForTime();

    tMS = new merStream(new kMerBuilder(merSize, compression, 0L), tSS, true, false);
    tPS = new positionDB(tMS, merSize, 0, 0L, 0L, MF, 0, 0, 0, 0, beVerbose);

    //  Filter out single copy mers, and mers too high...but ONLY if
    //  there is a merCountsFile.  In particular, the single copy mers
    //  in a table without counts can be multi-copy when combined with
    //  their reverse-complement mer.
    //
    if (MF)
      tPS->filter(2, maxCountGlobal);

    delete MF;
  };

  uint32    getClrBeg(AS_IID iid) {
    assert(tBeg <= iid);
    return(table_clrBeg[iid - tBeg]);
  };

  uint32    getUntrimLength(AS_IID iid) {
    assert(tBeg <= iid);
    return(table_untrimLength[iid - tBeg]);
  };


  //  Command line parameters
  //
  const char *gkpPath;
  const char *merCountsFile;
  uint32      merSize;
  uint32      compression;
  uint32      maxCountGlobal;
  uint32      maxCountType;
  uint32      numThreads;
  bool        beVerbose;

  //  for the READER only
  //
  gkStore           *qGK;
  gkStream          *qFS;
  uint32             qBeg;
  uint32             qEnd;

  //  for the WORKERS.
  //
  seqStream         *tSS;  //  needs to be public so we can offset coords
  merStream         *tMS;
  positionDB        *tPS;  //  needs to be public!  (this is the main table)
  uint32             tBeg;
  uint32             tEnd;

  //  for the WORKERS - we need to know the clrBeg and full read
  //  length for stuff in the table, which we can't get from the
  //  READER gkp.
  //
  uint32            *table_clrBeg;
  uint32            *table_untrimLength;

  //  for the WRITER only.
  //
  const char        *outputName;
  BinaryOverlapFile *outputFile;
};



class ovmThreadData {
public:
  ovmThreadData(ovmGlobalData *g) {
    qKB      = new kMerBuilder(g->merSize, g->compression, 0L);

    posnF    = 0L;
    posnFMax = 0;
    posnFLen = 0;

    posnR    = 0L;
    posnRMax = 0;
    posnRLen = 0;

    hitsLen = 0;
    hitsMax = 0;
    hits    = 0L;

    merfound = 0;
    ovlfound = 0;
  };

  ~ovmThreadData() {
    delete    qKB;
    delete [] posnF;
    delete [] posnR;
    delete [] hits;
  };

  void
  addHit(seqStream   *SS,
         AS_IID       iid,
         uint64       qpos,
         uint64       pos,
         uint64       cnt,
         uint64       pal,
         uint64       fwd) {

    uint32  seq = SS->sequenceNumberOfPosition(pos);

    pos -= SS->startOf(seq);
    seq  = SS->IIDOf(seq);

    if (iid == seq)
      return;

    if (iid < seq)
      return;

    if (hitsLen >= hitsMax) {
      if (hitsMax == 0) {
        hitsMax = 1048576;  //  tiny, 8MB per thread
        hits    = new kmerhit [hitsMax];
      } else {
        hitsMax *= 2;
        kmerhit *h = new kmerhit [hitsMax];
        memcpy(h, hits, sizeof(kmerhit) * hitsLen);
        delete [] hits;
        hits = h;
      }
    }

    hits[hitsLen].setInteger(seq, cnt, qpos, pos, pal, fwd);

    hitsLen++;
    merfound++;
  };


  kMerBuilder  *qKB;

  uint64        posnFLen;
  uint64        posnFMax;
  uint64       *posnF;

  uint64        posnRLen;
  uint64        posnRMax;
  uint64       *posnR;

  uint32        hitsLen;
  uint32        hitsMax;
  kmerhit      *hits;

  uint64        merfound;
  uint64        ovlfound;
};



class ovmComputation {
public:
  ovmComputation(gkFragment *fr) {
    beg = fr->gkFragment_getClearRegionBegin();
    end = fr->gkFragment_getClearRegionEnd  ();
    tln = fr->gkFragment_getSequenceLength();

    iid = fr->gkFragment_getReadIID();
    uid = fr->gkFragment_getReadUID();

    memset(seq, 0, AS_READ_MAX_NORMAL_LEN);
    strcpy(seq, fr->gkFragment_getSequence());

    ovsLen = 0;
    ovsMax = 1024;
    ovs    = new OVSoverlap [ovsMax];
  };

  ~ovmComputation() {
    delete [] ovs;
  };

  void        addOverlap(OVSoverlap *overlap) {
    if (ovsLen >= ovsMax) {
      ovsMax *= 2;
      OVSoverlap *o = new OVSoverlap [ovsMax];
      memcpy(o, ovs, sizeof(OVSoverlap) * ovsLen);
      delete [] ovs;
      ovs = o;
    }
    ovs[ovsLen++] = *overlap;
  };

  void        writeOverlaps(BinaryOverlapFile *outputFile) {
    for (uint32 i=0; i<ovsLen; i++)
      AS_OVS_writeOverlap(outputFile, ovs + i);
  };

  uint32      beg;
  uint32      end;
  uint32      tln;

  AS_IID      iid;
  AS_UID      uid;

  char        seq[AS_READ_MAX_NORMAL_LEN+1];

  uint32      ovsLen;  //  Overlap Storage, waiting for output
  uint32      ovsMax;
  OVSoverlap *ovs;
};




uint32
ovmWorker_analyzeReadForThreshold(ovmGlobalData    *g,
                                  ovmThreadData    *t,
                                  ovmComputation   *s,
                                  merStream        *sMSTR,
                                  uint32           *sSPAN) {

  uint32  maxCount = 0;
  uint32  cnt = 0;
  uint32  ave = 0;

  while (sMSTR->nextMer()) {
    uint32  c = (uint32)g->tPS->countExact(sMSTR->theFMer());

    if (c > 1) {
      sSPAN[cnt]  = c;
      ave        += c;
      cnt++;
    }
  }

  uint32  minc = 0;  //  Min count observed
  uint32  maxc = 0;  //  Max count observed

  uint32  medi = 0;  //  Median count
  uint32  mean = 0;  //  Mean count;

  uint32  mode = 0;  //  Modal count
  uint32  mcnt = 0;  //  Times we saw the modal count

  uint32  mtmp = 0;  //  A temporary for counting the mode

  if (cnt > 0) {
    std::sort(sSPAN, sSPAN + cnt);

    minc = sSPAN[0];
    medi = sSPAN[cnt/2];
    maxc = sSPAN[cnt-1];
    mean = sSPAN[0];

    mode  = sSPAN[0];
    mcnt  = 1;

    mtmp = 1;

    for (uint32 i=1; i<cnt; i++) {
      mean += sSPAN[i];

      if (sSPAN[i] == sSPAN[i-1])
        mtmp++;
      else
        mtmp = 1;

      if (mcnt < mtmp) {
        mode = sSPAN[i-1];
        mcnt = mtmp;
      }
    }

    mean /= cnt;
  }

  fprintf(stderr, "FRAG %u MIN %u MED %u MAX %u MEAN %u MODE %u (%u) NEWMAX %u\n",
          s->iid, minc, medi, maxc, mean, mode, mcnt, maxCount);

  sMSTR->rewind();

  switch (g->maxCountType) {
    case MAX_COUNT_FRAG_MEAN:
      maxCount = mean;
      break;
    case MAX_COUNT_FRAG_MODE:
      maxCount = mode;
    default:
      maxCount = 0;
  }

  return(maxCount);
}



void
ovmWorker(void *G, void *T, void *S) {
  ovmGlobalData    *g = (ovmGlobalData  *)G;
  ovmThreadData    *t = (ovmThreadData  *)T;
  ovmComputation   *s = (ovmComputation *)S;

  OVSoverlap        overlap;

  merStream        *sMSTR  = new merStream(t->qKB,
                                           new seqStream(s->seq + s->beg, s->end - s->beg),
                                           false, true);
  uint32           *sSPAN  = new uint32 [s->end - s->beg];

  uint32            maxCountForFrag = g->maxCountGlobal;

  switch (g->maxCountType) {
    case MAX_COUNT_GLOBAL:
      maxCountForFrag = g->maxCountGlobal;
      break;

    case MAX_COUNT_FRAG_MEAN:
      maxCountForFrag = ovmWorker_analyzeReadForThreshold(g, t, s, sMSTR, sSPAN);
      break;

    case MAX_COUNT_FRAG_MODE:
      maxCountForFrag = ovmWorker_analyzeReadForThreshold(g, t, s, sMSTR, sSPAN);
      break;

    default:
      assert(0);
      break;
  }

  t->hitsLen  = 0;
  t->posnFLen = 0;
  t->posnRLen = 0;

  while (sMSTR->nextMer()) {
    uint64  qpos   = sMSTR->thePositionInSequence();
    uint64  fcount = 0;
    uint64  rcount = 0;
    uint64  tcount = 0;

    sSPAN[qpos] = sMSTR->theFMer().getMerSpan();
    assert(qpos <= s->end - s->beg);

    if (sMSTR->theFMer() == sMSTR->theRMer()) {
      g->tPS->getExact(sMSTR->theFMer(), t->posnF, t->posnFMax, t->posnFLen, fcount);

      if (fcount < maxCountForFrag) {
        for (uint32 i=0; i<t->posnFLen; i++)
          t->addHit(g->tSS, s->iid, qpos, t->posnF[i], fcount, 1, 0);
      }
    } else {
      g->tPS->getExact(sMSTR->theFMer(), t->posnF, t->posnFMax, t->posnFLen, fcount);
      g->tPS->getExact(sMSTR->theRMer(), t->posnR, t->posnRMax, t->posnRLen, rcount);

      //  If we don't have a mer counts file, then we need to add the f and r counts to get the
      //  canonical count.  If we do have the mer counts, it is assumed those counts are canonical,
      //  we just have to pick the biggest. -- we're guaranteed to find at most one mer here
      //  (palindromes are handled above).

      if (g->merCountsFile == 0L) {
        //  No mer counts file, the count for this mer is the sum of the forward and reverse mers in
        //  the table.
        tcount = fcount + rcount;

        assert(t->posnFLen == fcount);  //  Sanity.
        assert(t->posnRLen == rcount);

      } else {
        //  Mer counts file exists.  Both the F and R mers are set to the same value, but one or
        //  both might not exist in this table.  Pick the largest.

        tcount = (fcount > rcount) ? fcount : rcount;

        //  A quirk (well, actually a user error); if the count in the global file is LESS THAN the
        //  count in the table, it is likely that we are using the wrong global counts.  Detect
        //  that, print a warning that will fill the disk, and keep going.

        if (tcount < t->posnFLen + t->posnRLen) {
          char tmpstrf[65];
          char tmpstrr[65];

          fprintf(stderr, "WARNING:  mer '%s' ('%s') has count " F_U64 "," F_U64 " but posnLen " F_U64 "," F_U64 "\n",
                  sMSTR->theFMer().merToString(tmpstrf),
                  sMSTR->theRMer().merToString(tmpstrr),
                  fcount, rcount,
                  t->posnFLen, t->posnRLen);
          fprintf(stderr, "          This can be caused by using the wrong merCount file (-mc option) for this data.\n");
          fprintf(stderr, "          Using the local count for this mer.\n");

          tcount = t->posnFLen + t->posnRLen;
        }

        //  Check sanity - if both mers are present, the counts should be the same.

        if ((t->posnFLen > 0) && (t->posnRLen > 0))
          assert(fcount == rcount);
        if ((t->posnFLen > 0) || (t->posnRLen > 0))
          assert(tcount > 0);

        //  Check sanity - the count from the position table should be at most the global count.

        assert(tcount >= t->posnFLen);
        assert(tcount >= t->posnRLen);
        assert(tcount >= t->posnFLen + t->posnRLen);
      }


      if (tcount < maxCountForFrag) {
        for (uint32 i=0; i<t->posnFLen; i++)
          t->addHit(g->tSS, s->iid, qpos, t->posnF[i], tcount, 0, 1);
        for (uint32 i=0; i<t->posnRLen; i++)
          t->addHit(g->tSS, s->iid, qpos, t->posnR[i], tcount, 0, 0);
      }
    }
  }

  delete sMSTR;

  if (t->hitsLen == 0) {
    delete [] sSPAN;
    return;
  }


  //  We have all the hits for this frag.  Sort them by sequence (the
  //  other sequence), then pick out the one with the least count for
  //  each sequence.
  //
  //  STL sort is ~10% faster than C qsort().  The compare function in
  //  qsort() is the dominant user of CPU time.
  //
  std::sort(t->hits, t->hits + t->hitsLen);

  for (uint32 i=0; i<t->hitsLen; i++)
    t->hits[i].unpackInteger();

#if 0
  //  Debug, I guess.  Generates lots of output, since frags with
  //  big identical overlaps will have lots and lots of mers in
  //  common.
  //
  for (uint32 i=0; i<t->hitsLen; i++) {
    if (i != t->hitsLen) {
      fprintf(stderr, F_U32"\t" F_U64 "\t" F_U32 "\t" F_U64 "\t%c\t" F_U32 "\n",
              t->hits[i].dat.val.tseq, t->hits[i].dat.val.tpos,
              s->iid,  t->hits[i].dat.val.qpos,
              t->hits[i].dat.val.pal ? 'p' : (t->hits[i].dat.val.fwd ? 'f' : 'r'),
              t->hits[i].dat.val.cnt);
    }
  }
#endif


  //  Make sure all the bits we never touch are zero
  memset(&overlap, 0, sizeof(OVSoverlap));


  for (uint32 i=0; i<t->hitsLen; ) {
    //fprintf(stderr, "FILTER STARTS i=" F_U32 " tseq=" F_U64 " tpos=" F_U64 " qpos=" F_U64 "\n",
    //          i, t->hits[i].dat.val.tseq, t->hits[i].dat.val.tpos, t->hits[i].dat.val.qpos);

    //  By the definition of our sort, the least common mer is the
    //  first hit in the list for each pair of sequences.
    //
    t->ovlfound++;

    //  Adjust coords to be relative to whole read -- the table is
    //  built using only sequence in the clear.  The query is
    //  built starting at the begin of the clear.  Same effect for
    //  both just a different mechanism.
    //
    t->hits[i].dat.val.tpos += g->getClrBeg(t->hits[i].dat.val.tseq);
    t->hits[i].dat.val.qpos += s->beg;

    //  Reverse if needed -- we need to remember, from when we were
    //  grabbing mers, the length of the uncompressed mer -- that's
    //  the sSPAN; if we're not using compressed seeds, sSPAN ==
    //  g->merSize.  The [t->hits[i].qpos - s->beg] array index is
    //  simply the position in the trimmed read.
    //
    if (t->hits[i].dat.val.fwd == false)
      t->hits[i].dat.val.qpos = s->tln - t->hits[i].dat.val.qpos - sSPAN[t->hits[i].dat.val.qpos - s->beg];

    //  Save off the A vs B overlap
    //
    overlap.a_iid                      = t->hits[i].dat.val.tseq;
    overlap.b_iid                      = s->iid;

    overlap.dat.dat[0] = 0;
    overlap.dat.dat[1] = 0;
#if AS_OVS_NWORDS > 2
    overlap.dat.dat[2] = 0;
#endif

    overlap.dat.mer.compression_length = g->compression;
    overlap.dat.mer.fwd                = t->hits[i].dat.val.fwd;
    overlap.dat.mer.palindrome         = t->hits[i].dat.val.pal;
    overlap.dat.mer.a_pos              = t->hits[i].dat.val.tpos;
    overlap.dat.mer.b_pos              = t->hits[i].dat.val.qpos;
    overlap.dat.mer.k_count            = t->hits[i].dat.val.cnt;
    overlap.dat.mer.k_len              = g->merSize;
    overlap.dat.mer.type               = AS_OVS_TYPE_MER;
    s->addOverlap(&overlap);

    //  Save off the B vs A overlap
    //
    overlap.a_iid = s->iid;
    overlap.b_iid = t->hits[i].dat.val.tseq;

    if (overlap.dat.mer.fwd) {
      overlap.dat.mer.a_pos = t->hits[i].dat.val.qpos;
      overlap.dat.mer.b_pos = t->hits[i].dat.val.tpos;
    } else {
      uint32 othlen = g->getUntrimLength(t->hits[i].dat.val.tseq);

      //  The -1 is to back up to the last base in the mer.

      overlap.dat.mer.a_pos = s->tln - t->hits[i].dat.val.qpos - 1;
      overlap.dat.mer.b_pos = othlen - t->hits[i].dat.val.tpos - 1;
    }
    s->addOverlap(&overlap);

    //  Now, skip ahead until we find the next pair.
    //
    uint64  lastiid = t->hits[i].dat.val.tseq;
    while ((i < t->hitsLen) && (t->hits[i].dat.val.tseq == lastiid)) {
      //fprintf(stderr, "FILTER OUT i=" F_U32 " tseq=" F_U64 " tpos=" F_U64 " qpos=" F_U64 "\n",
      //        i, t->hits[i].dat.val.tseq, t->hits[i].dat.val.tpos, t->hits[i].dat.val.qpos);
      i++;
    }
  }  //  over all hits

  delete [] sSPAN;
}






void*
ovmReader(void *G) {
  static gkFragment  fr;  //  static only for performance

  do {
    if (((ovmGlobalData *)G)->qFS->next(&fr) == 0)
      return(0L);
  } while (fr.gkFragment_getIsDeleted());

  return(new ovmComputation(&fr));
}



void
ovmWriter(void *G, void *S) {
  ovmGlobalData    *g = (ovmGlobalData  *)G;
  ovmComputation   *s = (ovmComputation *)S;

  s->writeOverlaps(g->outputFile);

  delete s;
}



int
main(int argc, const char** argv) {
  ovmGlobalData  *g = new ovmGlobalData;

  //assert(sizeof(kmerhit) == 8);

  argc = AS_configure(argc, argv);

  int arg=1;
  int err=0;
  while (arg < argc) {
    if        (strcmp(argv[arg], "-g") == 0) {
      g->gkpPath = argv[++arg];

    } else if (strcmp(argv[arg], "-m") == 0) {
      g->merSize = atoi(argv[++arg]);
    } else if (strcmp(argv[arg], "-c") == 0) {
      g->compression = atoi(argv[++arg]);

    } else if (strcmp(argv[arg], "-T") == 0) {
      ++arg;

      //  Do NOT reset maxCountGlobal on the per frag types.  We might want to use the global to
      //  limit pathological cases.
      //
      //  Do NOT set maxCountType on the global setting.  This will let us do "-T mean -T 100" or
      //  "-T 100 -T mean".
      //
      if        (strcmp(argv[arg], "mean") == 0)
        g->maxCountType   = MAX_COUNT_FRAG_MEAN;
      else if (strcmp(argv[arg], "mode") == 0)
        g->maxCountType   = MAX_COUNT_FRAG_MODE;
      else
        g->maxCountGlobal = atoi(argv[arg]);

    } else if (strcmp(argv[arg], "-mc") == 0) {
      g->merCountsFile = argv[++arg];

    } else if (strcmp(argv[arg], "-t") == 0) {
      g->numThreads = atoi(argv[++arg]);

    } else if (strcmp(argv[arg], "-tb") == 0) {
      g->tBeg = atoi(argv[++arg]);
    } else if (strcmp(argv[arg], "-te") == 0) {
      g->tEnd = atoi(argv[++arg]);
    } else if (strcmp(argv[arg], "-qb") == 0) {
      g->qBeg = atoi(argv[++arg]);
    } else if (strcmp(argv[arg], "-qe") == 0) {
      g->qEnd = atoi(argv[++arg]);

    } else if (strcmp(argv[arg], "-v") == 0) {
      g->beVerbose = true;

    } else if (strcmp(argv[arg], "-o") == 0) {
      g->outputName = argv[++arg];

    } else {
      fprintf(stderr, "unknown option '%s'\n", argv[arg]);
      err++;
    }
    arg++;
  }
  if ((g->gkpPath == 0L) || (err)) {
    fprintf(stderr, "usage: %s [opts]\n", argv[0]);
    fprintf(stderr, "\n");
    fprintf(stderr, "  -g gkpStore     path to the gkpStore\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -m merSize      mer size in bases\n");
    fprintf(stderr, "  -c compression  compression level; homopolymer runs longer than this length\n");
    fprintf(stderr, "                    are compressed to exactly this length\n");
    fprintf(stderr, "  -T threshold    ignore mers occuring more than 'threshold' times\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -mc counts      file of mercounts\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -t numThreads   number of compute threads\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -tb m           hash table fragment IID range\n");
    fprintf(stderr, "  -te n           hash table fragment IID range\n");
    fprintf(stderr, "                    fragments with IID x, m <= x < n, are used for the hash table\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -qb M           query fragment IID range (must be >= -tb)\n");
    fprintf(stderr, "  -qe N           query fragment IID range\n");
    fprintf(stderr, "                    fragments with IID y, M <= y < N, are used for the queries\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -v              entertain the user with progress reports\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "  -o outputName   output written here\n");
    exit(1);
  }

  gkpStoreFile::registerFile();

  g->build();

  sweatShop *ss = new sweatShop(ovmReader, ovmWorker, ovmWriter);

  ss->setLoaderQueueSize(131072);
  ss->setWriterQueueSize(1024);

  ss->setNumberOfWorkers(g->numThreads);

  for (uint32 w=0; w<g->numThreads; w++)
    ss->setThreadData(w, new ovmThreadData(g));  //  these leak

  ss->run(g, g->beVerbose);  //  true == verbose

  delete g;

  fprintf(stderr, "\nSuccess!  Bye.\n");

  return(0);
}
