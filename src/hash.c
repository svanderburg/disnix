/*
 * Disnix - A distributed application layer for Nix
 * Copyright (C) 2008  Sander van der Burg
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <openssl/sha.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static const char *base32Chars = "0123456789abcdfghijklmnpqrsvwxyz";

static char *hash_string(const unsigned char *bytes)
{
    unsigned char *ret = (unsigned char*)malloc(32 * sizeof(unsigned char));
    SHA256_CTX ctx;
    
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, bytes, strlen(bytes));
    SHA256_Final(ret, &ctx);

    return ret;
}

static unsigned char div_mod(unsigned char *bytes, unsigned char y)
{
    unsigned int borrow = 0;
    int pos = 32 - 1;
    
    while (pos >= 0 && !bytes[pos])
	--pos;

    for ( ; pos >= 0; --pos)
    {
        unsigned int s = bytes[pos] + (borrow << 8);
        unsigned int d = s / y;
        borrow = s % y;
        bytes[pos] = d;
    }

    return borrow;
}

static char *print_hash32(unsigned char *bytes)
{
    unsigned int len = (32 * 8 - 1) / 5 + 1;
    char *ret = (char*)malloc((len + 1) * sizeof(char));
    int pos, i;
    
    for(i = 0; i < len; i++)
	ret[i] = '0';
    ret[len] = '\0';
    
    pos = len - 1;
    while(pos >= 0)
    {
	unsigned char digit = div_mod(bytes, 32);
	ret[pos--] = base32Chars[digit];
    }
    
    return ret;
}

char *string_to_hash(char *string)
{
    char *hash = hash_string((unsigned char*)string);
    char *ret = print_hash32(hash); 
    free(hash);
    return ret;
}
