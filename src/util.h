/*
 *	Subgraph Isomorphism - Utility functions
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

#ifndef __UTIL_H__
#define __UTIL_H__

#include "common.h"

#define GET_BIT(x, pos)   ((x) & (1U << (pos)))
#define SET_BIT(x, pos)   ((x) | (1U << (pos)))
#define UNSET_BIT(x, pos) ((x) - (1U << (pos)))
#define BIT_COMPL(x, g)   ((1U << ((g)->n_cnt)) - 1 - (x))

/* -------------------------
 * Macros: BTB_PATT, BTB(byte), PRINT_BINARY(x)
 * -------------------------
 * PRINT_BINARY prints binary content of 4B integer 
 * 
 * Params:
 *   x - integer to be printed
 */
#define BTB_PATT          "%d%d%d%d%d%d%d%d"
#define BTB(byte) \
        (byte & 0x80 ? 1 : 0), \
        (byte & 0x40 ? 1 : 0), \
        (byte & 0x20 ? 1 : 0), \
        (byte & 0x10 ? 1 : 0), \
        (byte & 0x08 ? 1 : 0), \
        (byte & 0x04 ? 1 : 0), \
        (byte & 0x02 ? 1 : 0), \
        (byte & 0x01 ? 1 : 0)
     
#define PRINT_BINARY(x) \
        DBG(BTB_PATT""BTB_PATT""BTB_PATT""BTB_PATT, BTB(x >> 24), BTB(x >> 16), BTB(x >> 8), BTB(x))

#ifdef LOCAL_DEBUG
  #define DBG_PRINT(x, y...) fprintf(stderr, x, y)
#else
  #define DBG_PRINT(x, y...)
#endif
        
/****************************************************************************
 * FUNCTIONS
 ***************************************************************************/

void free_all   (void);
void force_exit (void);

#endif /* __UTIL_H__ */
