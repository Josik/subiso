/*
 *	Subgraph Isomorphism - Isomorphism search
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __SUBISO_H__
#define __SUBISO_H__

#include "common.h"

/****************************************************************************
 * FUNCTIONS
 ***************************************************************************/

/* -------------------------
 * Function: subiso_run
 * -------------------------
 * Runs the main algorithm for searching subgraphs in given graph
 * Graph being searched should be stored in 'F_GRAPH' variable and graph being
 * searched should be stored in 'G_GRAPH' variable.
 * 
 * Params:
 *   ntd     - nice tree decomposition of F_GRAPH
 *   rep_cnt - number of algorithm repetitions
 *
 * Returns:
 *   Array with found results
 */
GRAPH_RESULT ** subiso_run (NICE_TREE_DEC * ntd, int rep_cnt);

#endif /* __SUBISO_H__ */
