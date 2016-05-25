/*
 *	Subgraph Isomorphism - Utility functions
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#include "util.h"
#include "graph.h"
#include "array.h"
#include <stdio.h>
#include <stdlib.h>

/****************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************/

/*---------------------------------------------------------------------------
 * Function: graph_free
 *-------------------------------------------------------------------------*/ 
void free_all (void)
{
  graph_free(G_GRAPH);
  graph_free(F_GRAPH);
  ARR_FREE(F_ECC);
}

/*---------------------------------------------------------------------------
 * Function: force_exit
 *-------------------------------------------------------------------------*/ 
void force_exit (void)
{
  free_all();
  exit(1);
}
