/*
 * stdarg.h - variable arguments
 *
 * From the ack cc
 *
 * (c) copyright 1987 by the Vrije Universiteit, Amsterdam, The Netherlands.
 * See the copyright notice in the ACK home directory, in the file "Copyright".
 */
/* $Id$ */

#ifndef _STDARG_H
#define	_STDARG_H

typedef char* va_list;

#define __vasz(x)		((sizeof(x)+sizeof(int)-1) & ~(sizeof(int) -1))

#define va_start(a, p)	(a = (va_list)&p + __vasz(p))
#define va_arg(a, t)	(*((t *)(void *)((a += __vasz(t)) - __vasz(t))))
#define va_end(a)

#endif
