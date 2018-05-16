/*
 * unfilter.c: educational C code for UPX post-decompression filters
 * by pts@fazekas.hu at Wed May 16 15:06:28 CEST 2018
 *
 * This C code compiles, otherwise it is untested!
 *
 * Compile with: gcc -c -O2 -W -Wall -Wextra -Werror unfilter.c
 *
 * Code based on upx-3.93-src/src/{filteri.cpp,ctok.h} .
 */

#include <assert.h>
#include <stdint.h>

/* Corresponding i386 assembly instructions matched:
 *
 * 00000000  E890909090        call     0x90909095
 * 00000005  E990909090        jmp      0x9090909a
 * 0000000A  0F8090909090      jo  near 0x909090a0
 * 00000010  0F8190909090      jno near 0x909090a6
 * 00000016  0F8290909090      jc  near 0x909090ac
 * 0000001C  0F8390909090      jnc near 0x909090b2
 * 00000022  0F8490909090      jz  near 0x909090b8
 * 00000028  0F8590909090      jnz near 0x909090be
 * 0000002E  0F8690909090      jna near 0x909090c4
 * 00000034  0F8790909090      ja  near 0x909090ca
 * 0000003A  0F8890909090      js  near 0x909090d0
 * 00000040  0F8990909090      jns near 0x909090d6
 * 00000046  0F8A90909090      jpe near 0x909090dc
 * 0000004C  0F8B90909090      jpo near 0x909090e2
 * 00000052  0F8C90909090      jl  near 0x909090e8
 * 00000058  0F8D90909090      jnl near 0x909090ee
 * 0000005E  0F8E90909090      jng near 0x909090f4
 * 00000064  0F8F90909090      jg  near 0x909090fa
 */
void unfilter(uint8_t *b, uint32_t b_size,
              uint8_t filter_id, uint8_t filter_cto,
              uint32_t addvalue  /* Typically 0. */) {
  assert(filter_id == 0 || filter_id == 0x46 || filter_id == 0x49);
  if (b_size > 5 && filter_id != 0) {  /* u_ctok32_e8e9_bswap_le */
    uint8_t *b0 = b, *b_end = b + b_size - 5, *lastcall = b;
    while (b != b_end) {
      if ((*b == 0xe8 || *b == 0xe9) ||
          (filter_id == 0x49 && (lastcall != b && 0xf == *(b - 1) && 0x80 <= *b && *b <= 0x8f))
         ) {
        if (*++b == filter_cto) {
          uint32_t jc = (uint32_t)*(b + 1) << 16 | (uint32_t)*(b + 2) << 8 | *(b + 3);  /* get_be32(b). */
          jc -= b - b0 + addvalue;
          *b++ = jc; *b++ = jc >> 8; *b++ = jc >> 16; *b++ = jc >> 24;  /* set_le32(b, jc); b += 4; */
          lastcall = b;
        }
      } else {
        ++b;
      }
    }
  }
}
