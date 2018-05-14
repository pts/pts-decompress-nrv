/*
 * decompress_nrv.c: educational C code for NRV decompression
 * by pts@fazekas.hu at Mon May 14 14:49:06 CEST 2018
 *
 * This C code compiles, otherwise it is untested!
 *
 * Compile with: gcc -c -O2 -W -Wall -Wextra -Werror decompress_nrv.c
 *
 * Code based on ucl-1.03/src/{getbit.h,n2b_d.c,n2d_d.c,n2e_d.c} . The
 * implementation in UCL is faster than this.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* !! Why does bitsize make a difference in the output size? */

#define fail(x,r) if (x) { fprintf(stderr, "fatal: %s\n", #r); fflush(stderr); abort(); }

#define NRVD_UINT32_C(c) ((uint32_t)(c##ul))

/* Always returns 0 or 1. */
static uint8_t getbit(uint32_t *bbp, unsigned *bcp, const uint8_t* src,
                      unsigned *ilenp, unsigned bitsize) {
  if (bitsize == 8) {
    return *bbp *= 2, *bbp & 0xff ? (*bbp>>8)&1 : ((*bbp=src[*ilenp++]*2+1)>>8)&1;
  } else if (bitsize == 16) {  /* le16. */
    return (*bbp*=2,*bbp&0xffff ? (*bbp>>16)&1 : (*ilenp+=2,((*bbp=(src[*ilenp-2]+src[*ilenp-1]*256u)*2+1)>>16)&1));
  } else if (bitsize == 32) {  /* le32. */
    return *bcp > 0 ? ((*bbp>>--*bcp)&1) : (*bcp=31, *bbp=src[*ilenp]+src[*ilenp+1]*0x100+src[*ilenp+2]*NRVD_UINT32_C(0x10000)+src[*ilenp+3]*NRVD_UINT32_C(0x1000000), *ilenp+=4,(*bbp>>31)&1);
  } else {
    fail(1, ERROR_BAD_BITSIZE);
  }
}

void decompress_nrv2b(const uint8_t* src, unsigned src_len,
                      uint8_t* dst, unsigned* dst_len,
                      unsigned bitsize) {
  unsigned bc = 0;
  uint32_t bb = 0;
  unsigned ilen = 0, olen = 0, last_m_off = 1;
  const unsigned oend = *dst_len;

  for (;;) {
    unsigned m_off, m_len;

    while (getbit(&bb, &bc, src, &ilen, bitsize)) {
      fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
      fail(olen >= oend, ERROR_OUTPUT_OVERRUN);
      dst[olen++] = src[ilen++];
    }
    m_off = 1;
    for (;;) {
      m_off = m_off*2 + getbit(&bb, &bc, src, &ilen, bitsize);
      fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
      fail(m_off > NRVD_UINT32_C(0xffffff) + 3, ERROR_LOOKBEHIND_OVERRUN);
      if (getbit(&bb, &bc, src, &ilen, bitsize)) break;
    }
    if (m_off == 2) {
      m_off = last_m_off;
    } else {
      fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
      m_off = (m_off-3)*256 + src[ilen++];
      if (m_off == NRVD_UINT32_C(0xffffffff))
        break;
      last_m_off = ++m_off;
    }
    m_len = getbit(&bb, &bc, src, &ilen, bitsize);
    m_len = m_len*2 + getbit(&bb, &bc, src, &ilen, bitsize);
    if (m_len == 0) {
      m_len++;
      do {
        m_len = m_len*2 + getbit(&bb, &bc, src, &ilen, bitsize);
        fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
        fail(m_len >= oend, ERROR_OUTPUT_OVERRUN);
      } while (!getbit(&bb, &bc, src, &ilen, bitsize));
      m_len += 2;
    }
    m_len += (m_off > 0xd00);
    fail(olen + m_len > oend, ERROR_OUTPUT_OVERRUN);
    fail(m_off > olen, ERROR_LOOKBEHIND_OVERRUN);
    {
      const uint8_t* m_pos;
      m_pos = dst + olen - m_off;
      dst[olen++] = *m_pos++;
      do dst[olen++] = *m_pos++; while (--m_len > 0);
    }
  }
  *dst_len = olen;
  fail(ilen < src_len, ERROR_INPUT_NOT_CONSUMED);
  fail(ilen > src_len, ERROR_INPUT_OVERRUN);
}

void decompress_nrv2d(const uint8_t* src, unsigned src_len,
                      uint8_t* dst, unsigned* dst_len,
                      unsigned bitsize) {
  unsigned bc = 0;
  uint32_t bb = 0;
  unsigned ilen = 0, olen = 0, last_m_off = 1;
  const unsigned oend = *dst_len;

  for (;;) {
    unsigned m_off, m_len;

    while (getbit(&bb, &bc, src, &ilen, bitsize)) {
      fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
      fail(olen >= oend, ERROR_OUTPUT_OVERRUN);
      dst[olen++] = src[ilen++];
    }
    m_off = 1;
    for (;;) {
      m_off = m_off*2 + getbit(&bb, &bc, src, &ilen, bitsize);
      fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
      fail(m_off > NRVD_UINT32_C(0xffffff) + 3, ERROR_LOOKBEHIND_OVERRUN);
      if (getbit(&bb, &bc, src, &ilen, bitsize)) break;
      m_off = (m_off-1)*2 + getbit(&bb, &bc, src, &ilen, bitsize);
    }
    if (m_off == 2) {
      m_off = last_m_off;
      m_len = getbit(&bb, &bc, src, &ilen, bitsize);
    } else {
      fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
      m_off = (m_off-3)*256 + src[ilen++];
      if (m_off == NRVD_UINT32_C(0xffffffff))
        break;
      m_len = (m_off ^ NRVD_UINT32_C(0xffffffff)) & 1;
      m_off >>= 1;
      last_m_off = ++m_off;
    }
    m_len = m_len*2 + getbit(&bb, &bc, src, &ilen, bitsize);
    if (m_len == 0) {
      m_len++;
      do {
        m_len = m_len*2 + getbit(&bb, &bc, src, &ilen, bitsize);
        fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
        fail(m_len >= oend, ERROR_OUTPUT_OVERRUN);
      } while (!getbit(&bb, &bc, src, &ilen, bitsize));
      m_len += 2;
    }
    m_len += (m_off > 0x500);
    fail(olen + m_len > oend, ERROR_OUTPUT_OVERRUN);
    fail(m_off > olen, ERROR_LOOKBEHIND_OVERRUN);
    {
      const uint8_t* m_pos;
      m_pos = dst + olen - m_off;
      dst[olen++] = *m_pos++;
      do dst[olen++] = *m_pos++; while (--m_len > 0);
    }
  }
  *dst_len = olen;
  fail(ilen < src_len, ERROR_INPUT_NOT_CONSUMED);
  fail(ilen > src_len, ERROR_INPUT_OVERRUN);
}

void decompress_nrv2e(const uint8_t* src, unsigned src_len,
                      uint8_t* dst, unsigned* dst_len,
                      unsigned bitsize) {
  unsigned bc = 0;
  uint32_t bb = 0;
  unsigned ilen = 0, olen = 0, last_m_off = 1;
  const unsigned oend = *dst_len;

  for (;;) {
    unsigned m_off, m_len;

    while (getbit(&bb, &bc, src, &ilen, bitsize)) {
      fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
      fail(olen >= oend, ERROR_OUTPUT_OVERRUN);
      dst[olen++] = src[ilen++];
    }
    m_off = 1;
    for (;;) {
      m_off = m_off*2 + getbit(&bb, &bc, src, &ilen, bitsize);
      fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
      fail(m_off > NRVD_UINT32_C(0xffffff) + 3, ERROR_LOOKBEHIND_OVERRUN);
      if (getbit(&bb, &bc, src, &ilen, bitsize)) break;
      m_off = (m_off-1)*2 + getbit(&bb, &bc, src, &ilen, bitsize);
    }
    if (m_off == 2) {
      m_off = last_m_off;
      m_len = getbit(&bb, &bc, src, &ilen, bitsize);
    } else {
      fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
      m_off = (m_off-3)*256 + src[ilen++];
      if (m_off == NRVD_UINT32_C(0xffffffff))
        break;
      m_len = (m_off ^ NRVD_UINT32_C(0xffffffff)) & 1;
      m_off >>= 1;
      last_m_off = ++m_off;
    }
    if (m_len) {
      m_len = 1 + getbit(&bb, &bc, src, &ilen, bitsize);
    } else if (getbit(&bb, &bc, src, &ilen, bitsize)) {
      m_len = 3 + getbit(&bb, &bc, src, &ilen, bitsize);
    } else {
      m_len++;
      do {
        m_len = m_len*2 + getbit(&bb, &bc, src, &ilen, bitsize);
        fail(ilen >= src_len, ERROR_INPUT_OVERRUN);
        fail(m_len >= oend, ERROR_OUTPUT_OVERRUN);
      } while (!getbit(&bb, &bc, src, &ilen, bitsize));
      m_len += 3;
    }
    m_len += (m_off > 0x500);
    fail(olen + m_len > oend, ERROR_OUTPUT_OVERRUN);
    fail(m_off > olen, ERROR_LOOKBEHIND_OVERRUN);
    {
      const uint8_t* m_pos;
      m_pos = dst + olen - m_off;
      dst[olen++] = *m_pos++;
      do dst[olen++] = *m_pos++; while (--m_len > 0);
    }
  }
  *dst_len = olen;
  fail(ilen < src_len, ERROR_INPUT_NOT_CONSUMED);
  fail(ilen > src_len, ERROR_INPUT_OVERRUN);
}
