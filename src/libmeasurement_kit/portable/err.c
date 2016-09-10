/*
 * Public domain
 * err.h compatibility shim
 */

#include <measurement_kit/portable/err.h>
#include <measurement_kit/portable/stdlib.h>

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void
mkp_err(int eval, const char *fmt, ...)
{
	int sverrno = errno;
	va_list ap;

	va_start(ap, fmt);
	if (fmt != NULL) {
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, ": ");
	}
	fprintf(stderr, "%s\n", strerror(sverrno));
	exit(eval);
	va_end(ap);
}

void
mkp_errx(int eval, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (fmt != NULL)
		vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	exit(eval);
	va_end(ap);
}

void
mkp_warn(const char *fmt, ...)
{
	int sverrno = errno;
	va_list ap;

	va_start(ap, fmt);
	if (fmt != NULL) {
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, ": ");
	}
	fprintf(stderr, "%s\n", strerror(sverrno));
	va_end(ap);
}

void
mkp_warnx(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	if (fmt != NULL)
		vfprintf(stderr, fmt, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}
