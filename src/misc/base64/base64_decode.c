/* LibTomCrypt, modular cryptographic library -- Tom St Denis
 *
 * LibTomCrypt is a library that provides various cryptographic
 * algorithms in a highly modular and flexible manner.
 *
 * The library is free for all purposes without any express
 * guarantee it works.
 *
 * Tom St Denis, tomstdenis@gmail.com, http://libtom.org
 */
#include "tomcrypt.h"

/**
  @file base64_decode.c
  Compliant base64 code donated by Wayne Scott (wscott@bitmover.com)
  base64 URL Safe variant (RFC 4648 section 5) by Karel Miko
*/


#if defined(LTC_BASE64) || defined (LTC_BASE64_URL)

#if defined(LTC_BASE64)
static const unsigned char map_base64[256] = {
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255,  62, 255, 255, 255,  63,
 52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
  7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
 19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255, 255,
255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
 37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
 49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255 };
#endif /* LTC_BASE64 */

#if defined(LTC_BASE64_URL)
static const unsigned char map_base64url[256] = {
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255, 255,
 52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,
255, 254, 255, 255, 255,   0,   1,   2,   3,   4,   5,   6,
  7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,
 19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255,  63,
255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,
 37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,
 49,  50,  51, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
255, 255, 255, 255 };
#endif /* LTC_BASE64_URL */

static int _base64_decode_internal(const unsigned char *in,  unsigned long inlen,
                                 unsigned char *out, unsigned long *outlen,
                           const unsigned char *map, int strict)
{
   unsigned long t, x, y, z;
   unsigned char c;
   int           g;

   LTC_ARGCHK(in     != NULL);
   LTC_ARGCHK(out    != NULL);
   LTC_ARGCHK(outlen != NULL);

   g = 3;
   for (x = y = z = t = 0; x < inlen; x++) {
       c = map[in[x]&0xFF];
       if (c == 255) {
          if (strict)
             return CRYPT_INVALID_PACKET;
          else
             continue;
       }
       /* the final = symbols are read and used to trim the remaining bytes */
       if (c == 254) {
          c = 0;
          /* prevent g < 0 which would potentially allow an overflow later */
          if (--g < 0) {
             return CRYPT_INVALID_PACKET;
          }
       } else if (g != 3) {
          /* we only allow = to be at the end */
          return CRYPT_INVALID_PACKET;
       }

       t = (t<<6)|c;

       if (++y == 4) {
          if (z + g > *outlen) {
             return CRYPT_BUFFER_OVERFLOW;
          }
          out[z++] = (unsigned char)((t>>16)&255);
          if (g > 1) out[z++] = (unsigned char)((t>>8)&255);
          if (g > 2) out[z++] = (unsigned char)(t&255);
          y = t = 0;
       }
   }
   if (y != 0) {
       return CRYPT_INVALID_PACKET;
   }
   *outlen = z;
   return CRYPT_OK;
}

#if defined(LTC_BASE64)
/**
   base64 decode a block of memory
   @param in       The base64 data to decode
   @param inlen    The length of the base64 data
   @param out      [out] The destination of the binary decoded data
   @param outlen   [in/out] The max size and resulting size of the decoded data
   @return CRYPT_OK if successful
*/
int base64_decode_ex(const unsigned char *in,  unsigned long inlen,
                        unsigned char *out, unsigned long *outlen, int strict)
{
    return _base64_decode_internal(in, inlen, out, outlen, map_base64, strict);
}
#endif /* LTC_BASE64 */

#if defined(LTC_BASE64_URL)
/**
   base64 (URL Safe, RFC 4648 section 5) decode a block of memory
   @param in       The base64 data to decode
   @param inlen    The length of the base64 data
   @param out      [out] The destination of the binary decoded data
   @param outlen   [in/out] The max size and resulting size of the decoded data
   @return CRYPT_OK if successful
*/
int base64url_decode_ex(const unsigned char *in,  unsigned long inlen,
                           unsigned char *out, unsigned long *outlen, int strict)
{
    return _base64_decode_internal(in, inlen, out, outlen, map_base64url, strict);
}
#endif /* LTC_BASE64_URL */

#endif


/* $Source$ */
/* $Revision$ */
/* $Date$ */
