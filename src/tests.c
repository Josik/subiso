/*
 *	Subgraph Isomorphism - Qualitative tests
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "tests.h"
#include "graph.h"
#include "graph_result.h"
#include "array.h"
#include "util.h"
#include "tree_dec.h"
#include "nice_tree_dec.h"

/****************************************************************************
 * STATIC FUNCTIONS
 ***************************************************************************/

/*---------------------------------------------------------------------------
 * Function: dfs_trv_td
 *---------------------------------------------------------------------------
 */ 
void dfs_trv_td (TREE_DEC * td, int x, int v_i, int * vis_dfs)
{
  if (vis_dfs[x] || !GET_BIT(td->nodes[x].bag, v_i)) return;
  vis_dfs[x] = 1;
  for (int i = 0; i < F_GRAPH->n_cnt; i++) 
  {
    if (GET_BIT(td->nodes[x].adj, i)) dfs_trv_td(td, i, v_i, vis_dfs);
  }
}

/*---------------------------------------------------------------------------
 * Function: dfs_trv_ntd
 *-------------------------------------------------------------------------*/
void dfs_trv_ntd (NICE_TREE_DEC_NODE * x, int v_i, int * vis_dfs, NICE_TREE_DEC_NODE * prev)
{
  if (!x || vis_dfs[x->idx] || !GET_BIT(x->bag, v_i)) return;
  vis_dfs[x->idx] = 1;
  if (x->parent != prev) dfs_trv_ntd(x->parent, v_i, vis_dfs, x);
  if (x->child_1 != prev) dfs_trv_ntd(x->child_1, v_i, vis_dfs, x);
  if (x->child_2 != prev) dfs_trv_ntd(x->child_2, v_i, vis_dfs, x);
}
 
/****************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************/

/*---------------------------------------------------------------------------
 * Function: test_tree_dec 
 *-------------------------------------------------------------------------*/ 
int test_tree_dec      (TREE_DEC * td)
{
  /* Test vertex coverage */
  int vis_ver[MAX_F_VERTICES];
  memset(vis_ver, 0, sizeof(vis_ver));
  for (int i = 0; i < F_GRAPH->n_cnt; i++)
  {
    for (int j = 0; j < F_GRAPH->n_cnt; j++) if (GET_BIT(td->nodes[i].bag, j)) vis_ver[j] = 1; 
  }
  for (int i = 0; i < F_GRAPH->n_cnt; i++) if (!vis_ver[i]) return TEST_NOK;
  /* Test edge coverage */
  int vis_edg[MAX_F_VERTICES][MAX_F_VERTICES];
  memset(vis_edg, 0, sizeof(vis_edg));
  for (int i = 0; i < F_GRAPH->n_cnt; i++)
  {
    for (int j = 0; j < F_GRAPH->n_cnt; j++) for (int k = j + 1; k < F_GRAPH->n_cnt; k++) 
    {
      if (GET_BIT(td->nodes[i].bag, j) && GET_BIT(td->nodes[i].bag, k)) 
      {
        vis_edg[j][k] = vis_edg[k][j] = 1;
      }
    }
  }
  for (int i = 0; i < F_GRAPH->n_cnt; i++)
  {
    FOR_ADJ(F_GRAPH->edges[i], node)
    {
      if (!vis_edg[i][node->key]) return TEST_NOK;
    }
    FOR_ADJ_END;
  }
  /* Test vertex-induced connectivity */
  for (int v_i = 0; v_i < F_GRAPH->n_cnt; v_i++)
  {
    int cc, vis_dfs[MAX_F_VERTICES];
    cc = 0;
    memset(vis_dfs, 0, sizeof(vis_dfs));
    for (int i = 0; i < F_GRAPH->n_cnt; i++) if (GET_BIT(td->nodes[i].bag, v_i) && !vis_dfs[i])
    {
      dfs_trv_td(td, i, v_i, vis_dfs);
      if (++cc > 1) return TEST_NOK;
    }
  }
  /* Test treewidth optimality */
  int ms = 0;
  for (int i = 0; i < F_GRAPH->n_cnt; i++)
  {
    int cs = 0;
    for (int j = 0; j < F_GRAPH->n_cnt; j++) if (GET_BIT(td->nodes[i].bag, j)) ++cs;
    ms = MAX(ms, cs);
  }
  if (ms - 1 != td->tw) return TEST_NOK;
  return TEST_OK;
}

/*---------------------------------------------------------------------------
 * Function: test_nice_tree_dec 
 *-------------------------------------------------------------------------*/ 
int test_nice_tree_dec (NICE_TREE_DEC * ntd)
{
  /* Test correctness of auxilary data fields */
  int bag_ver[MAX_F_VERTICES];
  memset(bag_ver, 0, sizeof(bag_ver));
  for (int i = 0; i < ntd->b_cnt; i++)
  {
    for (int j = 0; j < F_GRAPH->n_cnt; j++) if (GET_BIT(ntd->nodes[i].bag, j)) bag_ver[j] = 1;
    for (int j = 0; j < ARR_LEN(ntd->nodes[i].bag_cont); j++) if (!bag_ver[ntd->nodes[i].bag_cont[j]]) return TEST_NOK;
  }
  for (int i = 0; i < ntd->b_cnt; i++)
  {
    int ac = ARR_LEN(ntd->nodes[i].adj);
    switch (ntd->nodes[i].type)
    {
      case LEAF_NODE:
        if (ac != 1) return TEST_NOK;
        break;
      case FORGET_NODE:
        if (!ARR_LEN(ntd->nodes[i].bag_cont))
        {
          if (ac != 1) return TEST_NOK;
          else break;
        }
      case INTRODUCE_NODE:
        if (ac != 2) return TEST_NOK;
        break;
      case JOIN_NODE:
        if (ac != 3) return TEST_NOK;
        break;
      default:
        return TEST_NOK;
        break;
    }
  }
  /* Test vertex coverage */
  int vis_ver[MAX_F_VERTICES];
  memset(vis_ver, 0, sizeof(vis_ver));
  for (int i = 0; i < ntd->b_cnt; i++)
  {
    for (int j = 0; j < F_GRAPH->n_cnt; j++) if (GET_BIT(ntd->nodes[i].bag, j)) vis_ver[j] = 1;
  }
  for (int i = 0; i < F_GRAPH->n_cnt; i++) if (!vis_ver[i]) return TEST_NOK;
  /* Test edge coverage */
  int vis_edg[MAX_F_VERTICES][MAX_F_VERTICES];
  memset(vis_edg, 0, sizeof(vis_edg));
  for (int i = 0; i < ntd->b_cnt; i++)
  {
    for (int j = 0; j < F_GRAPH->n_cnt; j++) for (int k = j + 1; k < F_GRAPH->n_cnt; k++) 
    {
      if (GET_BIT(ntd->nodes[i].bag, j) && GET_BIT(ntd->nodes[i].bag, k)) 
      {
        vis_edg[j][k] = vis_edg[k][j] = 1;
      }
    }
  }
  for (int i = 0; i < F_GRAPH->n_cnt; i++)
  {
    FOR_ADJ(F_GRAPH->edges[i], node)
    {
      if (!vis_edg[i][node->key]) return TEST_NOK;
    }
    FOR_ADJ_END;
  }
  /* Test vertex-induced connectivity */
  int * vis_dfs;
  ARR_ALLOC(vis_dfs, ntd->b_cnt);
  for (int v_i = 0; v_i < F_GRAPH->n_cnt; v_i++)
  {
    int cc;
    cc = 0;
    memset(vis_dfs, 0, ntd->b_cnt * sizeof(*vis_dfs));
    for (int i = 0; i < ntd->b_cnt; i++) if (GET_BIT(ntd->nodes[i].bag, v_i) && !vis_dfs[i])
    {
      dfs_trv_ntd(&(ntd->nodes[i]), v_i, vis_dfs, &(ntd->nodes[i]));
      if (++cc > 1) return TEST_NOK;
    }
  }
  ARR_FREE(vis_dfs);
  /* Test treewidth optimality */
  int ms = 0;
  for (int i = 0; i < ntd->b_cnt; i++)
  {
    int cs = 0;
    for (int j = 0; j < F_GRAPH->n_cnt; j++) if (GET_BIT(ntd->nodes[i].bag, j)) ++cs;
    ms = MAX(ms, cs);
  }
  if (ms - 1 != ntd->tw) return TEST_NOK;
  return TEST_OK;
}

/*---------------------------------------------------------------------------
 * Function: test_results
 *-------------------------------------------------------------------------*/ 
int test_results       (GRAPH_RESULT ** results)
{
  for (int i = 0; i < ARR_LEN(results); i++)
  {
    for (int j = 0; j < F_GRAPH->n_cnt; j++) if (results[i]->mapping[j] >= G_GRAPH->n_cnt) return TEST_NOK;
    for (int j = 0; j < F_GRAPH->n_cnt; j++) for (int k = j + 1; k < F_GRAPH->n_cnt; k++) 
    {
      if (graph_is_adj(F_GRAPH, j, k) && 
          !graph_is_adj(G_GRAPH, results[i]->mapping[j], results[i]->mapping[k])) return TEST_NOK; 
    }
  }
  return TEST_OK; 
}