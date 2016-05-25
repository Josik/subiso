/*
 *	Subgraph Isomorphism - Result buffer
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __RESBUF_H__
#define __RESBUF_H__

#include "common.h"

/****************************************************************************
 * DECLARATIONS
 ***************************************************************************/

struct resbuf_struct
{
  FASTBUF * buf;
  u32       last;
  int       state;
};

/****************************************************************************
 * FUNCTIONS
 ***************************************************************************/

 /* -------------------------
 * Function: resbuf_init
 * -------------------------
 * Creates a new result buffer.
 * 
 * Returns:
 *   Pointer to the newly created result buffer.
 */
RESBUF * resbuf_init       (void);

/* -------------------------
 * Function: resbuf_free
 * -------------------------
 * Deallocates the memory needed by a result buffer. 
 * 
 * Params:
 *   rb - pointer to the result buffer to be freed
 */
void     resbuf_free       (RESBUF * rb);

/* -------------------------
 * Function: resbuf_chng_state
 * -------------------------
 * Changes a state of the underlying buffer
 * 
 * Params:
 *   rb    - pointer to the corresponding result buffer
 *   state - new state to be set
 */
void     resbuf_chng_state (RESBUF * rb, int state);

/* -------------------------
 * Function: resbuf_push
 * -------------------------
 * Pushes a mapping record into the result buffer
 * 
 * Params:
 *   rb    - pointer to the corresponding result buffer
 *   map   - mapping to be stored
 *   mlen  - length of the mapping to be stored
 *   col   - list of colors to be stored
 *   clen  - number of colors to be stored
 */
void     resbuf_push       (RESBUF * rb, u32 * map, int mlen, u32 * col, int clen);

/* -------------------------
 * Function: resbuf_read
 * -------------------------
 * Reads a single mapping record from the result buffer.
 * 
 * Params:
 *   rb    - pointer to the corresponding result buffer
 *   map   - an array tuple to store the read mapping to
 *   mlen  - length of the mapping to be retrieved
 *   col   - array to store the read color sets to
 *   clen  - number of color sets read
 *
 * Returns:
 *   Either RES_OK if there was a record, or RES_EOF if there is no more record
 *   stored in the result buffer.
 */
int      resbuf_read       (RESBUF * rb, u32 * map, int mlen, u32 * col, int * clen);

#endif /* __RESBUF_H__ */
