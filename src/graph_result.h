/*
 *	Subgraph Isomorphism - Graph results
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __GRAPH_RESULT_H__
#define __GRAPH_RESULT_H__

#include "common.h"
#include "string.h"

/****************************************************************************
 * DECLARATIONS
 ***************************************************************************/

/* Structure for internal representation of graph results */
struct graph_result_struct
{
  /* Array containing mapping in this particular result */
  int   * mapping;
  /* Colours used by current mapping */
  umask   used_cols;
  /* Graph corresponding to this result */
  GRAPH * g;
};

/****************************************************************************
 * FUNCTIONS
 ***************************************************************************/

/* -------------------------
 * Function: graph_result_init
 * -------------------------
 * Initializes graph result storage for a given graph
 * 
 * Params:
 *   g - graph for which the results are relevant
 *
 * Returns:
 *   Pointer to graph result storage
 */
GRAPH_RESULT *  graph_result_init   (GRAPH * g);

/* -------------------------
 * Function: graph_result_glmemory_init
 * -------------------------
 * Initializes memory of unique graph results
 * 
 * Params:
 *   g - graph for which the results are relevant
 */
void             graph_result_glmemory_init (GRAPH * g);

/* -------------------------
 * Function: graph_result_glmemory_add
 * -------------------------
 * Filters unique subgraphs (by vertices) from given array of graph results
 * and puts them into global results memory
 * 
 * Params:
 *   gr - array of graph results
 */
void            graph_result_glmemory_add (GRAPH_RESULT ** gra);

/* -------------------------
 * Function: graph_result_glmemory_reconstruct
 * -------------------------
 * Creates an array of found results from global results memory;
 * the memory is freed after this operation
 *
 * Params:
 *   none 
 *
 * Returns:
 *   gr - array of graph results
 */
GRAPH_RESULT ** graph_result_glmemory_reconstruct (void);

/* -------------------------
 * Function: graph_result_glmemory_size 
 * -------------------------
 * Counts number of unique subgraphs in global memory
 * 
 * Params:
 *   none
 *
 * Returns:
 *   number of unique subgraphs in global memory
 */

int             graph_result_glmemory_size (void);

/* -------------------------
 * Function: graph_result_free 
 * -------------------------
 * Frees memory allocated by the given graph result
 * 
 * Params:
 *   gr - graph result to be freed
 */

void            graph_result_free    (GRAPH_RESULT * gr);

/* -------------------------
 * Function: graph_result_array_free 
 * -------------------------
 * Frees memory allocated by the given graph result array
 * 
 * Params:
 *   gra - graph result array to be freed
 */
void            graph_result_array_free (GRAPH_RESULT ** gra);

/* -------------------------
 * Function: graph_result_print
 * -------------------------
 * Prints a graph that is stored in graph result representation
 * 
 * Params:
 *   gr - graph result to be printed
 */
void           graph_result_print   (GRAPH_RESULT * gr);
#endif /* __GRAPH_RESULT_H__ */