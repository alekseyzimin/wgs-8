
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

static const char *rcsid = "$Id: AS_CGB_Bubble_Popper.C 4371 2013-08-01 17:19:47Z brianwalenz $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AS_global.H"
#include "AS_ALN_aligners.H"
#include "AS_CGB_all.H"
#include "AS_CGB_methods.H"
#include "AS_CGB_Bubble_Graph.H"
#include "AS_CGB_Bubble.H"
#include "AS_CGB_Bubble_Popper.H"
#include "AS_CGB_Bubble_PopperMethods.H"
#include "AS_MSG_pmesg.H"
#include "AS_PER_gkpStore.H"

#undef AS_CGB_BUBBLE_VERBOSE2

extern int max_indel_AS_ALN_LOCOLAP_GLOBAL;

#define BP_SQR(x) ((x) * (x))

void
BP_init(BubblePopper_t bp, BubGraph_t bg, TChunkMesg *chunks,
	TChunkFrag cfrgs[], float global_arrival_rate,
        gkStore *gkpStore,
        const char * fileprefix)
{
  IntEdge_ID e;
  int i;

  memset((void *) bp, 0, sizeof(BubblePopper));	/* zero-out everything */

  for (e = 0; e < GetNumEdges(BG_edges(bg)); ++e)
    BG_E_setFlag(bg, e, AS_CGB_BUBBLE_E_UNUSED);

  bp->bg = bg;
  bp->chunks = chunks;
  bp->chunkFrags = cfrgs;
  bp->globalArrivalRate = global_arrival_rate;
  bp->gkpStore = gkpStore;

  bp->vidToBid     = (int *)safe_malloc(sizeof(int) * GetNumFragments(BG_vertices(bg)));

  bp->topDistArray = (int *)safe_malloc(sizeof(int) * POPPER_MAX_BUBBLE_SIZE);
  bp->dfsStack     = (BG_E_Iter *)safe_malloc(sizeof(BG_E_Iter) * POPPER_MAX_BUBBLE_SIZE);
  bp->bubFrags     = (IntFragment_ID *)safe_malloc(sizeof(IntFragment_ID) * POPPER_MAX_BUBBLE_SIZE);
  bp->bubMesgs     = (InternalFragMesg *)safe_calloc(sizeof(InternalFragMesg), POPPER_MAX_BUBBLE_SIZE);
  bp->adj          = (int *)safe_malloc(sizeof(int) * BP_SQR(POPPER_MAX_BUBBLE_SIZE));
  bp->bubOlaps     = (ALNoverlapFull *)safe_calloc(sizeof(ALNoverlapFull), BP_SQR(POPPER_MAX_BUBBLE_SIZE));

  for (i = 0; i < POPPER_MAX_BUBBLE_SIZE; ++i) {
    bp->bubMesgs[i].sequence = (char *)safe_malloc(sizeof(char) * (AS_READ_MAX_NORMAL_LEN + 3));
    /* This is a hack for DP_compare.  It might not be necessary. */
    bp->bubMesgs[i].sequence[0] = '\0';
    (bp->bubMesgs[i].sequence)++;

    bp->bubMesgs[i].quality = NULL;
  }

#if AS_CGB_BUBBLE_DIST_OUTPUT
  {
    char filename[FILENAME_MAX];
    sprintf(filename,"%s.bubble.nfrags.celagram",fileprefix);
    bp->nfragsFile = fopen(filename, "w");
    if (bp->nfragsFile)
      fprintf(bp->nfragsFile, "Fragments Per Raw Bubble\n");
    sprintf(filename,"%s.bubble.nfragspop.celagram",fileprefix);
    bp->nfragspopFile = fopen(filename, "w");
    if (bp->nfragspopFile)
      fprintf(bp->nfragspopFile, "Fragments Per Popped Bubble\n");
    sprintf(filename,"%s.bubble.sdiff.celagram",fileprefix);
    bp->sdiffFile = fopen(filename, "w");
    if (bp->sdiffFile)
      fprintf(bp->sdiffFile, "Largest Unaligned Block Per Bubble\n");
  }
#else
  bp->nfragsFile = bp->nfragspopFile = bp->sdiffFile = NULL;
#endif
}


void
BP_reset(BubblePopper_t bp)
{
  int i, j;
  BG_E_Iter cur_v_it;
  uint16 e_flags = AS_CGB_BUBBLE_E_IN_BUBBLE;
  IntEdge_ID e;

  for (i = 0; i < bp->curBubSize; ++i) {
    BG_V_clearFlag(bp->bg, bp->bubFrags[i], AS_CGB_BUBBLE_V_IN_BUBBLE);
    for (e = BGEI_bgn(bp->bg, &cur_v_it, bp->bubFrags[i], bgeiBoth, e_flags);
	 !BGEI_end(&cur_v_it);
	 e = BGEI_next(bp->bg, &cur_v_it, e_flags)) {
      BG_E_clearFlag(bp->bg, e, AS_CGB_BUBBLE_E_IN_BUBBLE);
      BG_E_setFlag(bp->bg, e, AS_CGB_BUBBLE_E_UNUSED);
    }
  }

  bp->curBubSize = 0;
  bp->numOlaps = 0;

  for (i = 0; i < POPPER_MAX_BUBBLE_SIZE; ++i)
    for (j = 0; j < POPPER_MAX_BUBBLE_SIZE; ++j)
      if (i != j)
	BP_setAdj(bp, i, j, 0);
      else
	BP_setAdj(bp, i, j, 1);
}


TChunkMesg *
BP_chunks(BubblePopper_t bp)
{
  return bp->chunks;
}


TChunkMesg *
BP_chunkFrags(BubblePopper_t bp)
{
  return bp->chunkFrags;
}


AChunkMesg *
BP_getChunk(BubblePopper_t bp, IntChunk_ID cid)
{
  return (AChunkMesg *) GetElement_VA(bp->chunks, cid);
}


IntFragment_ID
BP_numFrags(BubblePopper_t bp)
{
  return bp->curBubSize;
}


IntFragment_ID
BP_getFrag(BubblePopper_t bp, IntFragment_ID bid)
{
  return bp->bubFrags[bid];
}


int
BP_getBID(BubblePopper_t bp, IntFragment_ID vid)
{
  return bp->vidToBid[vid];
}


int
BP_findOverlap(BubblePopper_t bp, IntFragment_ID bid1, IntFragment_ID bid2)
{

  /* Setup fake fragment messages. */
  InternalFragMesg *if1 = &(bp->bubMesgs[bid1]);
  InternalFragMesg *if2 = &(bp->bubMesgs[bid2]);

  int32 id1 = get_iid_fragment(BG_vertices(bp->bg), bp->bubFrags[bid1]);
  if (if1->iaccession != id1) {
    char *seq_buf, *src, *dst;
    if1->iaccession = id1;
    bp->gkpStore->gkStore_getFragment(id1, &bp->rsp, GKFRAGMENT_SEQ);

    if1->clear_rng.bgn = bp->rsp.gkFragment_getClearRegionBegin();
    if1->clear_rng.end = bp->rsp.gkFragment_getClearRegionEnd  ();

    seq_buf = bp->rsp.gkFragment_getSequence();

    for (src = &(seq_buf[if1->clear_rng.bgn]), dst = if1->sequence;
	 src < &(seq_buf[if1->clear_rng.end]);
	 ++src, ++dst)
      *dst = *src;
    *dst = '\0';
  }

  int32 id2 = get_iid_fragment(BG_vertices(bp->bg), bp->bubFrags[bid2]);
  if (if2->iaccession != id2) {
    char *seq_buf, *src, *dst;
    if2->iaccession = id2;
    bp->gkpStore->gkStore_getFragment(id2, &bp->rsp, GKFRAGMENT_SEQ);

    if2->clear_rng.bgn = bp->rsp.gkFragment_getClearRegionBegin();
    if2->clear_rng.end = bp->rsp.gkFragment_getClearRegionEnd  ();

    seq_buf = bp->rsp.gkFragment_getSequence();

    for (src = &(seq_buf[if2->clear_rng.bgn]), dst = if2->sequence;
	 src < &(seq_buf[if2->clear_rng.end]);
	 ++src, ++dst)
      *dst = *src;
    *dst = '\0';
  }

#if AS_CGB_BUBBLE_VERBOSE
  fprintf(stderr, "Overlapping " F_IID " (" F_IID " , " F_IID ") and " F_IID " (" F_IID " , " F_IID ") ... ",
	  bid1, bp->bubFrags[bid1], id1, bid2, bp->bubFrags[bid2], id2);
#endif

  assert((0.0 <= POPPER_ALN_ERATE) && (POPPER_ALN_ERATE <= AS_MAX_ERROR_RATE));

  /* Compute overlap */
  int             orientation = FALSE;
  int             reversed    = FALSE;
  ALNoverlapFull *aln         = NULL;
  int             where       = 0;

  aln = POPPER_ALN_FN(if1, if2, -strlen(if2->sequence),
                          strlen(if1->sequence), FALSE,
                          POPPER_ALN_ERATE, POPPER_ALN_THRESH,
                          POPPER_ALN_MIN_LEN, POPPER_ALN_TYPE,
                          &where);
  if (!aln) {
    orientation = TRUE;
    aln = POPPER_ALN_FN(if1, if2, -strlen(if2->sequence),
                            strlen(if1->sequence), TRUE,
                            POPPER_ALN_ERATE, POPPER_ALN_THRESH,
                            POPPER_ALN_MIN_LEN, POPPER_ALN_TYPE,
                            &where);
    if (!aln)
      return FALSE;

    if ((aln->ahg < 0) && (aln->bhg < 0)) {
      reversed = TRUE;
      orientation = TRUE;
      aln = POPPER_ALN_FN(if2, if1, -strlen(if1->sequence),
                              strlen(if2->sequence), TRUE,
                              POPPER_ALN_ERATE, POPPER_ALN_THRESH,
                              POPPER_ALN_MIN_LEN, POPPER_ALN_TYPE,
                              &where);
    }

  }
  else if ((aln->ahg < 0) && (aln->bhg < 0)) {
    reversed = TRUE;
    orientation = FALSE;
    aln = POPPER_ALN_FN(if2, if1, -strlen(if1->sequence),
                            strlen(if2->sequence), FALSE,
                            POPPER_ALN_ERATE, POPPER_ALN_THRESH,
                            POPPER_ALN_MIN_LEN, POPPER_ALN_TYPE,
                            &where);
  }

  if (!aln)
    return FALSE;

  //  Save the alignment to our list of potential bubble overlaps

  bp->bubOlaps[bp->numOlaps].ahg          = aln->ahg;
  bp->bubOlaps[bp->numOlaps].bhg          = aln->bhg;
  bp->bubOlaps[bp->numOlaps].aifrag       = aln->aifrag;
  bp->bubOlaps[bp->numOlaps].bifrag       = aln->bifrag;
  bp->bubOlaps[bp->numOlaps].orientation  = aln->orientation;
  bp->bubOlaps[bp->numOlaps].overlap_type = aln->overlap_type;
  /* WARNING: HACK!  Next line has a big ol' hack.  See the comments
     in AS_CGB_Bubble.h for more explanation. */
  //bp->bubOlaps[bp->numOlaps].quality = -(aln->quality);
  bp->bubOlaps[bp->numOlaps].quality      = aln->quality;

  bp->numOlaps++;

  /* Update adjacency array. */
  if (!reversed)
    BP_setAdj(bp, bid1, bid2, 1);
  else
    BP_setAdj(bp, bid2, bid1, 1);

  return TRUE;
}


int
BP_getAdj(BubblePopper_t bp, IntFragment_ID bid1, IntFragment_ID bid2)
{
  return bp->adj[bid1 * POPPER_MAX_BUBBLE_SIZE + bid2];
}


int
BP_getAdj_VID(BubblePopper_t bp, IntFragment_ID vid1, IntFragment_ID vid2)
{
  return bp->adj[bp->vidToBid[vid1] * POPPER_MAX_BUBBLE_SIZE +
		bp->vidToBid[vid2]];
}


void
BP_setAdj(BubblePopper_t bp, IntFragment_ID bid1, IntFragment_ID bid2, int v)
{
  bp->adj[bid1 * POPPER_MAX_BUBBLE_SIZE + bid2] = v;
}


void
BP_setAdj_VID(BubblePopper_t bp, IntFragment_ID vid1, IntFragment_ID vid2,
	      int v)
{
  bp->adj[bp->vidToBid[vid1] * POPPER_MAX_BUBBLE_SIZE +
	 bp->vidToBid[vid2]] = v;
}



ALNoverlapFull *
AS_CGB_Bubble_pop_bubble(BubblePopper_t bp, IntFragment_ID start,
			 int start_sx, IntFragment_ID end,
			 int end_sx, int *num_olaps)
{
  IntFragment_ID tmp;
  int r, c, path_len, num_frags, bub_closed;
  int64 bub_size;
  BG_E_Iter e_it;
  IntEdge_ID e;
  uint16 bub_e_flags = AS_CGB_BUBBLE_E_VALID | AS_CGB_BUBBLE_E_IN_BUBBLE;
  float disc;
  int max_block_diff_size = -1;

  /* Initialize variables. */

  BP_reset(bp);

  if (BG_V_getDistance(bp->bg, start) > BG_V_getDistance(bp->bg, end)) {
    tmp = start;
    start = end;
    end = start;
  }

  /* Find fragments in bubble. */

  bp->numBubblesProcessed++;
  if (!BP_find_bubble_dfs(bp, start, end)) {
    *num_olaps = 0;
    return NULL;
  }

  num_frags = BP_numFrags(bp);
  bub_size = BG_V_getDistance(bp->bg, end) - BG_V_getDistance(bp->bg, start);

  if (bp->nfragsFile)
    fprintf(bp->nfragsFile, "%d\n", num_frags + 1);

  /* Create adjacency array. */

  for (r = 0; r < (num_frags - 1); ++r) {
    tmp = BP_getFrag(bp, r);
    for (e = BGEI_bgn(bp->bg, &e_it, tmp, bgeiOut, bub_e_flags);
	 !BGEI_end(&e_it);
	 e = BGEI_next(bp->bg, &e_it, bub_e_flags))
      BP_setAdj(bp, r, bp->vidToBid[BG_getOppositeVertex(bp->bg, e, tmp)],1);
  }

#if AS_CGB_BUBBLE_VERBOSE
  fprintf(stderr, "Fragments:\n");
  for (r = 0; r < num_frags; r++) {
    fprintf(stderr, "%d]\t\t" F_IID "\t(" F_IID ")\t" F_S64 "\n", r, bp->bubFrags[r],
	    get_iid_fragment(BG_vertices(bp->bg), bp->bubFrags[r]),
	    BG_V_getDistance(bp->bg, bp->bubFrags[r]));
  }
#endif

#if AS_CGB_BUBBLE_VERY_VERBOSE
  fprintf(stderr, "\nPre Transitive Closure Adjacency Array\n");
  for (r = 0; r < num_frags; r++) {
    for (c = 0; c < num_frags; c++)
      fprintf(stderr, "%d  ", BP_getAdj(bp, r, c));
    fprintf(stderr, "\n");
  }
#endif

  bp->numFragsInBubbles += num_frags;
  bp->totalDistSpannedByBubbles += bub_size;


  /* Check estimated discriminator score of de-bubbled unitig. */

  disc = BP_discriminator(bp);
#if AS_CGB_BUBBLE_VERBOSE
  fprintf(stderr, "Bubble discriminator is %.2f.\n", disc);
#endif
  if (disc < POPPER_MIN_DISCRIMINATOR) {
    bp->numRejectedByDiscriminator++;
#if AS_CGB_BUBBLE_VERY_VERBOSE
    fprintf(stderr, "REJECTING BUBBLE BASED ON DISCRIMINATOR.\n\n");
#endif
    *num_olaps = 0;
    return NULL;
  }

  /* Compute transitive closure */

  BP_transitive_closure(bp);

#if AS_CGB_BUBBLE_VERY_VERBOSE
  {
    fprintf(stderr, "\nPost Transitive Closure Adjacency Array\n");
    for (r = 0; r < num_frags; r++) {
      for (c = 0; c < num_frags; c++)
	fprintf(stderr, "%d  ", BP_getAdj(bp, r, c));
      fprintf(stderr, "\n");
    }
  }
#endif

  /* Find potentially missing overlaps */

  for (r = 0; r < num_frags; ++r)
    for (c = r + 1; c < num_frags; ++c)
      if (!BP_getAdj(bp, r, c)) {
	(bp->numOlapsComputed)++;
	if (BP_findOverlap(bp, r, c)) {
	  bp->numOlapsSuccessful++;
	  if (max_indel_AS_ALN_LOCOLAP_GLOBAL > max_block_diff_size)
	    max_block_diff_size = max_indel_AS_ALN_LOCOLAP_GLOBAL;
	}
      }

#if AS_CGB_BUBBLE_VERY_VERBOSE
  {
    fprintf(stderr, "\nPost Affine Overlap Adjacency Array\n");
    for (r = 0; r < num_frags; r++) {
      for (c = 0; c < num_frags; c++)
	fprintf(stderr, "%d  ", BP_getAdj(bp, r, c));
      fprintf(stderr, "\n");
    }
  }
#endif

  /* Find longest path in DAG (if possible).  If all fragments are
     on the path, keep the bubble; otherwise, reject it. */

  path_len = BP_DAG_longest_path(bp);
#if AS_CGB_BUBBLE_VERY_VERBOSE
  fprintf(stderr, "Found path of length %d.\n", path_len);
#endif

  bub_closed = ((path_len + 1) == num_frags);

#if AS_CGB_BUBBLE_VERBOSE
  if (bub_closed)
    fprintf(stderr, "SUMMARY:  Bubble should be popped!\n\n");
  else
    fprintf(stderr, "SUMMARY:  Bubble should be REJECTED!\n\n");
#endif

  if (bub_closed) {
    bp->numBubblesCollapsed++;
    bp->numFragsInCollapsedBubbles += num_frags;
    bp->totalDistSpannedByCollapsedBubbles += bub_size;
    bp->numOlapsRetained += bp->numOlaps;
    if (bp->sdiffFile)
      fprintf(bp->sdiffFile, "%d\n", max_block_diff_size);
  if (bp->nfragspopFile)
    fprintf(bp->nfragspopFile, "%d\n", num_frags + 1);

    *num_olaps = bp->numOlaps;
    return bp->bubOlaps;
  }
  else {
    *num_olaps = 0;
    return NULL;
  }
}


void
AS_CGB_Bubble_Popper_destroy(BubblePopper_t bp)
{
  int i;

  for (i = 0; i < POPPER_MAX_BUBBLE_SIZE; ++i) {
    (bp->bubMesgs[i].sequence)--; /* Other end of the hack in _init() */
    safe_free(bp->bubMesgs[i].sequence);
  }

  BG_destroy(bp->bg);
  safe_free(bp->vidToBid); // proportional to the number of fragments

  safe_free(bp->topDistArray); // POPPER_MAX_BUBBLE_SIZE
  safe_free(bp->dfsStack); // POPPER_MAX_BUBBLE_SIZE
  safe_free(bp->bubFrags); // POPPER_MAX_BUBBLE_SIZE
  safe_free(bp->bubMesgs); // POPPER_MAX_BUBBLE_SIZE
  safe_free(bp->adj);      // BP_SQR(POPPER_MAX_BUBBLE_SIZE)
  safe_free(bp->bubOlaps); // BP_SQR(POPPER_MAX_BUBBLE_SIZE)

  if (bp->nfragsFile)
    fclose(bp->nfragsFile);
  if (bp->nfragspopFile)
    fclose(bp->nfragspopFile);
  if (bp->sdiffFile)
    fclose(bp->sdiffFile);

  if (bp->allocatedByCreate) {
    safe_free(bp->bg);
    safe_free(bp);
  }
}

