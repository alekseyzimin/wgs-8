
/**************************************************************************
 * This file is part of Celera Assembler, a software program that
 * assembles whole-genome shotgun reads into contigs and scaffolds.
 * Copyright (C) 1999-2004, The Venter Institute. All rights reserved.
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

#ifndef INCLUDE_AS_BAT_DATATYPES
#define INCLUDE_AS_BAT_DATATYPES

static const char *rcsid_INCLUDE_AS_BAT_DATATYPES = "$Id: AS_BAT_Datatypes.H 4557 2014-08-11 12:24:27Z brianwalenz $";

#include "AS_global.H"
#include "AS_MSG_pmesg.H"
#include "AS_OVS_overlapStore.H"
#include "AS_PER_gkpStore.H"

#include <map>
#include <set>
#include <list>
#include <vector>
#include <algorithm>

#ifndef BROKEN_CLANG_OpenMP
#include <omp.h>
#endif

using namespace std;

#include "AS_BAT_Logging.H"
#include "AS_BAT_OverlapCache.H"

const uint32  noUnitig = 0xffffffff;

////////////////////////////////////////

#define BADMATE_INTRA_STDDEV 3  //  Mates more than this stddev away in the same unitig are bad
#define BADMATE_INTER_STDDEV 5  //  Mates more than this stddev away from the end of the unitig are bad

//  Historical.  Eli had problems with STL and max.
#undef max

////////////////////////////////////////

class FragmentInfo;
class OverlapCache;
class BestOverlapGraph;
class ChunkGraph;
class InsertSizes;

extern FragmentInfo     *FI;
extern OverlapCache     *OC;
extern BestOverlapGraph *OG;
extern ChunkGraph       *CG;
extern InsertSizes      *IS;

////////////////////////////////////////

////////////////////////////////////////

static const SeqInterval NULL_SEQ_LOC = {0,0};

inline
bool
isReverse(SeqInterval pos) {
  return(pos.bgn > pos.end);
}

inline
bool
operator==(SeqInterval a, SeqInterval b) {
  return(((a.bgn == b.bgn) && (a.end == b.end)) ||
         ((a.bgn == b.end) && (a.end == b.bgn)));
}

inline
bool
operator!=(SeqInterval a, SeqInterval b) {
  return(((a.bgn != b.bgn) || (a.end != b.end)) &&
         ((a.bgn != b.end) || (a.end != b.bgn)));
}

inline
bool
operator<(SeqInterval a, SeqInterval b) {
  if (isReverse(a)) {
    if (isReverse(b)) return a.end < b.end;
    else              return a.end < b.bgn;
  } else {
    if (isReverse(b)) return a.bgn < b.end;
    else              return a.bgn < b.bgn;
  }
}

////////////////////////////////////////

class FragmentEnd {
public:
  FragmentEnd() {
    _id  = 0;
    _e3p = false;
  };
  FragmentEnd(uint32 id, bool e3p) {
    _id  = id;
    _e3p = e3p;
  };

  uint32  fragId(void)  const { return(_id); };
  bool    frag3p(void)  const { return(_e3p == true);  };
  bool    frag5p(void)  const { return(_e3p == false); };

  bool operator==(FragmentEnd const that) const {
    return((fragId() == that.fragId()) && (frag3p() == that.frag3p()));
  };

  bool operator!=(FragmentEnd const that) const {
    return((fragId() != that.fragId()) || (frag3p() != that.frag3p()));
  };

  bool operator<(FragmentEnd const that) const {
    if (fragId() != that.fragId())
      return fragId() < that.fragId();
    else
      return frag3p() < that.frag3p();
  };

private:
  uint32   _id:31;
  uint32   _e3p:1;
};


//  Swiped from AS_OVS_overlap.h, modified to take a BAToverlap instead of an OVSoverlap.

static
uint32
AS_BAT_overlapAEndIs5prime(const BAToverlap& olap) {
  return((olap.a_hang < 0) && (olap.b_hang < 0));
}

static
uint32
AS_BAT_overlapAEndIs3prime(const BAToverlap& olap) {
  return((olap.a_hang > 0) && (olap.b_hang > 0));
}

static
uint32
AS_BAT_overlapBEndIs3prime(const BAToverlap& olap) {
  return((AS_BAT_overlapAEndIs5prime(olap) && (olap.flipped == false)) ||
         (AS_BAT_overlapAEndIs3prime(olap) && (olap.flipped == true)));
}


class BestEdgeOverlap {
public:
  BestEdgeOverlap() {
    clear();
  };
  ~BestEdgeOverlap() {
  };

  void    clear(void) {
    _id    = 0;
    _e3p   = 0;
    _ahang = 0;
    _bhang = 0;
  };

  void    set(BAToverlap const &olap) {
    _id    = olap.b_iid;
    _e3p   = AS_BAT_overlapBEndIs3prime(olap);
    _ahang = olap.a_hang;
    _bhang = olap.b_hang;
  };

  void    set(uint32 id, bool e3p, int32 ahang, int32 bhang) {
    _id    = id;
    _e3p   = e3p;
    _ahang = ahang;
    _bhang = bhang;
  };


  uint32  fragId(void)  const { return(_id); };
  bool    frag3p(void)  const { return(_e3p == true);  };
  bool    frag5p(void)  const { return(_e3p == false); };

  int32   ahang(void)   const { return(_ahang); };
  int32   bhang(void)   const { return(_bhang); };

private:
  uint32            _id;
#if AS_OVS_HNGBITS < 16
  uint32            _e3p   : 1;    //  Overlap with the 3' end of that fragment
  int32             _ahang : AS_OVS_HNGBITS;
  int32             _bhang : AS_OVS_HNGBITS;
#else
  uint64            _e3p   : 1;
  int64             _ahang : AS_OVS_HNGBITS;
  int64             _bhang : AS_OVS_HNGBITS;
#endif
};


// Contains what kind of containment relationship exists between fragment a and fragment b
//
class BestContainment{
public:
  BestContainment() {
    clear();
  };
  ~BestContainment() {
  };

  void    clear(void) {
    container       = 0;
    isContained     = false;
    sameOrientation = false;
    a_hang          = 0;
    b_hang          = 0;
  };

  uint32  container;
#if AS_OVS_HNGBITS < 16
  uint32  isContained     : 1;  //  See comments in AS_BAT_Unitig_PlaceFragmentsUsingEdges.cc
  uint32  sameOrientation : 1;
  int32   a_hang          : AS_OVS_HNGBITS;
  int32   b_hang          : AS_OVS_HNGBITS;
#else
  uint64  isContained     : 1;
  uint64  sameOrientation : 1;
  int64   a_hang          : AS_OVS_HNGBITS;
  int64   b_hang          : AS_OVS_HNGBITS;
#endif
};




class FragmentInfo {
public:
  FragmentInfo(gkStore *gkpStore, const char *prefix, uint32 minReadLen);
  ~FragmentInfo();

  uint64  memoryUsage(void) {
    return((3 * sizeof(uint32) * _numFragments) +
           (2 * sizeof(double) * _numLibraries) + 
           (2 * sizeof(uint32) * _numLibraries));
  };

  uint32  numFragments(void) { return(_numFragments); };
  uint32  numLibraries(void) { return(_numLibraries); };

  uint32  fragmentLength(uint32 iid) { return(_fragLength[iid]); };
  uint32  mateIID(uint32 iid)        { return(_mateIID[iid]); };
  uint32  libraryIID(uint32 iid)     { return(_libIID[iid]);  };

  double  mean(uint32 iid)   { return(_mean[iid]); };
  double  stddev(uint32 iid) { return(_stddev[iid]); };

  uint32  numMatesInLib(uint32 iid) { return(_numMatesInLib[iid]); };

  uint32  overlapLength(uint32 a_iid, uint32 b_iid, int32 a_hang, int32 b_hang) {
    int32  alen = fragmentLength(a_iid);
    int32  blen = fragmentLength(b_iid);
    int32  aovl = 0;
    int32  bovl = 0;

    assert(alen > 0);
    assert(blen > 0);

    if (a_hang < 0) {
      //  b_hang < 0      ?     ----------  :     ----
      //                  ?  ----------     :  ----------
      //
      aovl = (b_hang < 0) ? (alen + b_hang) : (alen);
      bovl = (b_hang < 0) ? (blen + a_hang) : (blen + a_hang - b_hang);
    } else {
      //  b_hang < 0      ?  ----------              :  ----------
      //                  ?     ----                 :     ----------
      //
      aovl = (b_hang < 0) ? (alen - a_hang + b_hang) : (alen - a_hang);
      bovl = (b_hang < 0) ? (blen)                   : (blen - b_hang);
    }

    if ((aovl <= 0) || (bovl <= 0) || (aovl > alen) || (bovl > blen)) {
      fprintf(stderr, "WARNING: bogus overlap found for A=" F_U32 " B=" F_U32 "\n", a_iid, b_iid);
      fprintf(stderr, "WARNING:                     A len=" F_S32 " hang=" F_S32 " ovl=" F_S32 "\n", alen, a_hang, aovl);
      fprintf(stderr, "WARNING:                     B len=" F_S32 " hang=" F_S32 " ovl=" F_S32 "\n", blen, b_hang, bovl);
    }

    if (aovl < 0)     aovl = 0;
    if (bovl < 0)     bovl = 0;

    if (aovl > alen)  aovl = alen;
    if (bovl > blen)  bovl = blen;

    assert(aovl > 0);
    assert(bovl > 0);
    assert(aovl <= alen);
    assert(bovl <= blen);

    //  AVE does not work.      return((uint32)((aovl, bovl)/2));
    //  MAX does not work.      return((uint32)MAX(aovl, bovl));

    return(aovl);
  };

  //  For DIAGNOSTIC ONLY.  Mark this fragment as 'deleted' so checkUnitigMembership() will pass
  //  when we ignore fragments.  The resuting assembly will fail in CGW.
  void    markAsIgnore(uint32 iid) {
    _fragLength[iid] = 0;
  };

private:
  void      save(const char *prefix);
  bool      load(const char *prefix);

  uint32   _numFragments;
  uint32   _numLibraries;

  uint32  *_fragLength;
  uint32  *_mateIID;
  uint32  *_libIID;

  double  *_mean;
  double  *_stddev;

  uint32  *_numFragsInLib;
  uint32  *_numMatesInLib;
};

#endif



