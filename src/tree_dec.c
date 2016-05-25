/*
 *	Subgraph Isomorphism - Tree decomposition
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */
 
#include "tree_dec.h"
#include "util.h"
#include "graph.h"
#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int * tw_dp;

/****************************************************************************
 * STATIC FUNCTIONS
 ***************************************************************************/

/*---------------------------------------------------------------------------
 * Function: get_q_component
 *---------------------------------------------------------------------------
 * Finds all vertices in the connected component of G[S union x]
 *
 * Params:
 *   g           - used graph G
 *   S           - mask representing current subset of vertices
 *   x           - node whose component is being searched in G[S union x]
 *   q_component - array to save information about component to (initially zeroed)
 */
static void get_q_component(GRAPH * g, umask S, int x, int * q_component)
{
  if (q_component[x]) return;
  q_component[x] = 1;
  FOR_ADJ(g->edges[x], node)
  {
    int y = node->key;
    if (GET_BIT(S, y)) get_q_component(g, S, y, q_component);
  }
  FOR_ADJ_END;
}

/*---------------------------------------------------------------------------
 * Function: q_function
 *---------------------------------------------------------------------------
 * Calculates |Q(S,v)| by the definition in "On exact algorithms for treewidth"
 * on page 12:4.
 *
 * Params:
 *   g - used graph
 *   S - mask representing current subset of vertices
 *   v - node for which this function is calculated
 *
 * Returns:
 *   Result of described computation
 */
static int q_function (GRAPH * g, umask S, int v)
{
  int res = 0;
  int q_component[MAX_F_VERTICES];
  memset(q_component, 0, sizeof(q_component));
  get_q_component(g, S, v, q_component);
  for (int i = 0; i < g->n_cnt; i++)
  {
    int has = 0;
    if (!GET_BIT(S, i) && i != v)
    {
      FOR_ADJ(g->edges[i], node)
      {
        int y = node->key;
        if (q_component[y]) has = 1;
      }
      FOR_ADJ_END;
    }
    if (has) ++res;
  }
  return res;
}

/*---------------------------------------------------------------------------
 * Function: get_perm_dp
 *---------------------------------------------------------------------------
 * Returns treewidth of subgraph of g induced by vertices in S
 *
 * Params:
 *   g - used graph
 *   S - mask representing current subset of vertices
 *
 * Returns:
 *   Result of computation - corresponding treewidth
 */
static int get_perm_dp (GRAPH * g, umask S)
{
  if (!S) return -INF;
  if (tw_dp[S] != NOT_USED) return tw_dp[S];
  int res = NOT_USED;
  for (int i = 0; i < g->n_cnt; i++)
  {
    if (GET_BIT(S, i))
    {
      int tww = get_perm_dp(g, UNSET_BIT(S, i));
      int qw  = q_function(g, UNSET_BIT(S, i), i);
      int t_res = MAX(tww, qw);
      if (res == NOT_USED || t_res < res) res = t_res;
    }
  }
  return tw_dp[S] = res;
}

/*---------------------------------------------------------------------------
 * Function: get_best_perm
 *---------------------------------------------------------------------------
 * Retrieves elimination ordering with minimal treewidth from DP tables
 * previously filled by function 'get_perm_dp'
 *
 * Params:
 *   g - used graph
 *   S - mask representing current subset of vertices
 *   depth - current position in el. ordering
 *   perm - array to store resulting el. ordering
 */
static void get_best_perm (GRAPH * g, umask S, int depth, int * perm)
{
  if (!S) return;
  int res = INF, r_n;
  for (int i = 0; i < g->n_cnt; i++)
  {
    if (GET_BIT(S, i))
    {
      if (tw_dp[UNSET_BIT(S, i)] < res)
      {
        res = tw_dp[UNSET_BIT(S, i)];
        r_n = i;
      }
    }
  }
  perm[depth] = r_n;
  get_best_perm(g, UNSET_BIT(S, r_n), depth - 1, perm);
}

/*---------------------------------------------------------------------------
 * Function: get_inv_perm
 *---------------------------------------------------------------------------
 * Creates inverse el. ordering to the given one
 *
 * Params:
 *   g        - used graph
 *   perm     - given el. ordering
 *   inv_perm - array to store resulting inverse el. ordering
 */
static void get_inv_perm (GRAPH * g, int * perm, int * inv_perm)
{
  for (int i = 0; i < g->n_cnt; i++) inv_perm[perm[i]] = i;
}

/*---------------------------------------------------------------------------
 * Function: set_tree_dec_node
 *---------------------------------------------------------------------------
 * Sets the content of a tree decomposition node
 *
 * Params:
 *   td  - corresponding tree decomposition
 *   pos - id number of tree decomposition node
 *   content - bitmask of vertices contained in this node's bag
 *   adj - bitmask of adjacent tree decomposition nodes
 */
static void set_tree_dec_node (TREE_DEC * td, unsigned int pos, umask content, umask adj)
{
  td->nodes[pos].bag = content;
  td->nodes[pos].adj |= adj; /* Mask can be already filled by adjacent nodes */
  for (int i = 0; i < td->b_cnt; i++) if (GET_BIT(adj, i)) td->nodes[i].adj = SET_BIT(td->nodes[i].adj, pos);
}

/*---------------------------------------------------------------------------
 * Function: eliminate
 *---------------------------------------------------------------------------
 * Eliminates node v, i.e. adds edge between its neighbours that are ranked
 * higher in el. ordering
 *
 * Params:
 *   g        - used graph
 *   inv_perm - array to store resulting inverse el. ordering
 *   v        - vertex being eliminated
 */
static void eliminate (GRAPH * g, int * inv_perm, int v)
{
  FOR_ADJ(g->edges[v], node1)
  {
    FOR_ADJ(g->edges[v], node2)
    {
      int w = node1->key;
      int x = node2->key;
      if (inv_perm[w] > inv_perm[v] && inv_perm[x] > inv_perm[v] && !graph_is_adj(g, w, x))
      {
        graph_add_edge(g, w, x);
        graph_add_edge(g, x, w);
      }
    }
    FOR_ADJ_END;
  }
  FOR_ADJ_END;
}

/*---------------------------------------------------------------------------
 * Function: td_perm_rec
 *---------------------------------------------------------------------------
 * Core function for function 'td_from_perm', which constructs
 * tree decomposition from given graph and el. ordering in a recursive manner.
 *
 * Params:
 *   td       - tree decomposition for result storage
 *   g        - used graph
 *   perm     - el. ordering
 *   inv_perm - inverse el. ordering
 *   cp       - current number of processed nodes of graph g
 */
static void td_perm_rec (TREE_DEC * td, GRAPH * g, int * perm, int * inv_perm, int cp)
{
  if (cp == td->b_cnt - 1)
  {
    set_tree_dec_node(td, perm[cp], SET_BIT(EMPTY_MASK, perm[cp]), EMPTY_MASK);
    return;
  }
  int low_nbr_pos = INF;
  umask vbag = SET_BIT(EMPTY_MASK, perm[cp]);
  FOR_ADJ(g->edges[perm[cp]], node)
  {
    int w = node->key;
    if (inv_perm[w] > inv_perm[perm[cp]])
    {
      low_nbr_pos = MIN(low_nbr_pos, inv_perm[w]);
      vbag = SET_BIT(vbag, w);
    }
  }
  FOR_ADJ_END;
  eliminate(g, inv_perm, perm[cp]);
  td_perm_rec(td, g, perm, inv_perm, cp + 1);
  set_tree_dec_node(td, perm[cp], vbag, SET_BIT(EMPTY_MASK, perm[low_nbr_pos]));
}

/*---------------------------------------------------------------------------
 * Function: td_from_perm
 *---------------------------------------------------------------------------
 * Creates a tree decomposition from given graph and el. ordering. Final
 * number of elements in individual bags depends on the given el. ordering.
 *
 * Params:
 *   g    - used graph
 *   perm - el. ordering
 *
 * Returns:
 *   Pointer to the tree decomposition
 */
static TREE_DEC * td_from_perm (GRAPH * g, int * perm)
{
  TREE_DEC * tmp = (TREE_DEC *)xmalloc(sizeof(*tmp));
  tmp->b_cnt = g->n_cnt;
  for (int i = 0; i < tmp->b_cnt; ++i) tmp->nodes[i].bag = tmp->nodes[i].adj = EMPTY_MASK;
  int inv_perm[MAX_F_VERTICES];
  get_inv_perm(g, perm, inv_perm);
  td_perm_rec(tmp, g, perm, inv_perm, 0);
  return tmp;
}

/****************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************/

/*---------------------------------------------------------------------------
 * Function: td_get
 *
 * Description:
 *   Tree decomposition is constructed from el. ordering, which is retrieved
 *   at the start of the function
 *-------------------------------------------------------------------------*/
TREE_DEC * td_get (GRAPH * g)
{
  ARR_ALLOC(tw_dp, 1U << g->n_cnt);
  memset(tw_dp, -1, ARR_LEN(tw_dp) * sizeof(*tw_dp));
  int tw = get_perm_dp(g, (1U << g->n_cnt) - 1);
  int best_perm[MAX_F_VERTICES];
  get_best_perm(g, (1U << g->n_cnt) - 1, g->n_cnt - 1, best_perm);

  DBG("TW = %d", tw);
  DBG("ES:");
  for (int i = 0; i < g->n_cnt; i++) DBG("%d", best_perm[i]);

  TREE_DEC * tmp = td_from_perm(g, best_perm);
  tmp->tw = tw;
  ARR_FREE(tw_dp);
  return tmp;
}

/*---------------------------------------------------------------------------
 * Function: td_free
 *-------------------------------------------------------------------------*/
void td_free (TREE_DEC * td)
{
  xfree(td);
}

/*---------------------------------------------------------------------------
 * Function: td_print
 *-------------------------------------------------------------------------*/
void td_print (TREE_DEC * td)
{
#ifdef LOCAL_DEBUG
  fprintf(stderr,"TD:\n");
  fprintf(stderr,"---------\n");
  for (int i = 0; i < td->b_cnt; i++)
  {
    fprintf(stderr,"BAG #%d:\n", i);
    fprintf(stderr,"Bag:");
    for (int j = 0; j < MAX_F_VERTICES; j++) if (GET_BIT(td->nodes[i].bag, j)) fprintf(stderr," %d", j);
    fprintf(stderr,"\n");
    fprintf(stderr,"Edges:");
    for (int j = 0; j < MAX_F_VERTICES; j++) if (GET_BIT(td->nodes[i].adj, j)) fprintf(stderr," B%d", j);
    fprintf(stderr,"\n\n");
  }
  fprintf(stderr,"---------\n");
#endif
}
