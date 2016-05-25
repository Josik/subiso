/*
 *	Subgraph Isomorphism - Extended array interface
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "common.h"
#include <ucw/gary.h>

/* -------------------------
 * Macro: ARR_INIT
 * -------------------------
 * Initializates an empty growing array to (typed) pointer ptr.
 *
 * Params:
 *   ptr - pointer for the array
 */
#define ARR_INIT(ptr) GARY_INIT((ptr), 0)

/* -------------------------
 * Macro: ARR_ALLOC
 * -------------------------
 * Initializates a growing array of length len to (typed) pointer ptr.
 *
 * Params:
 *   ptr - pointer for the array
 */
#define ARR_ALLOC(ptr, len) GARY_INIT((ptr), (len))

/* -------------------------
 * Macro: ARR_ALLOC
 * -------------------------
 * Releases space allocated for array at ptr.
 *
 * Params:
 *   ptr - pointer to the array
 */
#define ARR_FREE(ptr) GARY_FREE((ptr))

/* -------------------------
 * Macro: ARR_LEN
 * -------------------------
 * Returns number of elements in the array pointed by pointer ptr.
 *
 * Params:
 *   ptr - pointer to the array
 */
#define ARR_LEN(ptr) GARY_SIZE((ptr))

/* -------------------------
 * Macro: ARR_PUSH
 * -------------------------
 * Inserts element x to the array pointed by pointer ptr.
 *
 * Params:
 *   ptr - pointer to the array
 *   x   - element to be inserted
 */
#define ARR_PUSH(ptr, x)                       \
       ({                                      \
          GARY_PUSH((ptr));                    \
          (ptr)[(GARY_SIZE((ptr))) - 1] = (x); \
        })                                     \

/* -------------------------
 * Macro: ARR_POP
 * -------------------------
 * Deletion of last element in the array pointed by pointer ptr.
 *
 * Params:
 *   ptr - pointer to the array
 */
#define ARR_POP(ptr) GARY_POP((ptr))

#endif /* __ARRAY_H__ */
