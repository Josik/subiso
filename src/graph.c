/*
 *	Subgraph Isomorphism - Graph representation
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "graph.h"
#include "array.h"
#include "util.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

GRAPH * G_GRAPH;
GRAPH * F_GRAPH;
int   * F_ECC;


/****************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************/

/*---------------------------------------------------------------------------
 * Function: graph_load 
 *-------------------------------------------------------------------------*/ 
GRAPH * graph_load (const char * f_name, int max_v)
{
  FILE * in_f = fopen(f_name, "r");
  GRAPH * tmp = (GRAPH *)xmalloc(sizeof(*tmp));
  fscanf(in_f, "%d", &(tmp->n_cnt));
  if (tmp->n_cnt > max_v)
  {
    graph_free(tmp);
    return NULL;
  }
  /* Load adjacency lists from input file */
  ARR_ALLOC(tmp->edges, tmp->n_cnt);
  for (int i = 0; i < tmp->n_cnt; i++)
  {
    tmp->edges[i] = (ADJ_TABLE *)xmalloc(sizeof(**tmp->edges));
    table_adj_init(tmp->edges[i]);
    int cnt;
    fscanf(in_f, "%d", &cnt);
    for (int j = 0; j < cnt; j++)
    {
      int to;
      fscanf(in_f, "%d", &to);
      table_adj_new(tmp->edges[i], to);
    }
  }
  fclose(in_f);
  return tmp;
}

/*---------------------------------------------------------------------------
 * Function: graph_clone
 *-------------------------------------------------------------------------*/ 
GRAPH * graph_clone (GRAPH * g)
{
  GRAPH * tmp = (GRAPH *)xmalloc(sizeof(*tmp));
  tmp->n_cnt = g->n_cnt;
  ARR_ALLOC(tmp->edges, tmp->n_cnt);
  for (int i = 0; i < tmp->n_cnt; i++) 
  {
    tmp->edges[i] = (ADJ_TABLE *)xmalloc(sizeof(**tmp->edges));
    table_adj_init(tmp->edges[i]);
    FOR_ADJ(g->edges[i], node)
    {
      table_adj_new(tmp->edges[i], node->key);
    }
    FOR_ADJ_END;
  }
  return tmp;
}

/*---------------------------------------------------------------------------
 * Function: graph_free
 *-------------------------------------------------------------------------*/ 
void graph_free (GRAPH * g)
{
  if (!g) return;
  for (int i = 0; i < g->n_cnt; i++) 
  {
    table_adj_cleanup(g->edges[i]);
    xfree(g->edges[i]);
  }
  ARR_FREE(g->edges);
  xfree(g);
}

/*---------------------------------------------------------------------------
 * Function: graph_add_edge
 *-------------------------------------------------------------------------*/
void graph_add_edge (GRAPH * g, int from, int to)
{
  table_adj_lookup(g->edges[from], to);
}

/*---------------------------------------------------------------------------
 * Function: graph_is_adj
 *-------------------------------------------------------------------------*/ 
int graph_is_adj (GRAPH * g, int from, int to)
{
  if (!table_adj_find(g->edges[from], to)) return 0;
  return 1;
}

/*---------------------------------------------------------------------------
 * Function: graph_pre_f_ecc
 *-------------------------------------------------------------------------*/ 
void graph_pre_f_ecc (void)
{
  int * q, qh, qt, * d;
  
  ARR_ALLOC(F_ECC, F_GRAPH->n_cnt);
  ARR_ALLOC(q, F_GRAPH->n_cnt);
  ARR_ALLOC(d, F_GRAPH->n_cnt);
  for (int i = 0; i < ARR_LEN(F_ECC); i++) F_ECC[i] = 0;
  
  for (int i = 0; i < F_GRAPH->n_cnt; i++)
  {
    for (int j = 0; j < ARR_LEN(d); j++) d[j] = -1;
    qh = qt = 0;
    q[qh++] = i;
    d[i] = 0;
    while (qt < qh)
    {
      int x = q[qt++];
      FOR_ADJ(F_GRAPH->edges[x], node)
      {
        int y = node->key;
        if (d[y] < 0)
        {
          d[y] = d[x] + 1;
          q[qh++] = y;
        }
      }
      FOR_ADJ_END;
    }
    for (int j = 0; j < F_GRAPH->n_cnt; j++) F_ECC[i] = MAX(F_ECC[i], d[j]);
  }
  
  ARR_FREE(q);
  ARR_FREE(d);
}

/*---------------------------------------------------------------------------
 * Function: graph_print
 *-------------------------------------------------------------------------*/
int graph_print (GRAPH * g)
{
  DBG_PRINT("GRAPH: |V(G)| = %d\n-------------------\n", g->n_cnt);
  for (int i = 0; i < g->n_cnt; i++)
  {
    DBG_PRINT("Node #%d(deg = %d): ", i, g->edges[i]->hash_count);
    FOR_ADJ(g->edges[i], node)
    {
      DBG_PRINT("%d ", node->key);
    }
    FOR_ADJ_END;
    DBG_PRINT("%s", "\n");
  }
  DBG_PRINT("%s\n", "-------------------");
}
