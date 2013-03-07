/*
 * Copyright © 2012 oldfaber _at_ gmail.com
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 *  SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 *  RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 *  NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
 *  USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdarg.h>

#include "modbus-private.h"

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

void dbgprintf(int level_mask, int level, const char *format, ...)
{
	va_list vl;
	int len;
        /* wvsprintf has a 1024 byte limit and no floating point support */
	char putbuf[1025];
	static volatile int locked;

	if ((level_mask & level) == 0) {
		return;
	}
        putbuf[1024] = 0;
	va_start(vl, format);
        /* ASCII only */
	len = wvsprintfA(putbuf, format, vl);
	va_end(vl);
	if (locked) {
		Sleep(0);
		return;
	}
	locked = 1;
	if (len >= 1024)
		OutputDebugStringA("!!! wvsprintfA() OVEFLOW\n");
	else
		OutputDebugStringA(putbuf);
	locked = 0;
}

#else // not #defined(_WIN32)

#include <stdio.h>

void dbgprintf(int level_mask, int level, const char *format, ...)
{
	va_list vl;

	if ((level_mask & level) == 0) {
		return;
	}
	va_start(vl, format);
	vfprintf(stderr, format, vl);
	va_end(vl);
}

#endif
