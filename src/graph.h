/*
 *	Subgraph Isomorphism - Graph representation
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __GRAPH_H__
#define __GRAPH_H__

#include "common.h"

/* Libucw hash-table defines (for adjacency lists) */
typedef struct
{
  int key;
} graph_adj_table_node;

typedef struct table_adj_table ADJ_TABLE;

#define HASH_TABLE_DYNAMIC
#define HASH_NODE graph_adj_table_node
#define HASH_PREFIX(x) table_adj_##x
#define HASH_KEY_ATOMIC key
#define HASH_WANT_NEW
#define HASH_WANT_LOOKUP
#define HASH_WANT_CLEANUP
#define HASH_WANT_FIND
#define HASH_WANT_ITERATOR

#define FOR_ADJ(t, x) HASH_FOR_ALL_DYNAMIC(table_adj, t, x)
#define FOR_ADJ_END   HASH_END_FOR

#include <ucw/hashtable.h>

/****************************************************************************
 * DECLARATIONS
 ***************************************************************************/

/* Structure for internal representation of graphs */
struct graph_struct
{
  /* Number of vertices */
  int          n_cnt;
  /* Hash table of adjacent vertices (for edges) */
  ADJ_TABLE ** edges;
};

/****************************************************************************
 * FUNCTIONS
 ***************************************************************************/

/* -------------------------
 * Function: graph_load
 * -------------------------
 * Creates an internal representation of the graph specified in file f_name.
 * 
 * Params:
 *   f_name - name of the file containing text representation of the graph
 *   max_v  - maximal number of vertices allowed for the loaded graph
 *
 * Returns:
 *   Pointer to loaded graph represented by GRAPH structure,
 *   or NULL if number of vertices of the given graph is larger than max_v
 */
GRAPH * graph_load   (const char * fname, int maxv);

/* -------------------------
 * Function: graph_clone
 * -------------------------
 * Creates and returns an exact copy of the given graph g.
 * 
 * Params:
 *   g - pointer to the graph to be copied
 *
 * Returns:
 *   Pointer to the newly created copy of g
 */
GRAPH * graph_clone  (GRAPH * g);

/* -------------------------
 * Function: graph_free
 * -------------------------
 * Frees the space used by graph g. 
 * 
 * Params:
 *   g - pointer to a graph to be freed
 */
void    graph_free   (GRAPH * g);

/* -------------------------
 * Function: graph_add_edge
 * -------------------------
 * Adds edge between vertices from and to in graph g
 * 
 * Params:
 *   g    - corresponding graph
 *   from - number of the first vertex
 *   to   - number of the second vertex
 */
void    graph_add_edge (GRAPH * g, int from, int to);

/* -------------------------
 * Function: graph_is_adj
 * -------------------------
 * Checks whether vertices numbered from and to are adjacent in graph g.
 * 
 * Params:
 *   g    - pointer to the corresponding graph
 *   from - number of the first vertex
 *   to   - number of the second vertex
 *
 * Returns:
 *   1 if the given vertices are adjacent
 */
int     graph_is_adj (GRAPH * g, int from, int to);

/* -------------------------
 * Function: graph_pre_ecc
 * -------------------------
 * Precomputes eccentricity of all vertices in F_GRAPH
 * 
 */
void    graph_pre_f_ecc (void);

/* -------------------------
 * Function: graph_print
 * -------------------------
 * Prints a graph
 * 
 * Params:
 *   g    - pointer to the corresponding graph
 */
int     graph_print (GRAPH * g);
#endif /* __GRAPH_H__ */
