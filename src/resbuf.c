/*
 *	Subgraph Isomorphism - Result buffer
 *
 *	(c) 2016 Josef Malik <josef.malik@fit.cvut.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/* Debugging for buffer I/O */
#ifdef LOCAL_DEBUG_BUF
  #define DBG_BUF(x, y...) DBG(x, ##y)
#else
  #define DBG_BUF(x, y...)  
#endif

#include "resbuf.h"
#include "stdlib.h"
#include "array.h"
#include "util.h"
#include <ucw/fastbuf.h>
#include <ucw/varint.h>
#include <ucw/ff-binary.h>

/****************************************************************************
 * STATIC FUNCTIONS
 ***************************************************************************/

/* -------------------------
 * Function: encode_num
 * -------------------------
 * Stores a number to buffer in var. len. code along with delta compr.
 *
 * Params:
 *   rb - pointer to the corresponding result buffer
 *   x  - number to be stored  
 */
static void encode_num (RESBUF * rb, u32 x)
{
  u32 len, diff;
  byte tmp_buf[10];
  
  diff = x - rb->last;
  len = varint_put(tmp_buf, diff);
  bwrite(rb->buf, tmp_buf, len);
  rb->last = x;
  
  DBG_BUF("ENC %u", x);
}

/* -------------------------
 * Function: decode_num
 * -------------------------
 * Retrieves a number from a buffer
 *
 * Params:
 *   rb - pointer to the corresponding result buffer
 *
 * Returns:
 *   The actual number stored in rb
 */
static u32 decode_num (RESBUF * rb)
{
  u32 x;
  
  rb->buf->bptr = (byte *)varint_get32(rb->buf->bptr, &x);
  x += rb->last;
  rb->last = x;
  
  DBG_BUF("DEC %u", x);
  return x;
}

/* -------------------------
 * Function: print_buf
 * -------------------------
 * Prints a mapping record
 *
 * Params:
 *   map   - mapping
 *   mlen  - length of the mapping
 *   col   - list of colors
 *   clen  - number of colors
 */
void print_buf (u32 * map, int mlen, u32 * col, int clen)
{
#ifdef LOCAL_DEBUG_BUF  
  DBG_BUF("LEN map = %d", mlen);
  for (int i = 0; i < mlen; i++) DBG_BUF("(G%d)", map[i]);
  DBG_BUF("LEN col = %d", clen);
  for (int i = 0; i < clen; i++) PRINT_BINARY(col[i]);
#endif  
}

/****************************************************************************
 * INTERFACE FUNCTIONS
 ***************************************************************************/

/*---------------------------------------------------------------------------
 * Function: resbuf_init
 *-------------------------------------------------------------------------*/
RESBUF * resbuf_init (void)
{
  RESBUF * tmp = (RESBUF *)xmalloc(sizeof(*tmp));
  tmp->buf = fbgrow_create(1);
  tmp->last = 0;
  resbuf_chng_state(tmp, RES_WRITE);
  return tmp;
}

/*---------------------------------------------------------------------------
 * Function: resbuf_free
 *-------------------------------------------------------------------------*/
void resbuf_free (RESBUF * rb)
{
  if (!rb) return;
  bclose(rb->buf);
  free(rb);
}

/*---------------------------------------------------------------------------
 * Function: resbuf_chng_state
 *-------------------------------------------------------------------------*/
void resbuf_chng_state (RESBUF * rb, int state)
{
  rb->last = 0;
  rb->state = state;
  switch (state)
  {
    case RES_WRITE:
      fbgrow_reset(rb->buf);
      break;
    case RES_READ:
      fbgrow_rewind(rb->buf);
      break;
    default:
      break;
  }
}
 
/*---------------------------------------------------------------------------
 * Function: resbuf_push
 *-------------------------------------------------------------------------*/
void resbuf_push (RESBUF * rb, u32 * map, int mlen, u32 * col, int clen)
{
  DBG_BUF("RESBUF_PUSH");
#ifdef LOCAL_DEBUG_BUF  
  print_buf(map, mlen, col, clen);
#endif  
  DBG_BUF("ENC map");
  for (u32 i = 0; i < mlen; i++) encode_num(rb, map[i]);
  DBG_BUF("ENC clen");
  encode_num(rb, clen);
  DBG_BUF("ENC col");
  for (u32 i = 0; i < clen; i++) encode_num(rb, col[i]);
  DBG_BUF("-----------");
}

/*---------------------------------------------------------------------------
 * Function: resbuf_read
 *-------------------------------------------------------------------------*/
int resbuf_read (RESBUF * rb, u32 * map, int mlen, u32 * col, int * clen)
{
  if (beof(rb->buf)) return RES_EOF;
  DBG_BUF("RESBUF_READ");
  DBG_BUF("DEC map");
  for (u32 i = 0; i < mlen; i++) map[i] = decode_num(rb);
  DBG_BUF("DEC clen");
  *clen = decode_num(rb);
  DBG_BUF("DEC col");
  for (u32 i = 0; i < *clen; i++) col[i] = decode_num(rb);
  DBG_BUF("-----------");
#ifdef LOCAL_DEBUG_BUF
  print_buf(map, mlen, col, *clen);
#endif
  return RES_OK;
}
