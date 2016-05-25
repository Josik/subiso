/*
 *	Subgraph Isomorphism - Tree decomposition
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __TREE_DEC_H__
#define __TREE_DEC_H__

#include "common.h"

/****************************************************************************
 * DECLARATIONS
 ***************************************************************************/

 /* Structure representing a tree decomposition node */
struct tree_dec_node_struct
{
  /* Bag content in form of bitmask */
  umask bag;
  /* Adjacency array in form of bitmask (fits, since MAX_F_VERTICES <= 32)*/
  umask adj;
};

/* Structure representing a tree decomposition */
struct tree_dec_struct
{
  /* Treewidth */
  int           tw;
  /* Number of nodes */
  int           b_cnt;
  /* Array of td nodes */
  TREE_DEC_NODE nodes[MAX_F_VERTICES];
};

/****************************************************************************
 * FUNCTIONS
 ***************************************************************************/

/* -------------------------
 * Function: td_get
 * -------------------------
 * Creates a tree decomposition from the given graph.
 * 
 * Params:
 *   g - graph to be processed
 *
 * Returns:
 *   Pointer to the newly created tree decomposition
 */
TREE_DEC * td_get   (GRAPH * g);

/* -------------------------
 * Function: td_free
 * -------------------------
 * Frees the space allocated by a tree decomposition.
 * 
 * Params:
 *   td - pointer to the tree decomposition to be freed
 */
void       td_free  (TREE_DEC * td);

/* -------------------------
 * Function: td_print
 * -------------------------
 * Prints a tree decomposition.
 * 
 * Params:
 *   td - pointer to the tree decomposition to be printed
 */
void       td_print (TREE_DEC * td);

#endif /* __TREE_DEC_H__ */
