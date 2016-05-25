/*
 *	Subgraph Isomorphism - Nice tree decomposition
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "nice_tree_dec.h"
#include "tree_dec.h"
#include "util.h"
#include "graph.h"
#include "array.h"
#include "resbuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/****************************************************************************
 * STATIC FUNCTIONS
 ***************************************************************************/

/* -------------------------
 * Function: add_nice_tree_dec_node
 * -------------------------
 * Adds a new node to the ntd
 * 
 * Params:
 *   ntd     - corresponding nice tree decomposition
 *   content - bitmask of vertices contained in this node's bag
 *   adj     - array of adjacent nodes
 *   type    - type of ntd node
 *
 * Returns:
 *   Identification number in ntd scope of the newly created node
 */
static int add_nice_tree_dec_node (NICE_TREE_DEC * ntd, umask content, int * adj, int type)
{
  NICE_TREE_DEC_NODE tmp;
  tmp.bag = content;
  tmp.adj = adj;
  tmp.type = type;
  for (int i = 0; i < ARR_LEN(adj); i++) ARR_PUSH(ntd->nodes[adj[i]].adj, ntd->b_cnt);
  ARR_PUSH(ntd->nodes, tmp);
  return ntd->b_cnt++;
}

/* -------------------------
 * Function: ntd_preprocess
 * -------------------------
 * Recursively preprocesses nice tree decomposition for later usage in other algorithms.
 * Special attributes in ntd are filled during the run of this function.
 * 
 * Params:
 *   ntd  - corresponding nice tree decomposition
 *   x    - current node being preprocessed
 *   prev - previously preprocessed node (initially -1)
 *
 * Returns:
 *   Number of join nodes in x's subtree (required by recursion)
 */
static int ntd_preprocess (NICE_TREE_DEC * ntd, int x, int prev)
{
  ntd->nodes[x].idx = x;
  ntd->nodes[x].rbuf = NULL;
  /* Auxilary children array */
  int * adj_ch;
  ARR_INIT(adj_ch);
  for (int i = 0; i < ARR_LEN(ntd->nodes[x].adj); i++)
  {
    if (ntd->nodes[x].adj[i] != prev) ARR_PUSH(adj_ch, ntd->nodes[x].adj[i]);
  }
  /* Precomputation of maximal number of join nodes in subtrees */
  int maxjc, curjc, mpos;
  maxjc = 0;
  mpos = -1;
  for (int i = 0; i < ARR_LEN(adj_ch); i++)
  {
    curjc = ntd_preprocess(ntd, adj_ch[i], x);
    if (curjc >= maxjc)
    {
      maxjc = curjc;
      mpos = i;
    }
  }
  /* Creating of array with bag content from a bag content bitmask */
  ARR_INIT(ntd->nodes[x].bag_cont);
  for (int i = 0; i < MAX_F_VERTICES; i++)
  {
    if (GET_BIT(ntd->nodes[x].bag, i)) ARR_PUSH(ntd->nodes[x].bag_cont, i); 
  }
  /* Precomputation of the element (and its index) changed by introduce/forget node */
  if (ntd->nodes[x].type == INTRODUCE_NODE || ntd->nodes[x].type == FORGET_NODE)
  {
    for (int i = 0; i < MAX(ARR_LEN(ntd->nodes[x].bag_cont), ARR_LEN(ntd->nodes[adj_ch[0]].bag_cont)); i++)
    {
      if (i >= ARR_LEN(ntd->nodes[x].bag_cont) || 
          i >= ARR_LEN(ntd->nodes[adj_ch[0]].bag_cont) || 
          ntd->nodes[x].bag_cont[i] != ntd->nodes[adj_ch[0]].bag_cont[i])
      {
        ntd->nodes[x].chng_index = i;
        if (ntd->nodes[x].type == INTRODUCE_NODE) ntd->nodes[x].chng_vertex = ntd->nodes[x].bag_cont[i];
        else ntd->nodes[x].chng_vertex = ntd->nodes[adj_ch[0]].bag_cont[i];
        break;
      }
    }
  }
  else ntd->nodes[x].chng_vertex = ntd->nodes[x].chng_index = -1;
  /* Precomputation of pointers to parent/children */
  ntd->nodes[x].parent = (prev >= 0 ? &(ntd->nodes[prev]) : NULL);
  ntd->nodes[x].child_1 = ntd->nodes[x].child_2 = NULL;
  if (ntd->nodes[x].type != LEAF_NODE) ntd->nodes[x].child_1 = &(ntd->nodes[adj_ch[0]]);
  if (ntd->nodes[x].type == JOIN_NODE)
  {
    ++maxjc;
    if (!mpos) ntd->nodes[x].child_2 = &(ntd->nodes[adj_ch[1]]);
    else
    {
      ntd->nodes[x].child_1 = &(ntd->nodes[adj_ch[1]]);
      ntd->nodes[x].child_2 = &(ntd->nodes[adj_ch[0]]);
    }
  }
  ARR_FREE(adj_ch);
  return maxjc;
}

/* -------------------------
 * Function: ntd_connect
 * -------------------------
 * Connects two ntd nodes with different bag contents by adding new
 * (introduce/forget) nodes.
 * 
 * Params:
 *   ntd  - corresponding nice tree decomposition
 *   from - first node
 *   to   - second node
 */
static void ntd_connect (NICE_TREE_DEC * ntd, int from, int to)
{
  umask TS = 0, CS = ntd->nodes[from].bag;
  if (to >= 0) TS = ntd->nodes[to].bag;
  
  int last_node;
  umask last_bag, new_bag;
  int * new_arr;
  
  last_node = from;
  last_bag = ntd->nodes[from].bag;
  
  for (int i = 0; i < MAX_F_VERTICES; i++)
  {
    if (GET_BIT(CS, i) && !GET_BIT(TS, i)) 
    {
      new_bag = UNSET_BIT(last_bag, i);
      ntd->nodes[last_node].type = INTRODUCE_NODE;
    }
    else if (!GET_BIT(CS, i) && GET_BIT(TS, i)) 
    {
      new_bag = SET_BIT(last_bag, i);
      ntd->nodes[last_node].type = FORGET_NODE;
    }
    else continue;
    
    if (new_bag == TS)
    {
      if (to < 0)
      {
        /* Node with single element bag is considered a leaf -- current variant */
          ntd->nodes[last_node].type = LEAF_NODE;
        
        /* Addition of a leaf, which is a node with an empty bag -- obsoleted */
        /*
          ARR_INIT(new_arr);
          ARR_PUSH(new_arr, last_node);
          add_nice_tree_dec_node(ntd, EMPTY_MASK, new_arr, LEAF_NODE);
        */
      }
      else
      {
        ARR_PUSH(ntd->nodes[to].adj, last_node);
        ARR_PUSH(ntd->nodes[last_node].adj, to);
      }
      break;
    }
    else 
    {
      ARR_INIT(new_arr);
      ARR_PUSH(new_arr, last_node);
      last_node = add_nice_tree_dec_node(ntd, new_bag, new_arr, LEAF_NODE);
      last_bag = new_bag;
    }
  }
}

/* -------------------------
 * Function: td_dfs
 * -------------------------
 * Creates a nice tree decomposition from given tree decomposition by recursively
 * travelling it.
 * 
 * !!! It is required that ntd already contains all nodes from td with corresponding !!!
 * !!! identifiers before first calling this function                                !!!
 *
 * Params:
 *   td   - corresponding tree decomposition
 *   x    - current td node being processed
 *   prev - previously processed node (initially -1)
 *   ntd  - nice tree decomposition storage (needs to meet previously stated condition)
 */
static void td_dfs (TREE_DEC * td, int x, int prev, NICE_TREE_DEC * ntd)
{
  int nbg_cnt = 0, nbg;
  /* Retrieve neighbour count */
  for (int i = 0; i < td->b_cnt; i++)
  {
    if (GET_BIT(td->nodes[x].adj, i) && i != prev) 
    {
      ++nbg_cnt;
      nbg = i;
    }
  }
  
  if (nbg_cnt <= 1) /* Introduce/forget node - handled in ntd_connect */
  {
    ntd_connect(ntd, x, nbg_cnt ? nbg : -1);
    if (nbg_cnt) td_dfs(td, nbg, x, ntd);
  }
  else /* Join node - creates full bin.tree with all adjacent nodes in leaves */
  {
    ntd->nodes[x].type = JOIN_NODE;
    
    int q[MAX_F_VERTICES], q1_head, q1_tail, q2_tail, tmp_node;
    int * new_arr;
    
    q1_head = q1_tail = q2_tail = 0;
    for (int i = 0; i < td->b_cnt; i++)
    {
      if (GET_BIT(td->nodes[x].adj, i) && i != prev) 
      {
        if (ntd->nodes[i].bag == ntd->nodes[x].bag) q[q1_tail++] = i; 
        else
        {
          ARR_INIT(new_arr);
          q[q1_tail++] = tmp_node = add_nice_tree_dec_node(ntd, ntd->nodes[x].bag, new_arr, LEAF_NODE);
          ntd_connect(ntd, tmp_node, i);
        }
      }
    }
    
    while (q1_tail > 2)
    {
      while (q1_head < q1_tail)
      {
        if (q1_head == q1_tail - 1) q[q2_tail++] = q[q1_head]; 
        else
        {
          ARR_INIT(new_arr);
          ARR_PUSH(new_arr, q[q1_head]);
          ARR_PUSH(new_arr, q[q1_head + 1]);
          q[q2_tail++] = add_nice_tree_dec_node(ntd, ntd->nodes[x].bag, new_arr, JOIN_NODE);
        }
        q1_head += 2;
      }
      q1_tail = q2_tail;
      q1_head = q2_tail = 0;
    }
    ARR_PUSH(ntd->nodes[x].adj, q[q1_head]);
    ARR_PUSH(ntd->nodes[q[q1_head]].adj, x);
    ARR_PUSH(ntd->nodes[x].adj, q[q1_head + 1]);
    ARR_PUSH(ntd->nodes[q[q1_head + 1]].adj, x);
    for (int i = 0; i < td->b_cnt; i++)
    {
      if (GET_BIT(td->nodes[x].adj, i) && i != prev) td_dfs(td, i, x, ntd); 
    }
  }
}

/* -------------------------
 * Function: get_type
 * -------------------------
 * Returns string representation of ntd node
 * 
 * Params:
 *   x - constant representing ntd node
 *
 * Returns:
 *   String representation of ntd node
 */
static const char * get_type (int x)
{
  switch (x)
  {
    case LEAF_NODE: return "LEAF";
    case JOIN_NODE: return "JOIN";
    case INTRODUCE_NODE: return "INTRODUCE";
    case FORGET_NODE: return "FORGET";
    default: force_exit();
  }
}

/****************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************/

/*---------------------------------------------------------------------------
 * Function: ntd_get
 *
 * Description:
 *   Nice tree decomposition is constructed by traversing given tree
 *   decomposition and adding auxilary nodes to ensure ntd properties.
 *-------------------------------------------------------------------------*/
NICE_TREE_DEC * ntd_get (TREE_DEC * td)
{
  NICE_TREE_DEC * tmp = (NICE_TREE_DEC *)xmalloc(sizeof(*tmp));
  tmp->tw = td->tw;
  tmp->b_cnt = 0;
  ARR_INIT(tmp->nodes);
  u32 * new_arr;
  /* Addition of bags from td to ntd while perserving their identifiers in decomposition
     - alleviation and must-do before calling 'td_dfs' */
  for (int i = 0; i < td->b_cnt; i++) 
  {
    ARR_INIT(new_arr);
    add_nice_tree_dec_node(tmp, td->nodes[i].bag, new_arr, LEAF_NODE);
  }
  /* Construction */
  td_dfs(td, 0, -1, tmp);
  /* Dummy root forget node with an empty bag */
  ARR_INIT(new_arr);
  ARR_PUSH(new_arr, 0);
  tmp->root = add_nice_tree_dec_node(tmp, 0U, new_arr, FORGET_NODE);
  /* Preprocessing of ntd for its later usage in algorithm */
  ntd_preprocess(tmp, tmp->root, -1);
  return tmp;
}

/*---------------------------------------------------------------------------
 * Function: ntd_free
 *-------------------------------------------------------------------------*/
void ntd_free (NICE_TREE_DEC * ntd)
{
  if (!ntd) return;
  for (int i = 0; i < ntd->b_cnt; i++) 
  {
    ARR_FREE(ntd->nodes[i].adj);
    ARR_FREE(ntd->nodes[i].bag_cont);
    resbuf_free(ntd->nodes[i].rbuf);
  }
  ARR_FREE(ntd->nodes);
  xfree(ntd); 
}

/*---------------------------------------------------------------------------
 * Function: ntd_print_node
 *-------------------------------------------------------------------------*/
void ntd_print_node (NICE_TREE_DEC_NODE * node)
{
  fprintf(stderr,"BAG #%d, NODE TYPE = %s:\n", node->idx, get_type(node->type));
  fprintf(stderr,"Bag:");
  for (int j = 0; j < MAX_F_VERTICES; j++) if (GET_BIT(node->bag, j)) fprintf(stderr," %d", j);
  fprintf(stderr,"\n");
  fprintf(stderr,"Edges:");
  for (int j = 0; j < ARR_LEN(node->adj); j++) fprintf(stderr," B%d", node->adj[j]);
  fprintf(stderr,"\n");
  fprintf(stderr,"Pre_BAG:");
  for (int j = 0; j < ARR_LEN(node->bag_cont); j++) fprintf(stderr," %d", node->bag_cont[j]);
  fprintf(stderr,"\n");
  fprintf(stderr,"Pre_ADR:");
  fprintf(stderr,"%p\n", node);
  fprintf(stderr,"Pre_CHNG_VERTEX:");
  fprintf(stderr,"%d\n", node->chng_vertex);
  fprintf(stderr,"Pre_CHNG_INDEX:");
  fprintf(stderr,"%d\n", node->chng_index);
  fprintf(stderr,"Pre_P:");
  fprintf(stderr,"%p\n", node->parent);
  fprintf(stderr,"Pre_CH1:");
  fprintf(stderr,"%p\n", node->child_1);
  fprintf(stderr,"Pre_CH2:");
  fprintf(stderr,"%p\n", node->child_2);
}

/*---------------------------------------------------------------------------
 * Function: ntd_print
 *-------------------------------------------------------------------------*/
void ntd_print (NICE_TREE_DEC * ntd)
{
  fprintf(stderr,"NTD, ROOT = %d:\n", ntd->root);
  fprintf(stderr,"~~~~~~~~~\n");
  for (int i = 0; i < ntd->b_cnt; i++) 
  {
    ntd_print_node(ntd->nodes + i);
    fprintf(stderr,"\n");
  }
  fprintf(stderr,"~~~~~~~~~\n");
}
