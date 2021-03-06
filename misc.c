/*-
 * Copyright (c) 1991, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#if 0
#ifndef lint
static char sccsid[] = "@(#)misc.c      8.1 (Berkeley) 6/6/93";
#endif /*not lint */
#endif

#include "config.h"

#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <err.h>
#include <fts.h>
#include <stdio.h>
#include <unistd.h>

#ifdef HAVE_OPENSSL_MD5_H
#include <openssl/md5.h>
#endif
#ifdef HAVE_OPENSSL_SHA_H
#include <openssl/sha.h>
#endif
#ifdef HAVE_OPENSSL_RIPEMD_H
#include <openssl/ripemd.h>
#endif

#include "mtree.h"
#include "extern.h"

typedef struct _key {
        const char *name;                       /* key name */
        u_int val;                      /* value */

#define NEEDVALUE       0x01
        u_int flags;
} KEY;

/* NB: the following table must be sorted lexically. */
static KEY keylist[] = {
        {"cksum",       F_CKSUM,        NEEDVALUE},
        {"flags",       F_FLAGS,        NEEDVALUE},
        {"gid",         F_GID,          NEEDVALUE},
        {"gname",       F_GNAME,        NEEDVALUE},
        {"ignore",      F_IGN,          0},
        {"link",        F_SLINK,        NEEDVALUE},
#ifdef HAVE_OPENSSL_MD5_H
        {"md5digest",   F_MD5,          NEEDVALUE},
#endif
        {"mode",        F_MODE,         NEEDVALUE},
        {"nlink",       F_NLINK,        NEEDVALUE},
        {"nochange",    F_NOCHANGE,     0},
        {"optional",    F_OPT,          0},
#ifdef HAVE_OPENSSL_RIPEMD_H
        {"ripemd160digest", F_RMD160,   NEEDVALUE},
#endif
#ifdef HAVE_OPENSSL_SHA_H
        {"sha1digest",  F_SHA1,         NEEDVALUE},
        {"sha256digest",        F_SHA256,               NEEDVALUE},
#endif
        {"size",        F_SIZE,         NEEDVALUE},
        {"time",        F_TIME,         NEEDVALUE},
        {"type",        F_TYPE,         NEEDVALUE},
        {"uid",         F_UID,          NEEDVALUE},
        {"uname",       F_UNAME,        NEEDVALUE},
};

int keycompare(const void *, const void *);

u_int
parsekey(char *name, int *needvaluep)
{
        KEY *k, tmp;

        tmp.name = name;
        k = (KEY *)bsearch(&tmp, keylist, sizeof(keylist) / sizeof(KEY),
            sizeof(KEY), keycompare);
        if (k == NULL)
                errx(1, "line %d: unknown keyword %s", lineno, name);

        if (needvaluep)
                *needvaluep = k->flags & NEEDVALUE ? 1 : 0;
        return (k->val);
}

int
keycompare(const void *a, const void *b)
{
        return (strcmp(((const KEY *)a)->name, ((const KEY *)b)->name));
}

char *
flags_to_string(u_long fflags)
{
        char *string;

        string = strdup("");
        if (string != NULL && *string == '\0') {
                free(string);
                string = strdup("none");
        }
        if (string == NULL)
                err(1, NULL);

        return string;
}

#define DIGEST_FILE(PREFIX, CTX, LENGTH)                        \
char *                                                          \
PREFIX ## _File(const char *filename, char *result)              \
{                                                               \
    u_char md[LENGTH];                                          \
    u_char buf[1024];                                           \
    CTX ctx;                                                    \
    FILE *fp;                                                   \
    size_t r;                                                   \
    int i;                                                      \
                                                                \
    PREFIX ## _Init(&ctx);                                      \
    if ((fp = fopen(filename, "r")) == NULL)                    \
        return NULL;                                            \
    while ((r = fread(buf, 1, sizeof(buf), fp)) != 0)           \
        PREFIX ## _Update(&ctx, buf, r);                        \
    if (ferror(fp)) {                                           \
        fclose(fp);                                             \
        return NULL;                                            \
    }                                                           \
    fclose(fp);                                                 \
    PREFIX ## _Final(md, &ctx);                                 \
    for (i = 0; i < LENGTH; i++)                                \
        sprintf(result + 2 * i, "%02x", md[i]);                 \
    return result;                                              \
}

#ifdef HAVE_OPENSSL_MD5_H
DIGEST_FILE(MD5, MD5_CTX, MD5_DIGEST_LENGTH);
#endif

#ifdef HAVE_OPENSSL_SHA_H
DIGEST_FILE(SHA1, SHA_CTX, SHA_DIGEST_LENGTH);
DIGEST_FILE(SHA256, SHA256_CTX, SHA256_DIGEST_LENGTH);
#endif

#ifdef HAVE_OPENSSL_RIPEMD_H
DIGEST_FILE(RIPEMD160, RIPEMD160_CTX, RIPEMD160_DIGEST_LENGTH);
#endif

