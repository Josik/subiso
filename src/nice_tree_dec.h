/*
 *	Subgraph Isomorphism - Nice tree decomposition
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __NICE_TREE_DEC_H__
#define __NICE_TREE_DEC_H__

#include "common.h"

/****************************************************************************
 * DECLARATIONS
 ***************************************************************************/ 

/* Structure representing a nice tree decomposition node */
struct nice_tree_dec_node_struct
{
  /* >>> Base attributes of nice tree decomposition nodes <<< */
  
  /* Bag content in form of bitmask */
  umask                bag;
  /* Adjacency array */
  int *                adj;
  /* Node type */
  int                  type;
  /* Node idx (for backward identification) */
  int                  idx;
  
  /* >>> Attributes for the main algorithm <<< */
  
  /* Bag content as an sorted array of vertices */
  int *                bag_cont;
  /* Vertex that is the subject of addition/removal in introduce/forget node */
  int                  chng_vertex;
  /* Position of added/removed vertex in introduce/forget node in bag_cont array */
  int                  chng_index;
  /* Pointer to the parent node */
  NICE_TREE_DEC_NODE * parent;
  /* Pointer to the first child to be processed during the main algorithm */
  NICE_TREE_DEC_NODE * child_1;
  /* Pointer to the second child to be processed during the main algorithm */
  NICE_TREE_DEC_NODE * child_2;
  /* Buffer containing DP results after bottom-up run of the main algorithm*/
  RESBUF *             rbuf;
};

/* Structure representing a nice tree decomposition */
struct nice_tree_dec_struct
{
  /* Treewidth */
  int                  tw;
  /* Number of nodes */
  int                  b_cnt;
  /* Root node (index to nodes array) */
  int                  root;
  /* Array of ntd nodes */
  NICE_TREE_DEC_NODE * nodes;
};

/****************************************************************************
 * FUNCTIONS
 ***************************************************************************/

/* -------------------------
 * Function: ntd_get
 * -------------------------
 * Creates a nice tree decomposition from the given tree decomposition.
 * 
 * Params:
 *   td - pointer to the tree decomposition to be processed
 *
 * Returns:
 *   Pointer to the newly created nice tree decomposition
 */
NICE_TREE_DEC * ntd_get   (TREE_DEC * td);

/* -------------------------
 * Function: ntd_free
 * -------------------------
 * Frees the space allocated by a nice tree decomposition.
 * 
 * Params:
 *   ntd - pointer to the nice tree decomposition to be freed
 */
void            ntd_free  (NICE_TREE_DEC * ntd);

/* -------------------------
 * Function: ntd_print_node
 * -------------------------
 * Prints a nice tree decomposition node.
 * 
 * Params:
 *   node - pointer to the nice tree decomposition node to be printed
 */
void            ntd_print_node (NICE_TREE_DEC_NODE * node);

/* -------------------------
 * Function: ntd_print
 * -------------------------
 * Prints a nice tree decomposition.
 * 
 * Params:
 *   ntd - pointer to the nice tree decomposition to be printed
 */
void            ntd_print (NICE_TREE_DEC * ntd);

#endif /* __NICE_TREE_DEC_H__ */
