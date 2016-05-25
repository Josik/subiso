/*
 *	Subgraph Isomorphism - Isomorphism search
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "subiso.h"
#include "resbuf.h"
#include "nice_tree_dec.h"
#include "array.h"
#include "graph.h"
#include "graph_result.h"
#include "util.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

/* Map comparison defines */
#define MAP_EQUAL    0
#define MAP_LESS     1
#define MAP_GREATER  2

/* Libucw rb-tree defines (for sorting during introduce/forget node) */
typedef struct
{
  u32 * key;
  u32 * val;
} subiso_tree_node;

typedef struct rbtree_subiso_tree SUBISO_TREE;

#define TREE_NODE subiso_tree_node
#define TREE_PREFIX(x) rbtree_subiso_##x
#define TREE_KEY_ATOMIC key
#define TREE_ATOMIC_TYPE u32 *
#define TREE_WANT_NEW
#define TREE_WANT_LOOKUP
#define TREE_WANT_CLEANUP
#define TREE_WANT_REMOVE
#define TREE_WANT_FIND
#define TREE_WANT_ITERATOR

#define TREE_GIVE_CMP(key1, key2)
  int TREE_PREFIX(cmp) (TREE_ATOMIC_TYPE key1, TREE_ATOMIC_TYPE key2)
  {
    for (int i = 0; i < ARR_LEN(key1); i++)
    {
      if ((key1)[i] < (key2)[i]) return -1;
      else if ((key1)[i] > (key2)[i]) return 1;
    }
    return 0;
  }

#define TREE_GIVE_INIT_KEY
	void TREE_PREFIX(init_key) (TREE_NODE *n, TREE_ATOMIC_TYPE k)
	{
	  ARR_ALLOC(n->key, ARR_LEN(k));
	  memcpy(n->key, k, sizeof(*k) * ARR_LEN(k));
	}

#define TREE_GIVE_INIT_DATA
	void TREE_PREFIX(init_data) (TREE_NODE *n)
	{
	  ARR_INIT(n->val);
	}

#define FOR_SUBISO(tree, x) TREE_FOR_ALL(rbtree_subiso, tree, x)
#define FOR_SUBISO_END TREE_END_FOR

#include <ucw/redblack.h>

/* Libucw hash table defines (for operations in introduce node) */
typedef struct
{
  u32 key;
  u32 val;
} pair_table_node;

typedef struct table_pair_table PAIR_TABLE;

#define HASH_TABLE_DYNAMIC
#define HASH_NODE pair_table_node
#define HASH_PREFIX(x) table_pair_##x
#define HASH_KEY_ATOMIC key
#define HASH_WANT_NEW
#define HASH_WANT_LOOKUP
#define HASH_WANT_CLEANUP
#define HASH_WANT_REMOVE
#define HASH_WANT_FIND
#define HASH_WANT_ITERATOR

#define FOR_PAIR(tree, x) HASH_FOR_ALL_DYNAMIC(table_pair, tree, x)
#define FOR_PAIR_END      HASH_END_FOR

#include <ucw/hashtable.h>

/* -------------------------
 * Macro: FREE_FORGET_TREE
 * -------------------------
 * Frees all nodes from a forget tree
 *
 * Params:
 *   tree - tree to be freed
 */
#define FREE_SUBISO_TREE(tree)    \
  ({                              \
     FOR_SUBISO(tree, node)       \
     {                            \
       ARR_FREE(node->key);       \
       ARR_FREE(node->val);       \
     }                            \
     FOR_SUBISO_END;              \
     rbtree_subiso_cleanup(tree); \
  })

/* Libucw u32 sorter defines */
#define ASORT_PREFIX(X) u32arr_##X
#define ASORT_KEY_TYPE  u32
#include <ucw/sorter/array-simple.h>

/* Libucw int sorter defines */
#define ASORT_PREFIX(X) intarr_##X
#define ASORT_KEY_TYPE  int
#include <ucw/sorter/array-simple.h>

/* -------------------------
 * Macro: FREE_TRANS_ARR
 * -------------------------
 * Releases space allocated for transfer arrays used for decoding
 * from result buffers.
 *
 * Params:
 *   none
 */
#define FREE_TRANS_ARR(x)         \
        ({                        \
           ARR_FREE(map_old_1);   \
           ARR_FREE(col_old_1);   \
           ARR_FREE(map_old_2);   \
           ARR_FREE(col_old_2);   \
        })                        \

int * COLOUR;

/****************************************************************************
 * STATIC FUNCTIONS
 ***************************************************************************/

 /* -------------------------
 * Function: subiso_colouring
 * -------------------------
 * Randomly assigns |V(F_GRAPH)| colours to nodes of G_GRAPH.
 */
static void subiso_colouring (void)
{
  for (int i = 0; i < G_GRAPH->n_cnt; i++) COLOUR[i] = rand() % F_GRAPH->n_cnt;
}

/* -------------------------
 * Macro: NODE_CONSISTENT
 * -------------------------
 * Checks, whether node with given colour has not already been used in result construction.
 *
 * Params:
 *   x   - node of G_GRAPH to be checked
 *   col - mask of currently used colours
 *
 * Returns:
 *   0 if node has already been used
 */
#define NODE_CONSISTENT(x, col)  (!(SET_BIT((col), COLOUR[(x)]) == (col)))

/* -------------------------
 * Function: edge_consistent
 * -------------------------
 * Cheks, wheter subgraph construction doesn't make invalid assigment
 * edgewise.
 *
 * Params:
 *   chng_index - position of currently changed element in bag
 *   bag_cont   - bag content of corresponding ntd node
 *   map        - mapping assigned to this node
 *
 * Returns:
 *   0 if construction is invalid
 */
static int edge_consistent (int chng_index, int * bag_cont, u32 * map)
{
  for (int i = 0; i < ARR_LEN(map); i++)
  {
    if (i != chng_index)
    {
      if (graph_is_adj(F_GRAPH, bag_cont[chng_index], bag_cont[i]) &&
          !graph_is_adj(G_GRAPH, map[chng_index], map[i])) return 0;
    }
  }
  return 1;
}

/* -------------------------
 * Function: map_compare
 * -------------------------
 * Compares arrays of u32 lexicographicaly
 *
 * Params:
 *   map_1 - first array
 *   map_2 - second array
 *
 * Returns:
 *   MAP_LESS/MAP_GREATED/MAP_EQUAL in corresponding situations
 */
static int map_compare (u32 * map_1, u32 * map_2)
{
  for (int i = 0; i < ARR_LEN(map_1); i++)
  {
    if (map_1[i] < map_2[i]) return MAP_LESS;
    else if (map_1[i] > map_2[i]) return MAP_GREATER;
  }
  return MAP_EQUAL;
}

/* -------------------------
 * MACRO: COL_OK
 * -------------------------
 * Checks, wheter colours used in ntd join node's subtrees contain enough colours
 * for assigment in this node.
 *
 * Params:
 *   mcol  - colors of mapping assigment in this ntd node
 *   col_1 - bitmask of colours used in node's left subtree
 *   col_2 - bitmask of colours used in node's right subtree
 *
 * Returns:
 *   0 if colour bitmasks are invalid
 */

#define COL_OK(mcol, col_1, col_2) ((col_1 & col_2) == mcol)
 
/* -------------------------
 * Function: col_uniq
 * -------------------------
 * Moves duplicate elements from the given array of colour bitmasks
 * to the end of the array (and also puts unique elements in sorted order)
 *
 * Params:
 *   col - array to be uniq'd
 *   len - number of elements to be considered
 *
 * Returns:
 *   Number of unique elements
 */
static u32 col_uniq (u32 * col)
{
  u32arr_sort(col, ARR_LEN(col));
  u32 wi;
  wi = 0;
  for (int ri = 0; ri < ARR_LEN(col); ri++)
  {
    if (!ri || col[ri] != col[ri - 1]) col[wi++] = col[ri];
  }
  return wi;
}

/* -------------------------
 * Function: subiso_leaf
 * -------------------------
 * Handles main algorithm mapping in leaf nodes.
 *
 * Params:
 *   x       - current leaf node being processed
 *   r_new   - buffer to store outgoing information to
 */
static void subiso_leaf (NICE_TREE_DEC_NODE * x, RESBUF * r_new)
{
#ifdef LOCAL_DEBUG
  DBG("~~~~~~~~~ L ~~~~~~~~~");
  ntd_print_node(x);
#endif

  u32 * map_new, * col_new;

  ARR_ALLOC(map_new, 1);
  ARR_ALLOC(col_new, 1);
  
  for (int i = 0; i < G_GRAPH->n_cnt; i++)
  {
    map_new[0] = i;
    col_new[0] = SET_BIT(EMPTY_MASK, COLOUR[i]);
    resbuf_push(r_new, map_new, 1, col_new, 1);
  }
  ARR_FREE(map_new);
  ARR_FREE(col_new);
}


/* -------------------------
 * Function: subiso_introduce
 * -------------------------
 * Handles main algorithm mapping in introduce nodes.
 *
 * Params:
 *   x     - current leaf node being processed
 *   r_old - buffer storing ingoing information from child node
 *   r_new - buffer to store outgoing information to
 */
static void subiso_introduce (NICE_TREE_DEC_NODE * x, RESBUF * r_old, RESBUF * r_new)
{
#ifdef LOCAL_DEBUG
  DBG("~~~~~~~~~ I ~~~~~~~~~");
  ntd_print_node(x);
#endif

  u32 * map_old, * map_new, * col_old, * col_new, * prefix, * prefix_prev, * suffix;
  u32 mlen_old, mlen_new, clen_old, clen_new, prefix_len, suffix_len;
  int * s, *q, s_cnt, u, f_ind_deg, qt, qh;
  
  PAIR_TABLE * pair_mem;
  SUBISO_TREE * ft;

  mlen_new = ARR_LEN(x->bag_cont);
  mlen_old = mlen_new - 1;
  ARR_ALLOC(map_new, mlen_new);
  ARR_ALLOC(map_old, mlen_old);
  ARR_ALLOC(col_new, 1 << F_GRAPH->n_cnt);
  ARR_ALLOC(col_old, 1 << F_GRAPH->n_cnt);
  ARR_ALLOC(s, G_GRAPH->n_cnt);
  ARR_ALLOC(q, G_GRAPH->n_cnt);
  pair_mem = (PAIR_TABLE *)xmalloc(sizeof(*pair_mem));

  prefix_len = x->chng_index;
  ARR_ALLOC(prefix, prefix_len + 1);
  ARR_ALLOC(prefix_prev, prefix_len + 1);
  for (int i = 0; i < prefix_len + 1; i++) prefix[i] = prefix_prev[i] = INF;
  suffix_len = mlen_new - prefix_len;
  ARR_ALLOC(suffix, suffix_len);
  ft = (SUBISO_TREE * )xmalloc(sizeof(*ft));
  rbtree_subiso_init(ft);
  u = x->bag_cont[x->chng_index];
  while (resbuf_read(r_old, map_old, mlen_old, col_old, &clen_old) != RES_EOF)
  {
    if (prefix_len) memcpy(prefix, map_old, prefix_len * sizeof(*prefix));
    if (map_compare(prefix, prefix_prev) != MAP_EQUAL) /* New prefix -> push results of the last one */
    {
      FOR_SUBISO(ft, node)
      {
        clen_new = col_uniq(node->val);
        if (clen_new)
        {
          memcpy(map_new + prefix_len, node->key, suffix_len * sizeof(*node->key));
          resbuf_push(r_new, map_new, mlen_new, node->val, clen_new);
        }
      }
      FOR_SUBISO_END;
      FREE_SUBISO_TREE(ft);
      rbtree_subiso_init(ft);
      if (prefix_len) memcpy(map_new, prefix, prefix_len * sizeof(*prefix));
    }
    if (suffix_len - 1) memcpy(suffix + 1, map_old + prefix_len, (suffix_len - 1) * sizeof(*map_old)); /* suffix_len is always >= 1 */
    /* Try all assigments */
    memcpy(map_new + prefix_len, suffix, suffix_len * sizeof(*suffix));
    s_cnt = 0;
    int f_ind_deg = 0;
    for (int i = 0; i < mlen_new; i++) /* Find degree of u in F[V_x]*/
    {
      if (i != x->chng_index && graph_is_adj(F_GRAPH, u, x->bag_cont[i])) ++f_ind_deg;
    }
    
    if (f_ind_deg > 0) /* Map opt #1 */
    {
      pair_table_node * node;
      table_pair_init(pair_mem);
      int req = 0; /* Number of neighbors required so far */
      for (int i = 0; i < mlen_new; i++) 
      {
        if (i != x->chng_index && graph_is_adj(F_GRAPH, u, x->bag_cont[i]))
        {
          int phi_w = map_new[i];
          FOR_ADJ(G_GRAPH->edges[phi_w], neigh)
          {
            int w = neigh->key;
            node = table_pair_find(pair_mem, w);
            if (!node && !req) 
            {
              node = table_pair_new(pair_mem, w);
              node->val = 0;
            }
            if (node) ++node->val;
          }
          FOR_ADJ_END;
          ++req; 
        }
      }
      FOR_PAIR(pair_mem, px)
      {
        if (px->val == f_ind_deg) s[s_cnt++] = px->key;
      }
      FOR_PAIR_END;
      table_pair_cleanup(pair_mem);
    }
    else /* Map opt #2 */
    {
      int min_ecc = F_GRAPH->n_cnt;
      int min_vi = 0;
      for (int i = 0; i < mlen_new; i++) 
      {
        if (i != x->chng_index)
        {
          if (F_ECC[x->bag_cont[i]] < min_ecc)
          {
            min_ecc = F_ECC[x->bag_cont[i]];
            min_vi = i;
          }
        }
      }
      
      pair_table_node * node;
      table_pair_init(pair_mem);
      qt = qh = 0;
      q[qh++] = map_new[min_vi];
      node = table_pair_new(pair_mem, map_new[min_vi]);
      node->val = 0;
      while (qt < qh)
      {
        int x = q[qt++];
        s[s_cnt++] = x;
        node = table_pair_find(pair_mem, x);
        int cd = node->val;
        if (cd >= min_ecc) continue;
        FOR_ADJ(G_GRAPH->edges[x], neigh)
        {
          int y = neigh->key;
          node = table_pair_find(pair_mem, y);
          if (!node)
          {
            node = table_pair_new(pair_mem, y);
            node->val = cd + 1;
            q[qh++] = y;
          }
        }
        FOR_ADJ_END;
      }
      table_pair_cleanup(pair_mem);
    }
    
    for (int i = 0; i < s_cnt; i++)
    {
      *suffix = map_new[x->chng_index] = s[i];
      clen_new = 0;
      for (int j = 0; j < clen_old; j++)
      {
        if (NODE_CONSISTENT(s[i], col_old[j])) col_new[clen_new++] = SET_BIT(col_old[j], COLOUR[s[i]]);
      }
      if (clen_new)
      {
        subiso_tree_node * t_node = rbtree_subiso_lookup(ft, suffix);
        for (int j = 0; j < clen_new; j++) ARR_PUSH(t_node->val, col_new[j]);
      }
    }
    memcpy(prefix_prev, prefix, prefix_len * sizeof(*prefix));
  }
  /* Push results of the last prefix */
  FOR_SUBISO(ft, node)
  {
    clen_new = col_uniq(node->val);
    if (clen_new)
    {
      memcpy(map_new + prefix_len, node->key, suffix_len * sizeof(*node->key));
      resbuf_push(r_new, map_new, mlen_new, node->val, clen_new);
    }
  }
  FOR_SUBISO_END;
  FREE_SUBISO_TREE(ft);
  xfree(ft);
  xfree(pair_mem);
  ARR_FREE(q);
  ARR_FREE(s);
  ARR_FREE(map_new);
  ARR_FREE(map_old);
  ARR_FREE(col_new);
  ARR_FREE(col_old);
  ARR_FREE(prefix);
  ARR_FREE(prefix_prev);
  ARR_FREE(suffix);
}

/* -------------------------
 * Function: subiso_forget
 * -------------------------
 * Handles main algorithm mapping in forget nodes.
 *
 * Params:
 *   x     - current leaf node being processed
 *   r_old - buffer storing ingoing information from child node
 *   r_new - buffer to store outgoing information to
 */
static void subiso_forget (NICE_TREE_DEC_NODE * x, RESBUF * r_old, RESBUF * r_new)
{
#ifdef LOCAL_DEBUG
  DBG("~~~~~~~~~ F ~~~~~~~~~");
  ntd_print_node(x);
#endif

  u32 * map_old, * map_new, * col_old, * prefix, * prefix_prev, * suffix;
  u32 mlen_old, mlen_new, clen_old, clen_new, prefix_len, suffix_len;
  SUBISO_TREE * ft;

  mlen_new = ARR_LEN(x->bag_cont);
  mlen_old = mlen_new + 1;
  ARR_ALLOC(map_new, mlen_new);
  ARR_ALLOC(map_old, mlen_old);
  ARR_ALLOC(col_old, 1 << F_GRAPH->n_cnt);

  prefix_len = x->chng_index;
  ARR_ALLOC(prefix, prefix_len + 1);
  ARR_ALLOC(prefix_prev, prefix_len + 1);
  for (int i = 0; i < prefix_len + 1; i++) prefix[i] = prefix_prev[i] = INF;
  suffix_len = mlen_new - prefix_len;
  ARR_ALLOC(suffix, suffix_len + 1);
  for (int i = 0; i < suffix_len + 1; i++) suffix[i] = INF;
  ft = (SUBISO_TREE * )xmalloc(sizeof(*ft));
  rbtree_subiso_init(ft);
  while (resbuf_read(r_old, map_old, mlen_old, col_old, &clen_old) != RES_EOF)
  {
    if (prefix_len) memcpy(prefix, map_old, prefix_len * sizeof(*prefix));
    if (map_compare(prefix, prefix_prev) != MAP_EQUAL) /* New prefix -> push results of the last one */
    {
      FOR_SUBISO(ft, node)
      {
        clen_new = col_uniq(node->val);
        if (clen_new)
        {
          if (suffix_len) memcpy(map_new + prefix_len, node->key, suffix_len * sizeof(*map_new));
          resbuf_push(r_new, map_new, mlen_new, node->val, clen_new);
        }
      }
      FOR_SUBISO_END;
      FREE_SUBISO_TREE(ft);
      rbtree_subiso_init(ft);
      if (prefix_len) memcpy(map_new, prefix, prefix_len * sizeof(*prefix));
    }
    if (suffix_len) memcpy(suffix, map_old + prefix_len + 1, suffix_len * sizeof(*suffix));
    subiso_tree_node * node = rbtree_subiso_lookup(ft, suffix);
    for (int i = 0; i < clen_old; i++) ARR_PUSH(node->val, col_old[i]);
    memcpy(prefix_prev, prefix, prefix_len * sizeof(*prefix));
  }
  /* Push results of the last prefix */
  FOR_SUBISO(ft, node)
  {
    clen_new = col_uniq(node->val);
    if (clen_new)
    {
      if (suffix_len) memcpy(map_new + prefix_len, node->key, suffix_len * sizeof(*map_new));
      resbuf_push(r_new, map_new, mlen_new, node->val, clen_new);
    }
  }
  FOR_SUBISO_END;
  FREE_SUBISO_TREE(ft);
  xfree(ft);
  ARR_FREE(map_new);
  ARR_FREE(map_old);
  ARR_FREE(col_old);
  ARR_FREE(prefix);
  ARR_FREE(prefix_prev);
  ARR_FREE(suffix);
}

/* -------------------------
 * Function: subiso_join
 * -------------------------
 * Handles main algorithm mapping in join nodes.
 *
 * Params:
 *   x       - current leaf node being processed
 *   r_old_1 - buffer storing ingoing information from child node #1
 *   r_old_2 - buffer storing ingoing information from child node #2
 *   r_new   - buffer to store outgoing information to
 */
static void subiso_join (NICE_TREE_DEC_NODE * x, RESBUF * r_old_1, RESBUF * r_old_2, RESBUF * r_new)
{
#ifdef LOCAL_DEBUG
  DBG("~~~~~~~~~ J ~~~~~~~~~");
  ntd_print_node(x);
#endif

  u32 * map_old_1, * map_old_2, * col_old_1, * col_old_2, * col_new, * col_tmp;
  u32 mlen_new, mlen_old, clen_new, clen_old_1, clen_old_2;

  mlen_new = mlen_old = ARR_LEN(x->bag_cont);
  ARR_ALLOC(map_old_1, mlen_old);
  ARR_ALLOC(map_old_2, mlen_old);
  ARR_ALLOC(col_old_1, 1 << F_GRAPH->n_cnt);
  ARR_ALLOC(col_old_2, 1 << F_GRAPH->n_cnt);

  if (resbuf_read(r_old_1, map_old_1, mlen_old, col_old_1, &clen_old_1) == RES_EOF ||
      resbuf_read(r_old_2, map_old_2, mlen_old, col_old_2, &clen_old_2) == RES_EOF)
  {
    FREE_TRANS_ARR();
    return;
  }
  while (1)
  {
    int cmpr = map_compare(map_old_1, map_old_2);
    if (cmpr == MAP_LESS)
    {
      if (resbuf_read(r_old_1, map_old_1, mlen_old, col_old_1, &clen_old_1) == RES_EOF) break;
    }
    else if (cmpr == MAP_GREATER)
    {
      if (resbuf_read(r_old_2, map_old_2, mlen_old, col_old_2, &clen_old_2) == RES_EOF) break;
    }
    else
    {
      ARR_INIT(col_new);
      umask map_col = EMPTY_MASK;
      for (int i = 0; i < ARR_LEN(map_old_1); i++) map_col = SET_BIT(map_col, COLOUR[map_old_1[i]]);
      for (int i = 0; i < clen_old_1; i++) for (int j = 0; j < clen_old_2; j++)
      {
        if (COL_OK(map_col, col_old_1[i], col_old_2[j])) ARR_PUSH(col_new, col_old_1[i] | col_old_2[j]);
      }
      clen_new = col_uniq(col_new);
      if (clen_new)
      {
        resbuf_push(r_new, map_old_1, mlen_old, col_new, clen_new);
      }
      ARR_FREE(col_new);

      if (resbuf_read(r_old_1, map_old_1, mlen_old, col_old_1, &clen_old_1) == RES_EOF ||
          resbuf_read(r_old_2, map_old_2, mlen_old, col_old_2, &clen_old_2) == RES_EOF) break;
    }
  }
  FREE_TRANS_ARR();
}

/* -------------------------
 * Function: subiso_dp
 * -------------------------
 * Main algorithm filling DP tables in recursive manner bottom->top.
 *
 * Params:
 *   x    - current node being processed
 *
 * Returns:
 *   Buffer with outgoing information about valid assigments (DP table record) in this node.
 */
static RESBUF * subiso_dp (NICE_TREE_DEC_NODE * x)
{
  RESBUF * r_old_1, * r_old_2;
  r_old_1 = r_old_2 = NULL;
  resbuf_free(x->rbuf); /* If dp record was already filled in previous iteration */
  x->rbuf = resbuf_init();
  resbuf_chng_state(x->rbuf, RES_WRITE);
  switch (x->type)
  {
    case LEAF_NODE:
      {
        subiso_leaf(x, x->rbuf);
        break;
      }
    case INTRODUCE_NODE:
      {
        r_old_1 = subiso_dp(x->child_1);
        subiso_introduce(x, r_old_1, x->rbuf);
        break;
      }
    case FORGET_NODE:
      {
        r_old_1 = subiso_dp(x->child_1);
        subiso_forget(x, r_old_1, x->rbuf);
        break;
      }
    case JOIN_NODE:
      {
        r_old_1 = subiso_dp(x->child_1);
        r_old_2 = subiso_dp(x->child_2);
        subiso_join(x, r_old_1, r_old_2, x->rbuf);
        break;
      }
    default:
        break;
  }
  resbuf_chng_state(x->rbuf, RES_READ);
  return x->rbuf;
}

/* -------------------------
 * Function: can_add
 * -------------------------
 * Checks, whether a partial assigment in the current node can be added to an existing result
 *
 * Params:
 *   x       - current node being processed
 *   pos     - position of bag element (from F_GRAPH) that is being assigned node in G_GRAPH
 *   map     - assigment to all elements of this node's bag
 *   res_old - current result that is to be updated
 *
 * Returns:
 *   Graph result structure with assigment (possibly partial) if assigment is possible,
 *   or NULL otherwise
 */
static GRAPH_RESULT * can_add (NICE_TREE_DEC_NODE * x, int pos, u32 * map, GRAPH_RESULT * res_old)
{
  /* Check if the new mapping doesn't contradict used colours so far */
  if (!NODE_CONSISTENT(map[pos], res_old->used_cols)) return NULL;
  /* Check if the mapping that induces newly mapped node actually corresponds to the mapping retrieved so far */
  for (int j = 0; j < ARR_LEN(map); j++)
  {
    int mn = res_old->mapping[x->bag_cont[j]];
    if (j == pos)
    {
      if (mn != INF) return NULL;
    }
    else
    {
      if (mn != INF && mn != map[j]) return NULL;
    }
  }
  GRAPH_RESULT * res_new = graph_result_init(res_old->g);
  for (int i = 0; i < ARR_LEN(res_old->mapping); i++) res_new->mapping[i] = res_old->mapping[i];
  res_new->mapping[x->bag_cont[pos]] = map[pos];
  res_new->used_cols = SET_BIT(res_old->used_cols, COLOUR[map[pos]]);
  return res_new;
}

/* -------------------------
 * Function: subiso_reconstruct
 * -------------------------
 * Reconstructs all found subgraphs by recursively examining already filled DP tables
 *
 * Params:
 *   x       - current node being processed
 *   results - partial results from parent in nice tree decomposition
 *
 * Returns:
 *   Array of (possibly partial) results after considering all valid assigments in this node
 */
static GRAPH_RESULT ** subiso_reconstruct (NICE_TREE_DEC_NODE * x, GRAPH_RESULT ** results)
{
  GRAPH_RESULT ** child_results, ** tmp_results;
  switch (x->type)
  {
    case LEAF_NODE:
    {
#ifdef LOCAL_DEBUG
      DBG("~~~~~~~~~ R(L) ~~~~~~~~~");
      ntd_print_node(x);
#endif
      child_results = results;
      break;
    }
    case INTRODUCE_NODE:
    {
#ifdef LOCAL_DEBUG
      DBG("~~~~~~~~~ R(I) ~~~~~~~~~");
      ntd_print_node(x);
#endif
      child_results = subiso_reconstruct(x->child_1, results);
      break;
    }
    case FORGET_NODE:
    {
#ifdef LOCAL_DEBUG
      DBG("~~~~~~~~~ R(F) ~~~~~~~~~");
      ntd_print_node(x);
#endif
      u32 * map, * col;
      u32 mlen, clen;

      mlen = ARR_LEN(x->child_1->bag_cont);
      ARR_ALLOC(map, mlen);
      ARR_ALLOC(col, 1 << F_GRAPH->n_cnt);

      ARR_INIT(tmp_results);
      resbuf_chng_state(x->child_1->rbuf, RES_READ);
      while (resbuf_read(x->child_1->rbuf, map, mlen, col, &clen) != RES_EOF)
      {
        for (int i = 0; i < ARR_LEN(results); i++)
        {
          GRAPH_RESULT * added_res = can_add(x->child_1, x->chng_index, map, results[i]);
          if (added_res) ARR_PUSH(tmp_results, added_res);
        }
      }
      child_results = subiso_reconstruct(x->child_1, tmp_results);
      graph_result_array_free(results);
      ARR_FREE(map);
      ARR_FREE(col);
      break;
    }
    case JOIN_NODE:
    {
#ifdef LOCAL_DEBUG
      DBG("~~~~~~~~~ R(J) ~~~~~~~~~");
      ntd_print_node(x);
#endif
      tmp_results = subiso_reconstruct(x->child_1, results);
      child_results = subiso_reconstruct(x->child_2, tmp_results);
      break;
    }
    default:
      break;
  }
  return child_results;
}

/****************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************/

 /*---------------------------------------------------------------------------
 * Function: subiso_run
 *-------------------------------------------------------------------------*/
GRAPH_RESULT ** subiso_run (NICE_TREE_DEC * ntd, int rep_cnt)
{
  GRAPH_RESULT ** run_result, ** tmp_result;
  ARR_ALLOC(COLOUR, G_GRAPH->n_cnt);
  graph_result_glmemory_init(F_GRAPH);
  for (int i = 0; i < rep_cnt; i++)
  {
    clock_t start = clock();
    subiso_colouring();
    subiso_dp(&(ntd->nodes[ntd->root]));
    ARR_INIT(tmp_result);
    GRAPH_RESULT * dummy = graph_result_init(F_GRAPH);
    ARR_PUSH(tmp_result, dummy);
    run_result = subiso_reconstruct(&(ntd->nodes[ntd->root]), tmp_result);
    graph_result_glmemory_add(run_result);
    clock_t end = clock();
    A_TIME += end - start;
    if (i % 1000); else printf(">>> UNIQUE subgraphs so far after run #%d = %d <<<\n", i + 1, graph_result_glmemory_size());
  }
  printf("~~~~~~END~~~~~~\n");
  ARR_FREE(COLOUR);
  return graph_result_glmemory_reconstruct();
}
