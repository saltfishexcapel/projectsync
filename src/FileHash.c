/**
 * MD5
 */
#include "FileHash.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static o_uint Ti[64]        = {0};
static bool   is_Ti_changed = false;

static void
ti_init ()
{
        for (int i = 1; i <= 64; ++i)
                Ti[i] = 4294967296u * (o_uint)abs ((int)(sin ((double)i)));
}

const unsigned int S[4][4] = {
        {7, 12, 17, 22},
        {5,  9, 14, 20},
        {4, 11, 16, 23},
        {6, 10, 15, 21}
};

static inline o_uint
_F (o_uint x, o_uint y, o_uint z)
{
        return ((x & y) | ((~x) & z));
}

static inline o_uint
_G (o_uint x, o_uint y, o_uint z)
{
        return ((x & y) | (y & (~z)));
}

static inline o_uint
_H (o_uint x, o_uint y, o_uint z)
{
        return (x ^ y ^ z);
}

static inline o_uint
_I (o_uint x, o_uint y, o_uint z)
{
        return (y ^ (x | (~z)));
}

static o_uint (*_Func_along[4]) (o_uint, o_uint, o_uint) = {_F, _G, _H, _I};

static inline o_uint
s_move (o_uint num, int offset)
{
        return ((num << offset) | (num >> (sizeof (o_uint) - offset)));
}

static void
md5_func (o_uint* a,
          o_uint  b,
          o_uint  c,
          o_uint  d,
          o_uint  M,
          int     s,
          o_uint  ti,
          int     _Fnum)
{
        *a = b + s_move ((*a + _Func_along[_Fnum](b, c, d) + M + ti), s);
}

bool
file_hash_generate (FileHash* fhash, const char* file_name)
{
        FILE*   fp;
        size_t  fsize, k, read_status;
        o_uint* a[4] = {&fhash->p1, &fhash->p2, &fhash->p3, &fhash->p4};
        o_uint  chunk[16];
        unsigned char* _chunk = (unsigned char*)chunk;

        if (!fhash || !file_name)
                return false;
        fp = fopen (file_name, "r");
        if (!fp)
                return false;
        fseek (fp, 0, SEEK_END);
        fsize = ftell (fp);

        /*初始化 Ti*/
        if (!is_Ti_changed) {
                ti_init ();
                is_Ti_changed = true;
        }

        /*初始化 ABCD*/
        *a[3] = 0x76543210;
        *a[2] = 0xfedcba98;
        *a[1] = 0x01234567;
        *a[0] = 0x89abcdef;

        k     = fsize % 64;
        fseek (fp, 0, SEEK_SET);

retry_read:
        read_status = fread (O_PTR (chunk), 64, 1, fp);
        if (read_status != 1) {
                memset (&_chunk[k], 0, (64 - k));
                memcpy (&chunk[14], &fsize, 8);
        }
        for (int i = 0; i < 4; ++i) {
                for (int j = 0; j < 16; ++j) {
                        md5_func (a[(j + 3) % 4],
                                  *a[(j + 2) % 4],
                                  *a[(j + 1) % 4],
                                  *a[j % 4],
                                  chunk[j],
                                  S[i][j % 4],
                                  Ti[16 * i + j],
                                  i);
                }
        }
        if (read_status == 1)
                goto retry_read;

        fclose (fp);
        return true;
}