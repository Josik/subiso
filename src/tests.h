/*
 *	Subgraph Isomorphism - Qualitative tests
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __TESTS_H__
#define __TESTS_H__

#include "common.h"

/****************************************************************************
 * FUNCTIONS
 ***************************************************************************/

/* -------------------------
* Function: test_tree_dec
* -------------------------
* Checks, whether given tree decomposition is valid and optimal
* 
* Params:
*   td - given tree decomposition
*
* Returns:
*   TEST_OK if the tree decomposition is OK
*/
int test_tree_dec      (TREE_DEC * td);


/* -------------------------
* Function: test_nice_tree_dec
* -------------------------
* Checks, whether given nice tree decomposition is valid and optimal
* 
* Params:
*   ntd - given nice tree decomposition
*
* Returns:
*   TEST_OK if the nice tree decomposition is OK
*/
int test_nice_tree_dec (NICE_TREE_DEC * ntd);


/* -------------------------
* Function: test_results
* -------------------------
* Checks, whether found subgraphs are valid in the graph that has been searched
* 
* Params:
*   results - array of found results
*
* Returns:
*   TEST_OK if all results are OK
*/
int test_results       (GRAPH_RESULT ** results);
 
#endif /* __TESTS_H__ */