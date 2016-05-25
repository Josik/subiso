/*
 *	Subgraph Isomorphism - Main procedure
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include "common.h"
#include "tree_dec.h"
#include "nice_tree_dec.h"
#include "util.h"
#include "graph.h"
#include "graph_result.h"
#include "subiso.h"
#include "resbuf.h"
#include "array.h"
#include "tests.h"
#include <ucw/fastbuf.h>
#include <ucw/varint.h>

int      SEED;
double A_TIME;

int main (int argc, char * argv [])
{
  int rep_cnt = -1;
  SEED = time(NULL);
  
  if (argc < 3)
  {
    fprintf(stderr, "Usage: ./grs <graph_big> <graph_pattern> [seed] [iteration count]\n");
    force_exit();
  }
  if (argc >= 4)
  {
    SEED = atoi(argv[3]);
    if (argc >= 5) rep_cnt = atoi(argv[4]);
  }
  
  G_GRAPH = graph_load(argv[1], MAX_G_VERTICES);
  F_GRAPH = graph_load(argv[2], MAX_F_VERTICES);
  graph_pre_f_ecc();
  GRAPH * tmp_f = graph_clone(F_GRAPH);
  TREE_DEC * ftd = td_get(tmp_f);
  NICE_TREE_DEC * nftd = ntd_get(ftd);

#ifdef LOCAL_DEBUG
  DBG_PRINT("%s\n", "Graph G:");
  graph_print(G_GRAPH);
  DBG_PRINT("%s\n", "Graph F:");
  graph_print(F_GRAPH);
  td_print(ftd);
  ntd_print(nftd);
#endif

  srand(SEED);
  if (rep_cnt < 0)
  {
    rep_cnt = 1;
    for (int i = 0; i < F_GRAPH->n_cnt; i++) rep_cnt *= 3; 
  }
  
  clock_t start = clock();
  GRAPH_RESULT ** result = subiso_run(nftd, rep_cnt);
  clock_t end = clock();
  for (int i = 0; i < ARR_LEN(result); i++) graph_result_print(result[i]);
  printf(">>> UNIQUE subgraphs found after %d runs = %d <<<\n", rep_cnt, ARR_LEN(result));
  printf(">> Time = %.6f, avg time per iteration = %.6f<<\n", (end - start) / (double)CLOCKS_PER_SEC, A_TIME / (rep_cnt * (double)CLOCKS_PER_SEC));
  
#ifdef TESTING
  assert(test_tree_dec(ftd) == TEST_OK);
  assert(test_nice_tree_dec(nftd) == TEST_OK);
  assert(test_results(result) == TEST_OK);
#endif
  
  graph_free(tmp_f);
  td_free(ftd);
  ntd_free(nftd);
  graph_result_array_free(result);
  free_all();
  
  return 0;
}
