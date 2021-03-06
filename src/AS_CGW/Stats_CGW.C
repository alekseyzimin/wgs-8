
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
static const char *rcsid = "$Id: Stats_CGW.C 4371 2013-08-01 17:19:47Z brianwalenz $";

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "ScaffoldGraphIterator_CGW.H"
#include "ScaffoldGraph_CGW.H"
#include "GraphCGW_T.H"
#include "AS_UTL_interval.H"
#include "AS_UTL_fileIO.H"
#include "MultiAlignment_CNS.H"

VA_DEF(int)


/* Generate statistics on the U-Unitig induced subgraph of
   the CIGraph */
void GenerateCIGraph_U_Stats(void){
  char buffer[2048];
  FILE *fout;
  GraphCGW_T *graph = ScaffoldGraph->CIGraph;
  VA_TYPE(int) *aEndDegree = CreateVA_int(GetNumGraphNodes(graph));
  VA_TYPE(int) *bEndDegree = CreateVA_int(GetNumGraphNodes(graph));
  GraphNodeIterator Nodes;
  NodeCGW_T *node;
  int unodes = 0;
  int uedges = 0;

  AS_UTL_mkdir("stat");

  fprintf(stderr,"**** GenerateCIGraph_U_Stats ****\n");

  InitGraphNodeIterator(&Nodes, graph, GRAPH_NODE_DEFAULT);
  while(NULL != (node = NextGraphNodeIterator(&Nodes))){
    int cnt = 0;

    if(node->type != DISCRIMINATORUNIQUECHUNK_CGW)
      continue;
    unodes++;

    {
      GraphEdgeIterator edges(graph, node->id, A_END, ALL_EDGES);
      EdgeCGW_T        *edge;

      while(NULL != (edge = edges.nextMerged())) {
        NodeCGW_T *other = GetGraphNode(graph,edge->idA == node->id? edge->idB: edge->idA);
        if(other->type != DISCRIMINATORUNIQUECHUNK_CGW)
          continue;
        if(edge->idA == node->id)
          uedges++;
        cnt++;
      }
    }

    Setint(aEndDegree, node->id, &cnt);

    {
      GraphEdgeIterator edges(graph, node->id, B_END, ALL_EDGES);
      EdgeCGW_T        *edge;

      while(NULL != (edge = edges.nextMerged())) {
        NodeCGW_T *other = GetGraphNode(graph,edge->idA == node->id? edge->idB: edge->idA);
        if(other->type != DISCRIMINATORUNIQUECHUNK_CGW)
          continue;
        if(edge->idA == node->id)
          uedges++;
        cnt++;
      }
    }

    Setint(bEndDegree, node->id, &cnt);
  }

  sprintf(buffer,"stat/CIGraph_U.nodeoutdegree.cgm");


  fout = fopen(buffer,"w");
  AssertPtr(fout);

  fprintf(fout,"CIGraph_U (V:%d  E:%d) OutDegrees\n",
          unodes, uedges);

  InitGraphNodeIterator(&Nodes, graph, GRAPH_NODE_UNIQUE_ONLY);
  while(NULL != (node = NextGraphNodeIterator(&Nodes))){
    int aDegree = *Getint(aEndDegree, node->id);
    int bDegree = *Getint(bEndDegree, node->id);
    fprintf(fout,"%d\n",aDegree + bDegree);
  }
  fclose(fout);

  sprintf(buffer,"stat/CIGraph_U.nodeendoutdegree.cgm");
  fout = fopen(buffer,"w");
  AssertPtr(fout);

  fprintf(fout,"CIGraph_U (V:%d  E:%d) End OutDegrees\n",
          2*unodes, uedges);

  InitGraphNodeIterator(&Nodes, graph, GRAPH_NODE_UNIQUE_ONLY);
  while(NULL != (node = NextGraphNodeIterator(&Nodes))){
    int aDegree = *Getint(aEndDegree, node->id);
    int bDegree = *Getint(bEndDegree, node->id);
    fprintf(fout,"%d\n",aDegree);
    fprintf(fout,"%d\n",bDegree);
  }
  fclose(fout);

  DeleteVA_int(aEndDegree);
  DeleteVA_int(bEndDegree);

}

/* Generate statistics on CIGraph */
void GenerateCIGraphStats(void){
  GraphCGW_T *graph = ScaffoldGraph->CIGraph;
  GraphNodeIterator Nodes;
  NodeCGW_T *node;
  int nu_unitigs = 0;
  int tfrags_nobf = 0;
  int tfrags_nolinks = 0;
  int tfrags_nobf_nolinks= 0;
  int nu_unitigs_no_bac_fragments = 0;
  int nu_unitigs_no_links = 0;
  int nu_unitigs_no_links_no_bac_fragments = 0;
  int n_unitigs = 0;

  fprintf(stderr,"**** GenerateCIGraphStats ****\n");

  InitGraphNodeIterator(&Nodes, graph, GRAPH_NODE_DEFAULT);
  while(NULL != (node = NextGraphNodeIterator(&Nodes))){
    int cnt = 0;

    // Filter surrogates
    if(ScaffoldGraph->tigStore->getNumFrags(node->id, TRUE) == 0)
      continue;

    n_unitigs++;
    if(node->flags.bits.isUnique)
      continue;
    nu_unitigs++;

    GraphEdgeIterator  edges(graph, node->id, A_END, ALL_EDGES);
    EdgeCGW_T         *edge;

    while(NULL != (edge = edges.nextMerged())) {
      cnt++;
    }

    if(cnt == 0){
      nu_unitigs_no_links++;
      tfrags_nolinks += ScaffoldGraph->tigStore->getNumFrags(node->id, TRUE);
    }
  }
  fprintf(stderr,"*@ Graph has %d unitigs of which %d are non-unique\n",
          n_unitigs, nu_unitigs);
  fprintf(stderr,"*@ %d unitigs have no external data comprising %d total fragments\n",
          nu_unitigs_no_bac_fragments, tfrags_nobf);
  fprintf(stderr,"*@ %d unitigs have no links comprising %d total fragments\n",
          nu_unitigs_no_links, tfrags_nolinks);
  fprintf(stderr,"*@ %d unitigs have no external data AND no links comprising %d total fragments\n",
          nu_unitigs_no_links_no_bac_fragments, tfrags_nobf_nolinks);
}


/* Generate stats for the subgraph of the Contig graph induced by placed contigs */
void GeneratePlacedContigGraphStats(const char *label,int iteration){

  char buffer[2048];
  FILE *fout;
  FILE *fEndOut;
  FILE *fLengthOut;
  FILE *fUnitigOut;
  GraphCGW_T *graph = ScaffoldGraph->ContigGraph;
  VA_TYPE(int) *aEndDegree = CreateVA_int(GetNumGraphNodes(graph));
  VA_TYPE(int) *bEndDegree = CreateVA_int(GetNumGraphNodes(graph));
  GraphNodeIterator Nodes;
  NodeCGW_T *node;
  int unodes = 0;
  int uedges = 0;

  if(GetNumGraphNodes(ScaffoldGraph->ScaffoldGraph) < 1){
    return;
  }

  AS_UTL_mkdir("stat");

  fprintf(stderr,"**** GeneratePlacedContigStats %s %d****\n", label, iteration);


  sprintf(buffer,"stat/%s%d.PlacedContig.nodelength.cgm", label, iteration);
  fLengthOut = fopen(buffer,"w");
  AssertPtr(fLengthOut);

  sprintf(buffer,"stat/%s%d.PlacedContig.unitigs.cgm", label,iteration);
  fUnitigOut = fopen(buffer,"w");
  AssertPtr(fUnitigOut);

  sprintf(buffer,"stat/%s%d.PlacedContig.nodeoutdegree.cgm", label,iteration);
  fout = fopen(buffer,"w");
  AssertPtr(fout);

  sprintf(buffer,"stat/%s%d.PlacedContig.nodeendoutdegree.cgm",label,iteration);
  fEndOut = fopen(buffer,"w");
  AssertPtr(fEndOut);

  InitGraphNodeIterator(&Nodes, graph, GRAPH_NODE_DEFAULT);
  while(NULL != (node = NextGraphNodeIterator(&Nodes))){
    int cnt = 0;

    if(node->scaffoldID == NULLINDEX){
      Setint(aEndDegree, node->id, &cnt);
      Setint(bEndDegree, node->id, &cnt);
      continue;
    }

    unodes++;

    {
    GraphEdgeIterator edges(graph, node->id, A_END, ALL_EDGES);
    EdgeCGW_T        *edge;

    while (NULL != (edge = edges.nextMerged())) {
      NodeCGW_T *other = GetGraphNode(graph,edge->idA == node->id? edge->idB: edge->idA);
      if(other->scaffoldID == NULLINDEX)
        continue;
      if(edge->idA == node->id)
        uedges++;
      cnt++;
    }
    }

    Setint(aEndDegree, node->id, &cnt);

    cnt = 0;

    {
    GraphEdgeIterator edges(graph, node->id, B_END, ALL_EDGES);
    EdgeCGW_T        *edge;

    while (NULL != (edge = edges.nextMerged())) {
      NodeCGW_T *other = GetGraphNode(graph,edge->idA == node->id? edge->idB: edge->idA);
      if(other->scaffoldID == NULLINDEX)
        continue;
      if(edge->idA == node->id)
        uedges++;
      cnt++;
    }
    }

    Setint(bEndDegree, node->id, &cnt);
  }


  fprintf(fout,"%s (V:%d E:%d) OutDegrees\n",
          label, unodes, uedges);
  fprintf(fEndOut,"PContig (V:%d  E:%d) End OutDegrees\n",
          2*unodes, uedges);
  fprintf(fLengthOut,"%s (V:%d E:%d) Length\n",
          label, unodes, uedges);
  fprintf(fUnitigOut,"%s (V:%d E:%d) Unitigs\n",
          label, unodes, uedges);

  InitGraphNodeIterator(&Nodes, graph, GRAPH_NODE_DEFAULT);
  while(NULL != (node = NextGraphNodeIterator(&Nodes))){
    int aDegree;
    int bDegree;

    if(node->scaffoldID == NULLINDEX)
      continue;

    aDegree = *Getint(aEndDegree, node->id);
    bDegree = *Getint(bEndDegree, node->id);

    fprintf(fLengthOut,"%d\n", (int)node->bpLength.mean);
    fprintf(fUnitigOut,"%d\n", node->info.Contig.numCI);
    fprintf(fout,"%d\n",aDegree + bDegree);
    fprintf(fEndOut,"%d\n",aDegree);
    fprintf(fEndOut,"%d\n",bDegree);
  }

  fclose(fout);
  fclose(fEndOut);
  fclose(fLengthOut);
  fclose(fUnitigOut);
  DeleteVA_int(aEndDegree);
  DeleteVA_int(bEndDegree);

}



/* Generate stats for the scaffold graph  */
/* # of scaffolds
   # scaffold edges (weight 2 or more?)
   # scaffold edges BAC only (weight 2 or more?)
   Scaffold lengths
   Contigs/Scaffold
   Intra-Scaffold Gaps
   Links/Scaffold edge w/o BACs   (weight 2 or more?)
   Links/Scaffold edge w/BACs     (weight 2 or more?)
   Nature of 2/10k scaffolding (output to text file)
*/
void GenerateScaffoldGraphStats(const char *label, int iteration){

  char buffer[2048];
  FILE *fout;
  FILE *fEndOut;
  FILE *fLengthOut, *fLengthSinglesOut;
  FILE *fContigOut;
  FILE *fLinksPerEdge_WBacs;
  FILE *fLinksPerEdge_WOBacs;
  FILE *fScaffoldGapMeans, *fScaffoldGapStds;
  FILE *fScaffoldNature;
  GraphCGW_T *graph = ScaffoldGraph->ScaffoldGraph;
  VA_TYPE(int) *aEndDegree = CreateVA_int(GetNumGraphNodes(graph));
  VA_TYPE(int) *bEndDegree = CreateVA_int(GetNumGraphNodes(graph));
  GraphNodeIterator Nodes;
  NodeCGW_T *node;
  int nodes = 0;
  int edges = 0;
  int bacOnlyEdges = 0;
  int numberInferred = 0;
  int numberRemoved = 0;

  if(GetNumGraphNodes(graph) < 1){
    return;
  }

  AS_UTL_mkdir("stat");

  fprintf(stderr,"**** GeneratePlacedContigStats %s %d ****\n", label,iteration);


  sprintf(buffer,"stat/%s%d.Scaffolds.nodelength.cgm", label,iteration);
  fLengthOut = fopen(buffer,"w");
  AssertPtr(fLengthOut);

  sprintf(buffer,"stat/%s%d.SingleScaffolds.nodelength.cgm", label,iteration);
  fLengthSinglesOut = fopen(buffer,"w");
  AssertPtr(fLengthSinglesOut);

  sprintf(buffer,"stat/%s%d.Scaffolds.Nature.txt", label,iteration);
  fScaffoldNature = fopen(buffer,"w");
  AssertPtr(fScaffoldNature);

  sprintf(buffer,"stat/%s%d.Scaffolds.contigs.cgm", label,iteration);
  fContigOut = fopen(buffer,"w");
  AssertPtr(fContigOut);

  sprintf(buffer,"stat/%s%d.Scaffolds.nodeoutdegree.cgm", label,iteration);
  fout = fopen(buffer,"w");
  AssertPtr(fout);

  sprintf(buffer,"stat/%s%d.Scaffolds.nodeendoutdegree.cgm", label,iteration);
  fEndOut = fopen(buffer,"w");
  AssertPtr(fEndOut);

  sprintf(buffer,"stat/%s%d.Scaffolds.links_per_edge_w_bac.cgm",label,iteration);
  fLinksPerEdge_WBacs = fopen(buffer,"w");
  AssertPtr(fLinksPerEdge_WBacs);

  sprintf(buffer,"stat/%s%d.Scaffolds.links_per_edge_wo_bac.cgm",label,iteration);
  fLinksPerEdge_WOBacs = fopen(buffer,"w");
  AssertPtr(fLinksPerEdge_WOBacs);

  sprintf(buffer,"stat/%s%d.Scaffolds.intra_scaffold_gap_means.cgm",label,iteration);
  fScaffoldGapMeans = fopen(buffer,"w");
  AssertPtr(fScaffoldGapMeans);

  sprintf(buffer,"stat/%s%d.Scaffolds.intra_scaffold_gap_stds.cgm",label,iteration);
  fScaffoldGapStds = fopen(buffer,"w");
  AssertPtr(fScaffoldGapStds);

  fprintf(fLinksPerEdge_WBacs, "Links Per Edge including BACs\n");
  fprintf(fLinksPerEdge_WOBacs, "Links Per Edge excluding BACs\n");

  InitGraphNodeIterator(&Nodes, graph, GRAPH_NODE_DEFAULT);
  while(NULL != (node = NextGraphNodeIterator(&Nodes))){
    int cnt = 0;
    int noBacDegree = 0, totalDegree = 0;

    nodes++;

    {
    GraphEdgeIterator edgei(graph, node->id, A_END, ALL_EDGES);
    EdgeCGW_T        *edge;

    while (NULL != (edge = edgei.nextMerged())) {
      if(edge->idA == node->id){
        if(edge->flags.bits.isInferred){
          numberInferred++;
        }
        if(edge->flags.bits.isTransitivelyRemoved){
          numberRemoved++;
        }
        if(edge->edgesContributing >= MIN_EDGES){
          edges++;
          totalDegree = EdgeDegree(graph,edge);
          assert(edge->edgesContributing == totalDegree);
          if(totalDegree && (totalDegree == noBacDegree))
            bacOnlyEdges++;
          fprintf(fLinksPerEdge_WBacs,  "%d\n",totalDegree);
          fprintf(fLinksPerEdge_WOBacs,  "%d\n",noBacDegree);
        }
        cnt++;
      }
    }
    }
    Setint(aEndDegree, node->id, &cnt);

    cnt = 0;
    {
    GraphEdgeIterator edgei(graph, node->id, B_END, ALL_EDGES);
    EdgeCGW_T        *edge;

    while (NULL != (edge = edgei.nextMerged())) {
      if(edge->idA == node->id){
        if( edge->edgesContributing >= MIN_EDGES){
          edges++;
          totalDegree = EdgeDegree(graph,edge);
          assert(edge->edgesContributing == totalDegree);
          if(totalDegree && (totalDegree == noBacDegree))
            bacOnlyEdges++;
          fprintf(fLinksPerEdge_WBacs,  "%d\n",totalDegree);
          fprintf(fLinksPerEdge_WOBacs,  "%d\n",noBacDegree);
        }
        cnt++;
      }
    }
    }
    Setint(bEndDegree, node->id, &cnt);
  }

  fprintf(fScaffoldNature,"Number removed by transitive reduction: %d\n", numberRemoved);
  fprintf(fScaffoldNature,"Number Edges marked inferred by end-tuck: %d\n", numberInferred);

  fprintf(fout,"%s (V:%d E:%d (%d)) OutDegrees\n",
          label, nodes, edges, bacOnlyEdges);
  fprintf(fEndOut,"Scaffolds (V:%d  E:%d (%d)) End OutDegrees\n",
          2*nodes, edges, bacOnlyEdges);
  fprintf(fLengthOut,"Scaffold (V:%d E:%d (%d)) Length\n",
          nodes, edges,bacOnlyEdges);
  fprintf(fLengthSinglesOut,"Scaffold (V:%d E:%d (%d)) Length\n",
          nodes, edges,bacOnlyEdges);
  fprintf(fContigOut,"Scaffold (V:%d E:%d (%d)) Contigs\n",
          nodes, edges, bacOnlyEdges);
  fprintf(fScaffoldGapMeans, "Mean IntraScaffold Gap Sizes\n");
  fprintf(fScaffoldGapStds, "Std IntraScaffold Gap Sizes\n");

  InitGraphNodeIterator(&Nodes, graph, GRAPH_NODE_DEFAULT);
  while(NULL != (node = NextGraphNodeIterator(&Nodes))){

    int aDegree = *Getint(aEndDegree, node->id);
    int bDegree = *Getint(bEndDegree, node->id);


    if(node->info.Scaffold.numElements > 1)
      fprintf(fLengthOut,"%d\n", (int)node->bpLength.mean);
    else
      fprintf(fLengthSinglesOut,"%d\n", (int)node->bpLength.mean);

    fprintf(fContigOut, "%d\n", node->info.Scaffold.numElements);
    fprintf(fout, "%d\n",aDegree + bDegree);
    fprintf(fEndOut, "%d\n",aDegree);
    fprintf(fEndOut, "%d\n",bDegree);

    {
      CIScaffoldTIterator Contigs;
      NodeCGW_T *prev = NULL;
      NodeCGW_T *next;
      double mean, std;

      InitCIScaffoldTIterator(ScaffoldGraph, node, TRUE, FALSE, &Contigs);
      for(next = NextCIScaffoldTIterator(&Contigs), prev = NULL; next != NULL; prev = next, next = NextCIScaffoldTIterator(&Contigs)){
        if(prev && next){
          if(prev->offsetAEnd.mean < prev->offsetBEnd.mean){
            if(next->offsetAEnd.mean < next->offsetBEnd.mean){
              mean = next->offsetAEnd.mean - prev->offsetBEnd.mean;
              std = next->offsetAEnd.variance - prev->offsetBEnd.variance;
            }else{
              mean = next->offsetBEnd.mean - prev->offsetBEnd.mean;
              std = next->offsetBEnd.variance - prev->offsetBEnd.variance;
            }
          }else{
            if(next->offsetAEnd.mean < next->offsetBEnd.mean){
              mean = next->offsetAEnd.mean - prev->offsetAEnd.mean;
              std = next->offsetAEnd.variance - prev->offsetAEnd.variance;
            }else{
              mean = next->offsetBEnd.mean - prev->offsetAEnd.mean;
              std = next->offsetBEnd.variance - prev->offsetAEnd.variance;
            }

          }
          if(std > 0)
            std = sqrt(std);
          else
            std = 0.0;
          fprintf(fScaffoldGapMeans, "%d\n", (int)mean);
          fprintf(fScaffoldGapStds, "%d\n", (int)std);

        }

      }
    }
  }


  fclose(fout);
  fclose(fEndOut);
  fclose(fLengthOut);
  fclose(fLengthSinglesOut);
  fclose(fContigOut);
  fclose(fScaffoldNature);
  fclose(fScaffoldGapMeans);
  fclose(fScaffoldGapStds);
  DeleteVA_int(aEndDegree);
  DeleteVA_int(bEndDegree);

}



/* Compute statistics on links
   # of mates per link edge
   std of link edges by
   all
   overlap confirmed (non tandem)
   not confirmed (no overlap)
*/
void GenerateLinkStats(GraphCGW_T *graph, const char *label, int iteration){
  int i;
  char buffer[2048];
  FILE *mates_per_link;
  FILE *linkstd_all;
  FILE *linkstd_w_overlap;
  FILE *linkstd_no_overlap;
  int mates;
  int cgbOverlap = 0;
  int nonCGBOverlap = 0;
  const char *graphName = (graph->type == CI_GRAPH?"CI":"Contig");

  AS_UTL_mkdir("stat");

  sprintf(buffer,"stat/%s%s%d.mates_per_link.cgm",graphName,label,iteration);
  mates_per_link = fopen(buffer,"w");
  AssertPtr(mates_per_link);
  fprintf(mates_per_link,"Mates per link %s\n", label);

  sprintf(buffer,"stat/%s%s%d.linkstd_all.cgm",graphName,label,iteration);
  linkstd_all = fopen(buffer,"w");
  AssertPtr(linkstd_all);
  fprintf(linkstd_all,"Link Standard Deviation -- All %s\n", label);

  sprintf(buffer,"stat/%s%s%d.linkstd_w_overlap.cgm",graphName,label,iteration);
  linkstd_w_overlap = fopen(buffer,"w");
  AssertPtr(linkstd_w_overlap);
  fprintf(linkstd_w_overlap,"Link Standard Deviation -- Confirmed %s\n", label);

  sprintf(buffer,"stat/%s%s%d.linkstd_no_overlap.cgm",graphName,label, iteration);
  linkstd_no_overlap = fopen(buffer,"w");
  AssertPtr(linkstd_no_overlap);
  fprintf(linkstd_no_overlap,"Link Standard Deviation -- Unconfirmed %s\n", label);



  for(i = 0; i < GetNumGraphEdges(graph); i++){
    EdgeCGW_T *edge = GetGraphEdge(graph,i);
    NodeCGW_T *nodeA, *nodeB;
    CDS_CID_t eid = GetVAIndex_EdgeCGW_T(graph->edges, edge);
    int std = (edge->distance.variance > 0.0 ? (int)sqrt(edge->distance.variance):-1);
    if(edge->flags.bits.isDeleted || edge->topLevelEdge != eid)
      continue;

    mates = edge->edgesContributing;
    nodeA = GetGraphNode(graph, edge->idA);
    nodeB = GetGraphNode(graph, edge->idB);

    if(graph->type == CONTIG_GRAPH &&
       (nodeA->scaffoldID == NULLINDEX ||
        nodeB->scaffoldID == NULLINDEX ))
      continue;

    if(isOverlapEdge(edge)){
      mates--;

      if(mates){  // we only want edges that are NOT overlap only
        fprintf(linkstd_w_overlap,"%d\n", std);
        if(graph->type == CONTIG_GRAPH){
          ChunkOverlapCheckT olap;
          int overlapFound = LookupOverlap(graph, edge->idA, edge->idB, edge->orient, &olap);
          if(overlapFound && olap.fromCGB){
            cgbOverlap++;
          }else{
            nonCGBOverlap++;
          }
        }
      }
    }else{
      fprintf(linkstd_no_overlap,"%d\n", std);
    }

    if(mates){
      fprintf(linkstd_all,"%d\n", std);
      fprintf(mates_per_link,"%d\n", mates);
    }
  }
  if(graph->type == CONTIG_GRAPH)
    fprintf(stderr,"*** Links confirmed by cgbOlaps %d  onCGBOlaps %d\n",
            cgbOverlap, nonCGBOverlap);

  fclose(linkstd_all);
  fclose(linkstd_w_overlap);
  fclose(linkstd_no_overlap);
  fclose(mates_per_link);
}


int32 ApproximateUnitigCoverage(NodeCGW_T *unitig){
  int32 length=0;
  int i;
  MultiAlignT *ma = ScaffoldGraph->tigStore->loadMultiAlign(unitig->id, TRUE);

  for(i = 0; i < GetNumIntMultiPoss(ma->f_list); i++){
    IntMultiPos *pos = GetIntMultiPos(ma->f_list, i);
    length += abs(pos->position.bgn - pos->position.end);
  }

  if(length == 0)
    return 0;

  return (int32) ( length / unitig->bpLength.mean);
}


void GenerateSurrogateStats(const char *phase){
  FILE *surrogPer, *surrogSize, *surrogFrags, *surrogRatio;
  FILE *surrogCreated;
  char buffer[1024];
  GraphNodeIterator Nodes;
  NodeCGW_T *node;
  int stoneSurrogs=0,
    walkSurrogs=0;

  AS_UTL_mkdir("stat");

  sprintf(buffer,"stat/%s.surrogates_per_repeatCI.cgm",phase);
  surrogPer = fopen(buffer,"w");
  AssertPtr(surrogPer);

  sprintf(buffer,"stat/%s.surrogates_Created.cgm", phase);
  surrogCreated = fopen(buffer,"w");
  AssertPtr(surrogCreated);

  fprintf(surrogPer,"Surrogates Per Repeat CI\n");

  sprintf(buffer,"stat/%s.surrogates_size.cgm", phase);
  surrogSize = fopen(buffer,"w");
  AssertPtr(surrogSize);

  fprintf(surrogSize,"Surrogate Sizes\n");

  sprintf(buffer,"stat/%s.surrogates_fragsPer.cgm", phase);
  surrogFrags = fopen(buffer,"w");
  AssertPtr(surrogFrags);

  fprintf(surrogFrags,"Fragments per Repeat CI\n");

  sprintf(buffer,"stat/%s.surrogates_ratio.cgm",phase);
  surrogRatio = fopen(buffer,"w");
  AssertPtr(surrogRatio);

  fprintf(surrogRatio,"Approx Cov / Num Copies\n");

  InitGraphNodeIterator(&Nodes, ScaffoldGraph->CIGraph, GRAPH_NODE_DEFAULT);
  while(NULL != (node = NextGraphNodeIterator(&Nodes))){
    char type;
    switch(node->type){
      case DISCRIMINATORUNIQUECHUNK_CGW:
        type = 'U';
        break;
      case UNIQUECHUNK_CGW:
        type = 'u';
        break;
      case UNRESOLVEDCHUNK_CGW:
        type = '?';
        break;
      case RESOLVEDREPEATCHUNK_CGW:
        type = 'R';
        if(node->flags.bits.isWalkSurrogate){
          walkSurrogs++;
        }else if(node->flags.bits.isStoneSurrogate){
          stoneSurrogs++;
        }
        break;
      default:
        type = 'X';
        break;
    }
#ifdef DEBUG_DETAILED
    fprintf(stderr,"* Node " F_CID" %c contig:" F_CID"  numInstances %d\n",
            node->id, type, node->info.CI.contigID, node->info.CI.numInstances);
#endif
    if((node->type != UNRESOLVEDCHUNK_CGW) ||    // is not a surrogate parent
       (node->info.CI.numInstances == 0))        // has no surrogates
      continue;
    fprintf(surrogSize,"%d\n", (int)node->bpLength.mean);
    fprintf(surrogPer, "%d\n", (int)node->info.CI.numInstances);
    fprintf(surrogFrags, "%d\n", ScaffoldGraph->tigStore->getNumFrags(node->id, TRUE));
    fprintf(surrogRatio, "%d\n", ApproximateUnitigCoverage(node)/node->info.CI.numInstances );
  }
  fprintf(stderr,"* Stones: %d  Walks:%d\n",
          stoneSurrogs, walkSurrogs);


  fclose(surrogCreated);
  fclose(surrogPer);
  fclose(surrogSize);
  fclose(surrogRatio);
  fclose(surrogFrags);
}
