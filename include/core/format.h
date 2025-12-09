/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef CORE_FORMAT_H
#define CORE_FORMAT_H
#pragma once

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint_least32_t __jacl_fmt_t;

// ============= BASE: bits 0–5 =============
#define JACL_FMT_BASE_int     0
#define JACL_FMT_BASE_bin     2
#define JACL_FMT_BASE_oct     8
#define JACL_FMT_BASE_hex     16
#define JACL_FMT_BASE_dec     10
#define JACL_FMT_BASE_ptr     16
#define JACL_FMT_BASE_MASK    0x3F   // bits 0–5 (2–36, or 0=auto, 10=dec, etc.)
#define JACL_FMT_BASE_exp     JACL_FMT_BASE_dec
#define JACL_FMT_BASE_alt     JACL_FMT_BASE_hex

// ============ LENGTH: bits 6–8 ============
#define JACL_FMT_LENGTH_0    (0 << 6)
#define JACL_FMT_LENGTH_hh   (1 << 6)
#define JACL_FMT_LENGTH_h    (2 << 6)
#define JACL_FMT_LENGTH_l    (3 << 6)
#define JACL_FMT_LENGTH_ll   (4 << 6)
#define JACL_FMT_LENGTH_z    (5 << 6)
#define JACL_FMT_LENGTH_t    (6 << 6)
#define JACL_FMT_LENGTH_j    (7 << 6)
#define JACL_FMT_LENGTH_L    JACL_FMT_LENGTH_ll
#define JACL_FMT_LENGTH_MASK (7 << 6) // bits 6–8

// ======== FLAGS: single bits, 9+ ========
#define JACL_FMT_FLAG_sign   (1 << 9)
#define JACL_FMT_FLAG_upper  (1 << 10)
#define JACL_FMT_FLAG_left   (1 << 11)
#define JACL_FMT_FLAG_pref   (1 << 12)
#define JACL_FMT_FLAG_plus   (1 << 13)
#define JACL_FMT_FLAG_spad   (1 << 14)
#define JACL_FMT_FLAG_zpad   (1 << 15)
#define JACL_FMT_FLAG_ksep   (1 << 16)
#define JACL_FMT_FLAG_wchar  (1 << 17)
#define JACL_FMT_FLAG_noout  JACL_FMT_FLAG_left

#define JACL_FMT_WIDTH_str   INT_MAX
#define JACL_FMT_WIDTH_char  1
#define JACL_FMT_WIDTH_num   INT_MAX

// ============= UNIFIED MACROS =============
#define JACL_FMT_VAL(PRE, suf)        (JACL_FMT_##PRE##_##suf)
#define JACL_FMT_GET(PRE, spec)       ((spec) & JACL_FMT_##PRE##_MASK)
#define JACL_FMT_ADD(PRE, spec, suf)  (spec |= JACL_FMT_VAL(PRE, suf))
#define JACL_FMT_SET(PRE, spec, suf)  (spec = (spec & ~JACL_FMT_##PRE##_MASK) | JACL_FMT_##PRE##_##suf)
#define JACL_FMT_CHK(PRE, spec, suf)  (JACL_FMT_GET(PRE, spec) == JACL_FMT_VAL(PRE, suf))
#define JACL_FMT_HAS(PRE, spec, suf)  ((spec) & JACL_FMT_VAL(PRE, suf))

// ============ SPECIAL VALUES ==============
#define JACL_VAL_NAN 0
#define JACL_VAL_INF 1

// ============= MATH FUNCTIONS =============
#if JACL_HAS_C99
#define POW(x, n)   powl((x), (n))
#else
#define POW(x, n)   ((long double)pow((double)(x), (double)(n)))
#endif

// ============= CHAR CONSTANTS =============
static const char NANC[] = "NnAaNn", INFC[] = "IiNnFfIiNnIiTtYy", LD[] = "0123456789abcdef", UD[] = "0123456789ABCDEF", D100[] =
	"0001020304050607080910111213141516171819202122232425262728293031323334353637383940414243444546474849"
	"5051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899";

// =========== WRITE PRIMITIVES ============
#define __jacl_write_char(stream, out, rem, len, ch) do { \
	if (stream) fputc((ch), stream); \
	else if (out && *out && *rem > 0) { *(*out)++ = (ch); (*rem)--; }\
	(len)++; \
} while(0)
#define __jacl_write_pad(stream, out, rem, len, ch, pad) do { int __i = (pad); while (__i-- > 0) __jacl_write_char(stream, out, rem, len, (ch)); } while(0)
#define __jacl_write_sign(stream, out, rem, len, spec, neg) do { if (neg) __jacl_write_char(stream, out, rem, len, '-'); else if ((spec) & JACL_FMT_FLAG_plus) __jacl_write_char(stream, out, rem, len, '+'); else if JACL_FMT_HAS(FLAG, spec, spad) __jacl_write_char(stream, out, rem, len, ' '); } while(0)

// ============ READ PRIMITIVES ============
#define __jacl_isspace(c) ((c)==' ' || (c)=='\t' || (c)=='\n' || (c)=='\r' || (c)=='\f' || (c)=='\v')
#define __jacl_read_next(ch, stream, in, read) do { if ((in) != NULL && (*in) != NULL) { (ch) = (**in) ? *(*in)++ : EOF; if ((ch) != EOF) (*read)++; } else { (ch) = fgetc((stream) ? (stream) : stdin); if ((ch) != EOF) (*read)++; } } while(0)
#define __jacl_read_back(ch, stream, in, read) do { if (in) { (*in)--; } else { ungetc((ch), (stream)); } (*read)--; } while(0)
#define __jacl_read_skip(ch, stream, in, read) do { do { __jacl_read_next(ch, stream, in, read); } while ((ch) != EOF && __jacl_isspace(ch)); if ((ch) != EOF) __jacl_read_back(ch, stream, in, read); } while(0)
#define __jacl_read_digits(ch, stream, in, read, width, count, limit, base, val, err) do { \
	while (*read < width && ch != EOF) { \
		int dval = -1; \
		if (ch >= '0' && ch <= '9') dval = ch - '0'; \
		else if (ch >= 'a') dval = ch - 'a' + 10; \
		else if (ch >= 'A') dval = ch - 'A' + 10; \
		if (dval < 0 || dval >= base) break; \
		if (val > (UINTMAX_MAX - dval) / base) { err = ERANGE; val = UINTMAX_MAX; } \
		else val = val * base + dval; \
		if (++count >= limit) break; \
		__jacl_read_next(ch, stream, in, read); \
	} \
} while(0)

// ============= SPEC FUNCTIONS =============
#define CASE(ch, N) case ch: (*fmt)++; return JACL_FMT_VAL(LENGTH, N)
static inline int __jacl_spec_length(const char **restrict fmt) {
	switch (**fmt) {
	#if JACL_HAS_C99
	case 'l': return ((*fmt)[1] == 'l') ? (*fmt += 2, JACL_FMT_LENGTH_ll) : ((*fmt)++, JACL_FMT_LENGTH_l);
	case 'h': return ((*fmt)[1] == 'h') ? (*fmt += 2, JACL_FMT_LENGTH_hh) : ((*fmt)++, JACL_FMT_LENGTH_h);
	CASE('z', z); CASE('t', t); CASE('j', j); CASE('L', L);
	#else
	CASE('l', l); CASE('h', h);
	#endif
	default: return 0;
	}
}
#undef CASE
static inline int __jacl_spec_value(const char **fmt, va_list ap) {
	int val = 0;

	if (**fmt == '*' && ap) { (*fmt)++; return va_arg(ap, int); }
	while (**fmt >= '0' && **fmt <= '9') { val = val * 10 + (**fmt - '0'); (*fmt)++; }

	return val;
}
#define CASE(ch, name) case ch: JACL_FMT_ADD(FLAG, spec, name); (*fmt)++; break
static inline __jacl_fmt_t __jacl_spec_printf(va_list ap, const char **fmt, int *prec, int *width, char *type) {
	__jacl_fmt_t spec = 0;
	int chk = 1;

	while (chk) {
		switch (**fmt) {
		CASE('-', left); CASE('+', plus); CASE(' ', spad); CASE('#', pref); CASE('0', zpad); CASE('\'', ksep);
		default: chk = 0; break;
		}
	}

	*width = __jacl_spec_value(fmt, ap);
	*prec = -1;

	if (*width < 0) { JACL_FMT_ADD(FLAG, spec, left); *width = -*width; }
	if (**fmt == '.') { (*fmt)++; *prec = __jacl_spec_value(fmt, ap); }

	spec |= __jacl_spec_length(fmt);

	if ((unsigned)(**fmt - 'A') < 26) JACL_FMT_ADD(FLAG, spec, upper);
	if (**fmt == 'd' || **fmt == 'i') JACL_FMT_ADD(FLAG, spec, sign);

	*type = **fmt;

	return spec;
}
#undef CASE
static inline __jacl_fmt_t __jacl_spec_scanf(const char **fmt, int *width, char *type) {
	__jacl_fmt_t spec = 0;

	if (**fmt == '*') { JACL_FMT_ADD(FLAG, spec, noout); (*fmt)++; }

	*width = __jacl_spec_value(fmt, NULL);
	spec |= __jacl_spec_length(fmt);
	*type = **fmt;

	if(*type != 'c') JACL_FMT_ADD(FLAG, spec, spad);
	if(*type != 'u') JACL_FMT_ADD(FLAG, spec, sign);
	if ((unsigned)(**fmt - 'A') < 26) JACL_FMT_ADD(FLAG, spec, upper);

	return spec;
}

// ============ HELPER FUNCTIONS ============
#define CASE(N,type) case JACL_FMT_VAL(BASE,type): return N; break
static inline int __jacl_get_base(__jacl_fmt_t spec) {
	switch (JACL_FMT_GET(BASE, spec)) { \
	CASE(10, dec); CASE(2, bin); CASE(8, oct); CASE(16, hex);
	} \
}
#undef CASE
#define CASE(N,type) case JACL_FMT_VAL(LENGTH,N): return (uintmax_t)va_arg(ap, type); break
static inline uintmax_t __jacl_get_signed(__jacl_fmt_t spec, va_list ap) {
	switch (JACL_FMT_GET(LENGTH, spec)) {
	CASE(h, int); CASE(l, long);
	#if JACL_HAS_C99
	CASE(hh, int); CASE(ll, long long); CASE(z, ssize_t); CASE(t, ptrdiff_t); CASE(j, intmax_t);
	#endif
	default: return (uintmax_t)va_arg(ap, int);
	}
}
static inline uintmax_t __jacl_get_unsigned(__jacl_fmt_t spec, va_list ap) {
	switch (JACL_FMT_GET(LENGTH, spec)) {
	CASE(h, unsigned int); CASE(l, unsigned long);
	#if JACL_HAS_C99
	CASE(hh, unsigned int); CASE(ll, unsigned long long); CASE(z, size_t); CASE(t, size_t); CASE(j, uintmax_t);
	#endif
	default: return va_arg(ap, unsigned int);
	}
}

#undef CASE
#define CASE(N,type) case JACL_FMT_VAL(LENGTH,N): return (long double)va_arg(ap, type); break
static inline long double __jacl_get_float(__jacl_fmt_t spec, va_list ap) {
	switch (JACL_FMT_GET(LENGTH, spec)) {
	CASE(l, double);
	#if JACL_HAS_C99 && JACL_LDBL_BITS > 64
	CASE(L, long double);
	#endif
	default: return (long double)va_arg(ap, double);
	}
}
#undef CASE
#define CASE(N,type) case JACL_FMT_VAL(LENGTH,N): *va_arg(ap, type*) = (type)v; break
static inline void __jacl_set_signed(__jacl_fmt_t spec, va_list ap, uintmax_t v) {
	switch (JACL_FMT_GET(LENGTH, spec)) {
	CASE(h, short); CASE(l, long);
	#if JACL_HAS_C99
	CASE(hh, signed char); CASE(ll, long long); CASE(z, ssize_t); CASE(t, ptrdiff_t); CASE(j, intmax_t);
	#endif
	default: *va_arg(ap, int*) = (int)v; break;
	}
}
static inline void __jacl_set_unsigned(__jacl_fmt_t spec, va_list ap, uintmax_t v) {
	switch (JACL_FMT_GET(LENGTH, spec)) {
	CASE(h, unsigned short); CASE(l, unsigned long);
	#if JACL_HAS_C99
	CASE(hh, unsigned char); CASE(ll, unsigned long long); CASE(z, size_t); CASE(t, size_t); CASE(j, uintmax_t);
	#endif
	default: *va_arg(ap, unsigned int*) = (unsigned int)v; break;
	}
}
#undef CASE

// ============ OUTPUT FUNCTIONS ============
static inline int __jacl_output_str(FILE * stream, char **out, size_t *rem, __jacl_fmt_t spec, const char *str, int prec, int width) {
	int len = 0, slen = 0, pad = 0, pos = 0;
	const char *s = str;

	if (prec < 0) prec = INT_MAX;
	while (s[slen] && slen < prec) slen++;

	pad = (width > slen) ? width - slen : 0;

	if (!JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);
	while (pos < slen) __jacl_write_char(stream, out, rem, len, str[pos++]);
	if (JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);

	return len;
}
#define CASE(base, mask, shift) case JACL_FMT_VAL(BASE,base): while (val) { buf[pos++] = digits[val & mask]; val >>= shift; } break;
static inline int __jacl_output_int(FILE * stream, char **out, size_t *rem, const __jacl_fmt_t spec, uintmax_t val, int prec, int width) {
	int pos = 0, num, neg = (JACL_FMT_HAS(FLAG, spec, sign) && (intmax_t)val < 0), pad, len = 0, sign = JACL_FMT_HAS(FLAG, spec, sign) && (neg || JACL_FMT_HAS(FLAG, spec, plus) || JACL_FMT_HAS(FLAG, spec, spad)), prefix = 0;
	const char *digits = JACL_FMT_HAS(FLAG, spec, upper) ? UD : LD;
	char buf[64] = {0};

	if (neg) val = (uintmax_t)(-(intmax_t)val);
	if (JACL_FMT_HAS(FLAG, spec, pref) && val) {
		if (JACL_FMT_CHK(BASE, spec, hex) || JACL_FMT_CHK(BASE, spec, bin)) prefix = 2;
		else if (JACL_FMT_CHK(BASE, spec, oct)) prefix = 1;
	}
	if (!(prec == 0 && val == 0)) {
		if (val == 0) buf[pos++] = '0';
		else switch (JACL_FMT_GET(BASE, spec)) {
		CASE(hex, 0xF, 4); CASE(oct, 0x7, 3); CASE(bin, 0x1, 1);
		default:
			while (val >= 100) {
				int digits = val % 100;
				buf[pos++] = D100[digits * 2 + 1];
				buf[pos++] = D100[digits * 2];
				val /= 100;
			}
			if (val >= 10) {
				buf[pos++] = D100[val * 2 + 1];
				buf[pos++] = D100[val * 2];
			} else buf[pos++] = '0' + val;
		}
	}
	while (pos < prec && pos < 64) buf[pos++] = '0';

	pad = (width > pos + sign + prefix) ? width - pos - sign - prefix : 0;

	if (!stream && (!out || !*out)) return pos + sign + prefix + pad;
	if (!JACL_FMT_HAS(FLAG, spec, left) && !(JACL_FMT_HAS(FLAG, spec, zpad) && (prec < 0))) __jacl_write_pad(stream, out, rem, len, ' ', pad);
	if (sign) __jacl_write_sign(stream, out, rem, len, spec, neg);
	if (prefix) { __jacl_write_char(stream, out, rem, len, '0'); if (prefix == 2) __jacl_write_char(stream, out, rem, len, JACL_FMT_HAS(FLAG, spec, upper) ? (JACL_FMT_CHK(BASE, spec, hex) ? 'X' : 'B') : (JACL_FMT_CHK(BASE, spec, hex) ? 'x' : 'b')); }
	if (!JACL_FMT_HAS(FLAG, spec, left) && (JACL_FMT_HAS(FLAG, spec, zpad) && (prec < 0))) __jacl_write_pad(stream, out, rem, len, '0', pad);
	while (pos--) __jacl_write_char(stream, out, rem, len, buf[pos]);
	if (JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);

	return len;
}
#undef CASE
static inline int __jacl_output_special(FILE * stream, char **out, size_t *rem, const __jacl_fmt_t spec, int val, int neg, int width) {
	int sign = (neg || JACL_FMT_HAS(FLAG, spec, plus) || JACL_FMT_HAS(FLAG, spec, spad)), count = sign + 3, pad = 0, len = 0;

	pad = (width > count) ? width - count : 0;

	if (!stream && (!out || !*out)) return count + pad;
	if (!JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);
	if (sign) __jacl_write_sign(stream, out, rem, len, spec, neg);
	if (val == JACL_VAL_NAN) len += __jacl_output_str(stream, out, rem, spec, JACL_FMT_HAS(FLAG, spec, upper) ? "NAN" : "nan", 3, 0);
	if (val == JACL_VAL_INF) len += __jacl_output_str(stream, out, rem, spec, JACL_FMT_HAS(FLAG, spec, upper) ? "INF" : "inf", 3, 0);
	if (JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);

	return len;
}
static inline int __jacl_output_float(FILE * stream, char **out, size_t *rem, const __jacl_fmt_t spec, long double val, int prec, int width) {
	if (prec < 0) prec = 6;

	__jacl_fdigits_t dec = __jacl_fdigits(val, prec, 0, JACL_FMT_CHK(LENGTH, spec, L));

	int i, len = 0, sign = (dec.sign || JACL_FMT_HAS(FLAG, spec, plus) || JACL_FMT_HAS(FLAG, spec, spad)), ipart = (dec.exp >= 0) ? (dec.exp + 1) : 0, digits = dec.end - dec.start;
	int whole = (ipart < digits) ? ipart : digits, lead = (dec.exp < 0) ? -(dec.exp + 1) : 0, post = (i = digits - whole) < (prec - lead) ? i : (prec - lead), trail, pad, total;

	if (dec.digits[0] > '9') return __jacl_output_special(stream, out, rem, spec, (val != val) ? JACL_VAL_NAN : JACL_VAL_INF, dec.sign, width);
	if (post < 0) post = 0;

	trail = prec - lead - post;
	trail = trail < 0 ? 0 : trail;
	total = sign + (ipart ? ipart : 1) + (prec > 0 || JACL_FMT_HAS(FLAG, spec, pref)) + lead + post + trail;
	pad = (width > total) ? width - total : 0;

	if (!stream && (!out || !*out)) return total + pad;
	if (!JACL_FMT_HAS(FLAG, spec, left) && !JACL_FMT_HAS(FLAG, spec, zpad)) __jacl_write_pad(stream, out, rem, len, ' ', pad);
	if (sign) __jacl_write_sign(stream, out, rem, len, spec, dec.sign);
	if (!JACL_FMT_HAS(FLAG, spec, left) && JACL_FMT_HAS(FLAG, spec, zpad)) __jacl_write_pad(stream, out, rem, len, '0', pad);
	if (!ipart) __jacl_write_char(stream, out, rem, len, '0');
	else { for (i = 0; i < whole; i++) __jacl_write_char(stream, out, rem, len, dec.digits[dec.start + i]); for (i = whole; i < ipart; i++) __jacl_write_char(stream, out, rem, len, '0'); }
	if (prec > 0 || JACL_FMT_HAS(FLAG, spec, pref)) __jacl_write_char(stream, out, rem, len, '.');
	for (i = 0; i < lead; i++) __jacl_write_char(stream, out, rem, len, '0');
	for (i = 0; i < post; i++) __jacl_write_char(stream, out, rem, len, dec.digits[dec.start + whole + i]);
	for (i = 0; i < trail; i++) __jacl_write_char(stream, out, rem, len, '0');
	if (JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);

	return len;
}
static inline int __jacl_output_exp(FILE * stream, char **out, size_t *rem, const __jacl_fmt_t spec, long double val, int prec, int width) {
	if (prec < 0) prec = 6;

	__jacl_fmt_t blank = {0};
	__jacl_fdigits_t dec = __jacl_fdigits(val, 0, prec + 1, JACL_FMT_CHK(LENGTH, spec, L));
	int i, len = 0, sign = (dec.sign || JACL_FMT_HAS(FLAG, spec, plus) || JACL_FMT_HAS(FLAG, spec, spad)), expval = (dec.exp < 0) ? -dec.exp : dec.exp, expdig = (expval < 10) ? 2 : (expval < 100) ? 2 : 3;
	int total = sign + 1 + (prec > 0 || JACL_FMT_HAS(FLAG, spec, pref)) + prec + 2 + expdig, pad = (width > total) ? width - total : 0, remain;

	if (dec.digits[0] > '9') return __jacl_output_special(stream, out, rem, spec, (val != val) ? JACL_VAL_NAN : JACL_VAL_INF, dec.sign, width);
	if (!stream && (!out || !*out)) return total + pad;
	if (!JACL_FMT_HAS(FLAG, spec, left) && !JACL_FMT_HAS(FLAG, spec, zpad)) __jacl_write_pad(stream, out, rem, len, ' ', pad);
	if (sign) __jacl_write_sign(stream, out, rem, len, spec, dec.sign);
	if (!JACL_FMT_HAS(FLAG, spec, left) && JACL_FMT_HAS(FLAG, spec, zpad)) __jacl_write_pad(stream, out, rem, len, '0', pad);

	__jacl_write_char(stream, out, rem, len, dec.digits[dec.start]);

	if (prec > 0 || JACL_FMT_HAS(FLAG, spec, pref)) __jacl_write_char(stream, out, rem, len, '.');
	for (i = dec.start + 1, remain = prec; i < dec.end && remain > 0; i++, remain--) __jacl_write_char(stream, out, rem, len, dec.digits[i]);
	while (remain-- > 0) __jacl_write_char(stream, out, rem, len, '0');

	__jacl_write_char(stream, out, rem, len, JACL_FMT_HAS(FLAG, spec, upper) ? 'E' : 'e');
	__jacl_write_char(stream, out, rem, len, dec.exp < 0 ? '-' : '+');
	len += __jacl_output_int(stream, out, rem, blank, expval, 2, 0);

	if (JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);

	return len;
}
static inline int __jacl_output_gen(FILE * stream, char **out, size_t *rem, const __jacl_fmt_t spec, long double val, int prec, int width) {
	char buf[256], *ptr = buf;
	size_t size = sizeof(buf);
	long double check = __jacl_signclr_LDBL(val);
	int i, ex, ep, dp, total, neg = __jacl_signget_LDBL(val), sign = (neg || JACL_FMT_HAS(FLAG, spec, plus) || JACL_FMT_HAS(FLAG, spec, spad)), len = 0, pad = 0, exp;
	__jacl_fmt_t blank = {0};

	blank |= JACL_FMT_HAS(FLAG, spec, upper);

	if (prec < 0) prec = 6;
	if (prec == 0) prec = 1;
	if (prec > LDBL_DIG) prec = LDBL_DIG;
	if (isnan(val)) return __jacl_output_special(stream, out, rem, spec, JACL_VAL_NAN, neg, width);
	if (isinf(val)) return __jacl_output_special(stream, out, rem, spec, JACL_VAL_INF, neg, width);

	// Calculate exponent
	for (exp = 0; check >= 10.0; check *= 0.1, exp++);
	for (; check < 1.0 && check > 0.0; check *= 10.0, exp--);

	// Format using exp or float, with absolute value
	val = __jacl_signclr_LDBL(val);
	total = (exp < -4 || exp >= prec)
	      ? __jacl_output_exp(NULL, &ptr, &size, blank, val, prec - 1, 0)
	      : __jacl_output_float(NULL, &ptr, &size, blank, val, (prec - exp - 1 > 0) ? prec - exp - 1 : 0, 0);

	// Find exponent and decimal point
	for (ep = 0; ep < total && buf[ep] != 'e' && buf[ep] != 'E'; ep++);
	for (dp = 0; dp < ep && buf[dp] != '.'; dp++);

	// Strip trailing zeros only if there's a decimal point
	if (dp < (ex = ep)) {
		while (ex > dp + 1 && buf[ex - 1] == '0') ex--;
		if (ex > 0 && !JACL_FMT_HAS(FLAG, spec, pref) && buf[ex - 1] == '.') ex--;
	}

	// Copy exponent forward if present
	while (ep < total) buf[ex++] = buf[ep++];

	// Calculate padding
	if (width > sign + ex) pad = width - sign - ex;
	if (!stream && (!out || !*out)) return sign + pad + ex;

	// Output with padding
	if (!JACL_FMT_HAS(FLAG, spec, left) && !JACL_FMT_HAS(FLAG, spec, zpad)) __jacl_write_pad(stream, out, rem, len, ' ', pad);
	if (sign) __jacl_write_sign(stream, out, rem, len, spec, neg);
	if (!JACL_FMT_HAS(FLAG, spec, left) && JACL_FMT_HAS(FLAG, spec, zpad)) __jacl_write_pad(stream, out, rem, len, '0', pad);
	for (i = 0; i < ex; i++) __jacl_write_char(stream, out, rem, len, buf[i]);
	if (JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);

	return len;
}
static inline int __jacl_output_alt(FILE * stream, char **out, size_t *rem, const __jacl_fmt_t spec, long double val, int prec, int width) {
	char hex[LDBL_MANT_DIG / 4 + 2] = {0};
	int exp, neg = __jacl_signget_LDBL(val), len = 0, leading = 0, num_digits = 0, max_digits = LDBL_MANT_DIG / 4, limit, exponent_absolute, total, padding, i;
	int sign = (neg || JACL_FMT_HAS(FLAG, spec, plus) || JACL_FMT_HAS(FLAG, spec, spad));
	const char *digits_ptr;
	long double mantissa = val;

	if (isnan(val)) return __jacl_output_special(stream, out, rem, spec, JACL_VAL_NAN, neg, width);
	if (isinf(val)) return __jacl_output_special(stream, out, rem, spec, JACL_VAL_INF, neg, width);

	// Extract mantissa and exponent
	if (val != 0.0L) {
		exp = __jacl_expfind_LDBL(__jacl_signclr_LDBL(val));
		mantissa = __jacl_expscal_LDBL(val, -exp); \
		leading = (int)mantissa;
		mantissa -= leading;
		exp--;
	}

	// Generate hex digits with optional rounding
	if (prec < 0) {
		while (mantissa > 0.0L && num_digits < max_digits) {
			mantissa = __jacl_expscal_LDBL(mantissa, 4);
			hex[num_digits] = (char)(int)mantissa;
			mantissa -= hex[num_digits++];
		}
	} else {
		limit = (prec > max_digits) ? max_digits : prec;

		for (num_digits = 0; num_digits <= limit; num_digits++) { hex[num_digits] = (char)(int)__jacl_expscal_LDBL(mantissa, 4); mantissa = __jacl_expscal_LDBL(mantissa, 4) - hex[num_digits]; }

		// Round at limit position
		if (hex[limit] >= 8) {
			for (num_digits = limit - 1; num_digits >= 0 && ++hex[num_digits] >= 16; num_digits--) hex[num_digits] = 0;
			if (num_digits < 0 && ++leading >= 2) { leading = 1; exp++; }
		}

		num_digits = limit;
	}

	// Calculate total width for padding (accounting for forced decimal point with # flag)
	exponent_absolute = (exp < 0) ? -exp : exp;
	total = 3 + sign + leading + (num_digits || JACL_FMT_HAS(FLAG, spec, pref) ? 1 + num_digits : 0) + (exponent_absolute >= 100 ? 4 : exponent_absolute >= 10 ? 3 : 2);
	padding = (width > total) ? width - total : 0;

	// Output with sign, prefix, and padding
	if (!stream && (!out || !*out)) return total + padding;
	if (!JACL_FMT_HAS(FLAG, spec, left) && !JACL_FMT_HAS(FLAG, spec, zpad)) __jacl_write_pad(stream, out, rem, len, ' ', padding);
	if (sign) __jacl_write_sign(stream, out, rem, len, spec, neg);

	__jacl_write_char(stream, out, rem, len, '0');
	__jacl_write_char(stream, out, rem, len, JACL_FMT_HAS(FLAG, spec, upper) ? 'X' : 'x');

	if (!JACL_FMT_HAS(FLAG, spec, left) && JACL_FMT_HAS(FLAG, spec, zpad)) __jacl_write_pad(stream, out, rem, len, '0', padding);

	// Output mantissa
	__jacl_write_char(stream, out, rem, len, '0' + leading);

	// Output decimal point and fractional digits (or just decimal point if # flag)
	if (num_digits || JACL_FMT_HAS(FLAG, spec, pref)) {
		digits_ptr = JACL_FMT_HAS(FLAG, spec, upper) ? UD : LD;

		__jacl_write_char(stream, out, rem, len, '.');

		for (i = 0; i < num_digits; i++) __jacl_write_char(stream, out, rem, len, digits_ptr[hex[i]]);
	}

	// Output exponent
	__jacl_write_char(stream, out, rem, len, JACL_FMT_HAS(FLAG, spec, upper) ? 'P' : 'p');
	__jacl_write_char(stream, out, rem, len, (exp < 0) ? '-' : '+');

	if (exponent_absolute >= 100) __jacl_write_char(stream, out, rem, len, '0' + exponent_absolute / 100);
	if (exponent_absolute >= 10) __jacl_write_char(stream, out, rem, len, '0' + (exponent_absolute / 10) % 10);

	__jacl_write_char(stream, out, rem, len, '0' + exponent_absolute % 10);

	if (JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', padding);

	return len;
}

// ============ PRINTF FUNCTIONS ============
static inline int __jacl_printf_ptr(FILE * stream, char **out, size_t *rem, __jacl_fmt_t spec, va_list ap, int prec, int width) {
	void *p = va_arg(ap, void*);
	union { void *p; uintmax_t u; } u;

	return (u.p = p)
	     ? __jacl_output_int(stream, out, rem, JACL_FMT_SET(BASE, spec, hex) | JACL_FMT_FLAG_pref, u.u, -1, width)
	     : __jacl_output_str(stream, out, rem, spec, "0x0", 3, width);
}
static inline int __jacl_printf_char(FILE * stream, char **out, size_t *rem, __jacl_fmt_t spec, va_list ap, int prec, int width) {
	int ch = va_arg(ap, int), pad = (width > 1) ? width - 1 : 0, len = 0;

	if (!JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);

	__jacl_write_char(stream, out, rem, len, (char)ch);

	if (JACL_FMT_HAS(FLAG, spec, left)) __jacl_write_pad(stream, out, rem, len, ' ', pad);

	return len;
}
static inline int __jacl_printf_str(FILE * stream, char **out, size_t *rem, __jacl_fmt_t spec, va_list ap, int prec, int width) {
	const char *str = va_arg(ap, const char*);

	if (!str) str = "(null)";

	return __jacl_output_str(stream, out, rem, spec, str, prec, width);
}
#define __jacl_printf_base(name) \
static inline int __jacl_printf_##name(FILE * stream, char **out, size_t *rem, __jacl_fmt_t spec, va_list ap, int prec, int width) { \
	uintmax_t val = JACL_FMT_HAS(FLAG, spec, sign) ? __jacl_get_signed(spec, ap) : __jacl_get_unsigned(spec, ap); \
	return __jacl_output_int(stream, out, rem, JACL_FMT_SET(BASE, spec, name), val, prec, width); \
}
#define __jacl_printf_style(name) \
static inline int __jacl_printf_##name(FILE * stream, char **out, size_t *rem, __jacl_fmt_t spec, va_list ap, int prec, int width) { \
	long double val = __jacl_get_float(spec, ap); \
	return __jacl_output_##name(stream, out, rem, spec, val, prec, width); \
}
__jacl_printf_base(dec)
__jacl_printf_base(oct)
__jacl_printf_base(hex)
__jacl_printf_style(float)
__jacl_printf_style(exp)
__jacl_printf_style(gen)
#if JACL_HAS_C99
__jacl_printf_style(alt)
#endif
#if JACL_HAS_C23
__jacl_printf_base(bin)
#endif

#define CASE(ch,type) case ch: len += __jacl_printf_##type(stream, &ptr, &rem, spec, ap, prec, width); break
static inline int __jacl_printf(FILE *stream, char * restrict out, size_t n, const char *fmt, va_list ap) {
	if (!fmt) return -1;

	char *ptr = out;
	size_t rem = (out && n > 0) ? n - 1 : 0;
	int len = 0, width, prec;
	char type;

	while (*fmt) {
		if (*fmt != '%') { __jacl_write_char(stream, &ptr, &rem, len, *fmt); fmt++; continue; }
		if (*(++fmt) == '%') { __jacl_write_char(stream, &ptr, &rem, len, '%'); fmt++; continue; }

		__jacl_fmt_t spec = __jacl_spec_printf(ap, &fmt, &prec, &width, &type);

		switch (type) {
		case 'n': __jacl_set_signed(spec, ap, (uintmax_t)len); break;
		CASE('c', char); CASE('s', str); CASE('d', dec); CASE('i', dec); CASE('u', dec); CASE('o', oct); CASE('x', hex); CASE('X', hex);
		#if JACL_HAS_C23
		CASE('b', bin); CASE('B', bin);
		#endif
		CASE('p', ptr); CASE('f', float); CASE('F', float); CASE('e', exp); CASE('E', exp); CASE('g', gen); CASE('G', gen);
		#if JACL_HAS_C99
		CASE('a', alt); CASE('A', alt);
		#endif
		default:
			__jacl_write_char(stream, &ptr, &rem, len, '%');
			__jacl_write_char(stream, &ptr, &rem, len, type);

			break;
		}

		fmt++;
	}
	if (out && n > 0) *ptr = '\0';

	return len;
}
#undef CASE

// ============ INPUT FUNCTIONS =============
static inline int __jacl_input_str(FILE *stream, const char **in, size_t *read, __jacl_fmt_t spec, int width, char *buf) {
	int i = 0, ch, pad = JACL_FMT_HAS(FLAG, spec, spad) ? 1 : 0;

	if (pad) __jacl_read_skip(ch, stream, in, read);

	while (i < width) {
		__jacl_read_next(ch, stream, in, read);

		if (ch == EOF || (pad && __jacl_isspace(ch))) break;
		if (buf) buf[i] = ch;

		i++;
	}
	if (buf && pad) buf[i] = '\0';

	return (pad && i == 0) ? 0 : 1;
}
static inline int __jacl_input_scan(FILE *stream, const char **in, size_t *read, const char **fmt, int width, char *buf) {
	char charset[256] = {0};
	int i = 0, invert = 0, ch, match;
	unsigned char start, end, c;

	// Skip opening '['
	(*fmt)++;

	// Check special chars
	if (**fmt == '^') { invert = 1; (*fmt)++; }
	if (**fmt == ']') { charset[(unsigned char)']'] = 1; (*fmt)++; }

	// Build charset
	while (**fmt && **fmt != ']') {
		// Check for range (a-z)
		if ((*fmt)[1] == '-' && (*fmt)[2] != ']' && (*fmt)[2] != '\0') {
			start = (unsigned char)**fmt;
			end = (unsigned char)(*fmt)[2];

			for (c = start; c <= end; c++) charset[c] = 1;

			(*fmt) += 3;
		} else {
			charset[(unsigned char)**fmt] = 1;
			(*fmt)++;
		}
	}

	// Skip leading whitespace
	__jacl_read_skip(ch, stream, in, read);

	// Read matching characters
	if (width == 0) width = INT_MAX;
	while (i < width) {
		__jacl_read_next(ch, stream, in, read);

		if (ch == EOF) break;

		match = charset[(unsigned char)ch];

		if (invert) match = !match;
		if (!match) { __jacl_read_back(ch, stream, in, read); break; }
		if (buf) buf[i] = (char)ch;

		i++;
	}
	if (buf) buf[i] = '\0';

	// Null-terminate and return success
	return (i > 0);
}
#define CASE(len, PRE, UPRE) case JACL_FMT_VAL(LENGTH, len): \
	if (JACL_FMT_HAS(FLAG, spec, sign) && (JACL_FMT_GET(BASE, spec) == JACL_FMT_BASE_dec || JACL_FMT_GET(BASE, spec) == JACL_FMT_BASE_int)) { \
		if (neg && val > (uintmax_t)-(PRE##_MIN + 1) + 1) { errno = ERANGE; val = (uintmax_t)PRE##_MIN; } \
		else if (!neg && val > (uintmax_t)(PRE##_MAX)) { errno = ERANGE; val = (uintmax_t)(PRE##_MAX); } \
		else if (neg) val = (uintmax_t)(-(intmax_t)val); \
	} else if (neg) { \
		val = (uintmax_t)(-(intmax_t)val); \
	} \
	break
static inline int __jacl_input_int(FILE *stream, const char **in, size_t *read, __jacl_fmt_t spec, int width, uintmax_t *rtn) {
	int neg = 0, pos_val = 0, *pos = &pos_val, digits = 0, ch, base = JACL_FMT_GET(BASE, spec);
	uintmax_t val = 0;

	__jacl_read_next(ch, stream, in, read);

	if (JACL_FMT_HAS(FLAG, spec, sign)) {
		if (ch == '-') { neg = 1; __jacl_read_next(ch, stream, in, pos); }
		else if (ch == '+') { __jacl_read_next(ch, stream, in, pos); }
	}

	if (base == 0) {
		if (ch == '0') {
			__jacl_read_next(ch, stream, in, pos);

			if (ch == 'x' || ch == 'X') { base = 16; __jacl_read_next(ch, stream, in, pos); }
			else base = 8;
		} else base = 10;
	} else if (base == 16 && ch == '0') {
		__jacl_read_next(ch, stream, in, pos);

		if (ch == 'x' || ch == 'X') { __jacl_read_next(ch, stream, in, pos); }
		else { __jacl_read_back(ch, stream, in, pos); ch = '0'; }
	}

	__jacl_read_digits(ch, stream, in, pos, width, digits, width, base, val, errno);

	if (digits > 0 && ch != EOF) __jacl_read_back(ch, stream, in, pos);
	switch (JACL_FMT_GET(LENGTH, spec)) {
	CASE(hh, SCHAR, UCHAR); CASE(h, SHRT, USHRT); CASE(l, LONG, ULONG); CASE(ll, LLONG, ULLONG); CASE(j, INTMAX, UINTMAX); CASE(t, PTRDIFF, SIZE); CASE(z, SSIZE, SIZE);
	default: CASE(0, INT, UINT);
	}
	if (base == 8 && val == 0) digits = 1;
	if (digits > 0) { *read += pos_val; *rtn = val; return 1; }

	return 0;
}
#undef CASE
static inline int __jacl_input_special(FILE *stream, const char **in, size_t *read, __jacl_fmt_t spec, int width, long double *rtn) {
	int i = 0, k = 0, neg = 0, sign = 0, ch;
	const char *pat = NULL;
	uintmax_t payload = 0;
	#if JACL_HAS_C99
		size_t size = (stream && stream->_ovrsiz) ? stream->_ovrsiz : OVRSIZ;
		char buf[size];
		memset(buf, 0, size);
	#else
		size_t size = OVRSIZ;
		char buf[OVRSIZ] = {0};
	#endif

	__jacl_read_next(ch, stream, in, read);

	// Fast fail
	if (ch == '-' || ch == '+') { sign = 1; if (ch == '-') neg = 1; __jacl_read_next(ch, stream, in, read); }
	if (ch == EOF || (!(pat = (ch == 'n' || ch == 'N' ? NANC : ch == 'i' || ch == 'I' ? INFC : NULL)))) {
		if (ch != EOF) __jacl_read_back(ch, stream, in, read);
		if (sign) __jacl_read_back(neg ? '-' : '+', stream, in, read);

		return 0;
	}

	do {
		buf[i++] = ch; __jacl_read_next(ch, stream, in, read);

		if (ch == EOF || (ch != pat[i * 2] && ch != pat[i * 2 + 1])) {
			if (ch != EOF) __jacl_read_back(ch, stream, in, read);

			break;
		}
	} while (i < 3);

	// Handle Infinity
	if (pat == INFC && i == 3) {
		while (i < 8 && ch != EOF && (ch == pat[i * 2] || ch == pat[i * 2 + 1])) { buf[i++] = ch; __jacl_read_next(ch, stream, in, read); }
		if (i < 8) while (i-- > 3) __jacl_read_back(buf[i], stream, in, read);

		*rtn = (neg)? -INFINITY : INFINITY;

		return 1;
	}

	// Handle NAN
	if (pat == NANC && i == 3) {
		if (ch == '(') {
			do {
				buf[k++] = ch; __jacl_read_next(ch, stream, in, read);

				if (ch == EOF || ch == ')') break;
			} while(k < size);

			if (ch == ')') buf[k] = '\0';
			else while(k--) __jacl_read_back(buf[k], stream, in, read);
			if (k > 1) {
				const char *p = &buf[1];
				size_t pl = 0;

				__jacl_input_int(NULL, &p, &pl, JACL_FMT_VAL(BASE, hex), size, &payload);
			}
		} else __jacl_read_back(ch, stream, in, read);

		*rtn = __jacl_payloadset_LDBL((uint64_t)payload, 0);

		if (neg) *rtn = -*rtn;

		return 1;
	}

	// Longer fail
	while (i-- > 0) __jacl_read_back(buf[i], stream, in, read);
	if (sign) __jacl_read_back(neg ? '-' : '+', stream, in, read);

	return 0;
}
#define CASE(N, PRE) case JACL_FMT_VAL(LENGTH,N): \
	limit = PRE##_MANT_DIG; \
	emax = (is_hex) ? PRE##_MAX_EXP : PRE##_MAX_10_EXP; \
	emin = (is_hex) ? PRE##_MIN_EXP : PRE##_MIN_10_EXP; \
	break;
static inline int __jacl_input_float(FILE *stream, const char **in, size_t *read, __jacl_fmt_t spec, int width, long double *rtn) {
	int ch, pos_val = 0, *pos = &pos_val, mdig = 0, edig = 0, mneg = 0, eneg = 0, err = 0, sign = 0, base = 10, is_hex = 0;
	intmax_t epart = 0, exp = 0, emax, emin, limit;
	uintmax_t digits = 0, ipart = 0, idig = 0, fpart = 0, trash = 0, slop = 0;

	__jacl_read_next(ch, stream, in, pos);

	// Parse prefix
	switch (JACL_FMT_GET(LENGTH, spec)) { CASE(0, FLT); CASE(l, DBL); CASE(L, LDBL); }
	if (ch == '+' || ch == '-') { sign = ch; mneg = (ch == '-'); __jacl_read_next(ch, stream, in, pos); }

	#if JACL_HAS_C99
		if (ch == '0') {
			__jacl_read_next(ch, stream, in, pos);

			if (ch == 'x' || ch == 'X') { is_hex = 1; base = 16; __jacl_read_next(ch, stream, in, pos); }
		}
	#endif

	// Parse integer
	if (width) __jacl_read_digits(ch, stream, in, pos, width, digits, limit, base, ipart, err);
	if (digits == limit) __jacl_read_digits(ch, stream, in, pos, width, exp, INT_MAX, base, trash, err);

	idig = digits;

	// Parse fractional
	if (ch == '.') {
		__jacl_read_next(ch, stream, in, pos);

		if (ipart == 0) { __jacl_read_digits(ch, stream, in, pos, width, exp, INT_MAX, 1, trash, err); exp = -exp; }
		if (digits < limit) __jacl_read_digits(ch, stream, in, pos, width, digits, limit, base, fpart, err);
		if (digits == limit) { __jacl_read_digits(ch, stream, in, pos, width, slop, INT_MAX, base, trash, err); }
	}

	// Parse exponent
	if (is_hex ? (ch == 'p' || ch == 'P') : (ch == 'e' || ch == 'E')) {
		__jacl_read_next(ch, stream, in, pos);

		if (ch == '+' || ch == '-') { eneg = (ch == '-'); __jacl_read_next(ch, stream, in, pos); }
		if (exp < emax  && exp > emin) __jacl_read_digits(ch, stream, in, pos, width, edig, INT_MAX, 10, epart, err);
		if (eneg) epart = -epart;
	}

	// Build result
	if (exp) epart += is_hex ? exp * 4 : exp;
	if (epart > emax) { errno = ERANGE; *rtn = INFINITY; }
	else if (epart < emin) { errno = ERANGE; *rtn = 0.0L; }
	else *rtn = (ipart + (long double)fpart / powl(base, digits - idig)) * powl(base == 16 ? 2.0L : 10.0L, epart);
	if (mneg) *rtn = __jacl_signset_LDBL(*rtn);
	if (ch != EOF) __jacl_read_back(ch, stream, in, pos);

	// Set outcome
	*read += pos_val;

	return 1;
}
#undef CASE

// ============ SCANF FUNCTIONS =============
static inline int __jacl_scanf_scan(FILE *stream, const char **in, size_t *read, __jacl_fmt_t spec, int width, va_list ap, const char **fmt) {
	char *p = (JACL_FMT_HAS(FLAG, spec, noout)) ? NULL : va_arg(ap, char*);

	if (width == 0) width = JACL_FMT_WIDTH_str;

	return __jacl_input_scan(stream, in, read, fmt, width, p);
}
#define CASE(N, type) case JACL_FMT_VAL(LENGTH,N): { type t = (type)val; *va_arg(ap, type*) = t; } break
static inline int __jacl_scanf_float(FILE *stream, const char **in, size_t *read, __jacl_fmt_t spec, int width, va_list ap) {
	long double val = 0.0L;
	int ch;
	__jacl_read_skip(ch, stream, in, read);
	if (width == 0) width = JACL_FMT_WIDTH_num;
	if (!__jacl_input_special(stream, in, read, spec, width, &val) && !__jacl_input_float(stream, in, read, spec, width, &val)) return 0;
	if (JACL_FMT_HAS(FLAG, spec, noout)) return 1;
	switch (JACL_FMT_GET(LENGTH, spec)) { CASE(0, float); CASE(l, double); CASE(L, long double); }
	return 1;
}
#undef CASE
#define __jacl_scanf_words(name) static inline int __jacl_scanf_##name(FILE *stream, const char **in, size_t *read, __jacl_fmt_t spec, int width, va_list ap) { \
	char *p = JACL_FMT_HAS(FLAG, spec, noout) ? NULL : va_arg(ap, char*); \
	if (width == 0) width = JACL_FMT_VAL(WIDTH,name); \
	return __jacl_input_str(stream, in, read, spec, width, p); \
}
#define __jacl_scanf_base(name) static inline int __jacl_scanf_##name(FILE *stream, const char **in, size_t *read, __jacl_fmt_t spec, int width, va_list ap) { \
	uintmax_t val = 0; \
	int ch; \
	__jacl_read_skip(ch, stream, in, read); \
	if (width == 0) width = JACL_FMT_WIDTH_num; \
	if (!__jacl_input_int(stream, in, read, JACL_FMT_SET(BASE, spec, name), width, &val)) return 0; \
	if (JACL_FMT_HAS(FLAG, spec, noout)) return 1; \
	if (JACL_FMT_GET(BASE, spec) == JACL_FMT_BASE_dec) __jacl_set_signed(spec, ap, val); \
	else __jacl_set_unsigned(spec, ap, val); \
	return 1; \
}
#define __jacl_scanf_base(name) static inline int __jacl_scanf_##name(FILE *stream, const char **in, size_t *read, __jacl_fmt_t spec, int width, va_list ap) { \
	uintmax_t val = 0; \
	int ch; \
	__jacl_read_skip(ch, stream, in, read); \
	if (width == 0) width = JACL_FMT_WIDTH_num; \
	if (!__jacl_input_int(stream, in, read, JACL_FMT_SET(BASE, spec, name), width, &val)) return 0; \
	if (JACL_FMT_HAS(FLAG, spec, noout)) return 1; \
	if (JACL_FMT_GET(BASE, spec) == JACL_FMT_BASE_dec || JACL_FMT_GET(BASE, spec) == JACL_FMT_BASE_int) { \
				if (JACL_FMT_HAS(FLAG, spec, sign)) __jacl_set_signed(spec, ap, val); \
				else __jacl_set_unsigned(spec, ap, val); \
			} else __jacl_set_unsigned(spec, ap, val); \
	return 1; \
}
__jacl_scanf_words(str)
__jacl_scanf_words(char)
__jacl_scanf_base(int)
__jacl_scanf_base(dec)
__jacl_scanf_base(oct)
__jacl_scanf_base(hex)
__jacl_scanf_base(ptr)
#if JACL_HAS_C23
__jacl_scanf_base(bin)
#endif

#define CASE(ch,type) case ch: \
	if (!__jacl_scanf_##type(stream, in, &read, spec, width, ap)) return (matched) ? count : EOF; \
	if (!JACL_FMT_HAS(FLAG, spec, noout)) count++; \
	matched = 1; \
	break
static inline int __jacl_scanf(FILE *stream, const char **in, const char * restrict fmt, va_list ap) {
	int ch, width = 0, count = 0, matched = 0;
	size_t read = 0;
	wchar_t type = 0;
	__jacl_fmt_t spec = 0;

	while (*fmt) {
		if (*fmt != '%') {
			if (__jacl_isspace(*fmt)) {
				__jacl_read_skip(ch, stream, in, &read);
			} else {
				__jacl_read_next(ch, stream, in, &read);

				if (ch != *fmt) {
					if (ch != EOF) __jacl_read_back(ch, stream, in, &read);

					return (ch == EOF && count == 0) ? EOF : count;
				}
			}

			fmt++;

			continue;
		} else spec = (fmt++, __jacl_spec_scanf(&fmt, &width, &type));

		switch (type) {
		case '%': __jacl_read_next(ch, stream, in, &read); if (ch != '%') return (ch == EOF && !matched) ? EOF : count; break;
		case 'n': if (!JACL_FMT_HAS(FLAG, spec, noout)) __jacl_set_signed(spec, ap, (uintmax_t)read); break;
		case '[': if (!__jacl_scanf_scan(stream, in, &read, spec, width, ap, &fmt)) return matched ? count : EOF; (!JACL_FMT_HAS(FLAG, spec, noout)) && count++; break;
		CASE('c', char); CASE('s', str); CASE('i', int); CASE('d', dec); CASE('u', dec); CASE('o',  oct); CASE('x', hex); CASE('X', hex);
		CASE('e', float); CASE('E', float); CASE('g', float); CASE('G', float); CASE('f', float); CASE('F', float); CASE('p', float);
		#if JACL_HAS_C23
		CASE('b', bin); CASE('B', bin);
		#endif
		#if JACL_HAS_C99
		CASE('a', float); CASE('A', float);
		#endif
		}

		fmt++;
	}

	return count;
}
#undef CASE

#ifdef __cplusplus
}
#endif

#endif /* CORE_FORMAT_H */
