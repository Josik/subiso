/*
 *	Subgraph Isomorphism - Common declarations
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include <ucw/lib.h>

/* Graph constants */
#define MAX_G_VERTICES   1000000000 
#define MAX_F_VERTICES   20 /* at least 2^(MAX_F_VERTICES + 2)B is needed for allocations */

/* DP constants*/
#define INF            1000000014
#define NOT_USED       -1

/* Nice tree decomposition constants */
#define LEAF_NODE      0
#define JOIN_NODE      1
#define INTRODUCE_NODE 2
#define FORGET_NODE    3

/* Result buffer constants */
#define RES_WRITE      1
#define RES_READ       2
#define RES_EOF        -1
#define RES_OK         0

/* Test constants */
#define TEST_OK        0
#define TEST_NOK       1

/* Own data types */
typedef u32 umask;
#define EMPTY_MASK 0U

/* Own structures */
typedef struct tree_dec_node_struct      TREE_DEC_NODE;
typedef struct tree_dec_struct           TREE_DEC;
typedef struct nice_tree_dec_node_struct NICE_TREE_DEC_NODE;
typedef struct nice_tree_dec_struct      NICE_TREE_DEC;
typedef struct graph_struct              GRAPH;
typedef struct graph_result_struct       GRAPH_RESULT;
typedef struct graph_result_array_struct GRAPH_RESULT_ARRAY;
typedef struct resbuf_struct             RESBUF;
/* libucw struktures */
typedef struct fastbuf                   FASTBUF;

/* Global variables */
extern GRAPH         * G_GRAPH;
extern GRAPH         * F_GRAPH;
extern int             SEED;
extern int           * COLOUR;
extern int           * F_ECC;

/* Time and memory measurement */
extern double          A_TIME;

#endif /*__COMMON_H__*/
