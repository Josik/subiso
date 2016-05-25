/*
 *	Subgraph Isomorphism - Graph results
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */
 
#include "graph_result.h"
#include "array.h"
#include "graph.h"
#include "util.h"
#include <stdio.h>

/* Libucw hash table defines (for uniq-ing results) */
typedef struct
{
  u32 *          key;
  GRAPH_RESULT * val;
} gresult_table_node;

typedef struct table_gresult_table GRESULT_TABLE;

#define HASH_TABLE_DYNAMIC
#define HASH_NODE gresult_table_node
#define HASH_PREFIX(x) table_gresult_##x
#define HASH_KEY_ATOMIC key
#define HASH_ATOMIC_TYPE u32 *
#define HASH_WANT_NEW
#define HASH_WANT_LOOKUP
#define HASH_WANT_CLEANUP
#define HASH_WANT_REMOVE
#define HASH_WANT_FIND
#define HASH_WANT_ITERATOR

#define HASH_GIVE_HASHFN
  uint HASH_PREFIX(hash) (GRESULT_TABLE * t, HASH_ATOMIC_TYPE k)
  {
    uint h = 0;
    for (int i = 0; i < ARR_LEN(k); i++) h += 19 * h + k[i];
    return h;
  }

#define HASH_GIVE_EQ
  int HASH_PREFIX(eq) (GRESULT_TABLE * t, HASH_ATOMIC_TYPE k1, HASH_ATOMIC_TYPE k2)
  {
    for (int i = 0; i < ARR_LEN(k1); i++)
    {
      if ((k1)[i] != (k2)[i]) return 0;
    }
    return 1;
  }

#define HASH_GIVE_INIT_KEY
	void HASH_PREFIX(init_key) (GRESULT_TABLE * t, HASH_NODE *n, HASH_ATOMIC_TYPE k)
	{
	  ARR_ALLOC(n->key, ARR_LEN(k));
	  memcpy(n->key, k, sizeof(*k) * ARR_LEN(k));
	}

#define HASH_GIVE_INIT_DATA
	void HASH_PREFIX(init_data) (GRESULT_TABLE * t, HASH_NODE *n)
	{
	  n->val = NULL;
	}

#define FOR_GRESULT(tree, x) HASH_FOR_ALL_DYNAMIC(table_gresult, tree, x)
#define FOR_GRESULT_END      HASH_END_FOR

#include <ucw/hashtable.h>

/* Hashtable used for uniquing results - global memory for unique results */
GRESULT_TABLE * result_mem;
/* Sort buffer for uniquing results */
int           * sort_buf;

/* Libucw sorter defines */
#define ASORT_PREFIX(X) intarr_##X
#define ASORT_KEY_TYPE  int
#include <ucw/sorter/array-simple.h>

/****************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************/

/*---------------------------------------------------------------------------
 * Function: graph_result_init 
 *-------------------------------------------------------------------------*/ 
GRAPH_RESULT * graph_result_init (GRAPH * g)
{
  GRAPH_RESULT * tmp = (GRAPH_RESULT *)xmalloc(sizeof(*tmp));
  tmp->used_cols = EMPTY_MASK;
  tmp->g = g;
  ARR_ALLOC(tmp->mapping, g->n_cnt);
  for (int i = 0; i < tmp->g->n_cnt; i++) tmp->mapping[i] = INF;
  return tmp;
}

/*---------------------------------------------------------------------------
 * Function: graph_result_glmemory_init 
 *-------------------------------------------------------------------------*/ 
void graph_result_glmemory_init (GRAPH * g)
{
  result_mem = (GRESULT_TABLE *)xmalloc(sizeof(*result_mem));
  table_gresult_init(result_mem);
  ARR_ALLOC(sort_buf, g->n_cnt);
}

/*---------------------------------------------------------------------------
 * Function: graph_result_glmemory_add
 *-------------------------------------------------------------------------*/
void graph_result_glmemory_add (GRAPH_RESULT ** gra)
{
  for (int i = 0; i < ARR_LEN(gra); i++)
  {
    memcpy(sort_buf, gra[i]->mapping, gra[i]->g->n_cnt * sizeof(*gra[i]->mapping));
    intarr_sort(sort_buf, ARR_LEN(sort_buf));
    gresult_table_node * node = table_gresult_lookup(result_mem, sort_buf);
    if (!node->val) node->val = gra[i];
    else graph_result_free(gra[i]);
  }
  ARR_FREE(gra);
}

/*---------------------------------------------------------------------------
 * Function: graph_result_glmemory_reconstruct
 *-------------------------------------------------------------------------*/
GRAPH_RESULT ** graph_result_glmemory_reconstruct (void)
{
  GRAPH_RESULT ** tmp;
  ARR_INIT(tmp);
  FOR_GRESULT(result_mem, node)
  {
    ARR_PUSH(tmp, node->val);
    ARR_FREE(node->key);
  }
  FOR_GRESULT_END;
  table_gresult_cleanup(result_mem);
  xfree(result_mem);
  ARR_FREE(sort_buf);
  return tmp;
}

/*---------------------------------------------------------------------------
 * Function: graph_result_glmemory_size
 *-------------------------------------------------------------------------*/
int graph_result_glmemory_size (void)
{
  return result_mem->hash_count;
}

/*---------------------------------------------------------------------------
 * Function: graph_result_free 
 *-------------------------------------------------------------------------*/ 
void graph_result_free (GRAPH_RESULT * gr)
{
  if (!gr) return;
  ARR_FREE(gr->mapping);
  xfree(gr);
}

/*---------------------------------------------------------------------------
 * Function: graph_result_array_free
 *-------------------------------------------------------------------------*/ 
void graph_result_array_free (GRAPH_RESULT ** gra)
{
  if (!gra) return;
  for (int i = 0; i < ARR_LEN(gra); i++) graph_result_free(gra[i]);
  ARR_FREE(gra);
}

/*---------------------------------------------------------------------------
 * Function: graph_result_print
 *-------------------------------------------------------------------------*/
void graph_result_print (GRAPH_RESULT * gr)
{
  printf("RESULT GRAPH: |V(R_G)| = %d\n###################\n", gr->g->n_cnt);
  for (int i = 0; i < gr->g->n_cnt; i++)
  {
    printf("Node #%d->%d(deg = %d): ", gr->mapping[i], i, gr->g->edges[i]->hash_count);
    FOR_ADJ(gr->g->edges[i], node)
    {
      printf("%d ", gr->mapping[node->key]);
    }
    FOR_ADJ_END;
    printf("%s", "\n");
  }
  printf("%s\n", "###################");
}
