/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef COMPLEX_H
#define COMPLEX_H
#pragma once

#include <config.h>
#include <math.h>

#if !JACL_HAS_C99
  #error "complex.h requires C99 or later"
#else

#ifdef __cplusplus
extern "C" {
#endif

#if JACL_HAS_C11
	#define CMPLX(x, y) ((double complex)((double)(x) + I * (double)(y)))
	#define CMPLXF(x, y) ((float complex)((float)(x) + I * (float)(y)))
	#define CMPLXL(x, y) ((long double complex)((long double)(x) + I * (long double)(y)))
#endif

#define __jacl_cmath(F) \
	__jacl_##F(float,				f) \
	__jacl_##F(double,			) \
	__jacl_##F(long double, l)

/* ================================================================ */
/* Manipulation functions                                           */
/* ================================================================ */

// creal, cimag, conj, cabs, carg, cproj
#define __jacl_creal(type, suf) static inline type creal##suf(type complex z) { union { type complex c; type a[2]; } u; u.c = z; return u.a[0]; }
#define __jacl_cimag(type, suf) static inline type cimag##suf(type complex z) { union { type complex c; type a[2]; } u; u.c = z; return u.a[1]; }
#define __jacl_conj(type, suf) static inline type complex conj##suf(type complex z) { return creal##suf(z) - I * cimag##suf(z); }
#define __jacl_cabs(type, suf) static inline type cabs##suf(type complex z) { type r = (type)creal##suf(z); type i = (type)cimag##suf(z); return hypot##suf(r, i); }
#define __jacl_carg(type, suf) static inline type carg##suf(type complex z) { type r = (type)creal##suf(z); type i = (type)cimag##suf(z); return atan2##suf(i, r); }
#define __jacl_cproj(type, suf) static inline type complex cproj##suf(type complex z) { \
	if (isfinite(creal##suf(z)) && isfinite(cimag##suf(z))) return z; \
	if (isnan(creal##suf(z)) || isnan(cimag##suf(z))) return z; \
	return (type complex)INFINITY + I * (type)0.0; \
}

/* ================================================================ */
/* Trigonometry functions                                           */
/* ================================================================ */

// ccos, csin, cacos, casin, catan, ctan
#define __jacl_ccos(type, suf) static inline type complex ccos##suf(type complex z) { type r = (type)creal##suf(z); type i = (type)cimag##suf(z); return cos##suf(r) * cosh##suf(i) - I * (sin##suf(r) * sinh##suf(i)); }
#define __jacl_csin(type, suf) static inline type complex csin##suf(type complex z) { type r = (type)creal##suf(z); type i = (type)cimag##suf(z); return sin##suf(r) * cosh##suf(i) + I * (cos##suf(r) * sinh##suf(i)); }
#define __jacl_cacos(type, suf) static inline type complex cacos##suf(type complex z) { type complex temp = csqrt##suf((type complex)1 - z * z); return -I * clog##suf(z + I * temp); }
#define __jacl_casin(type, suf) static inline type complex casin##suf(type complex z) { type complex temp = csqrt##suf((type complex)1 - z * z); return -I * clog##suf(I * z + temp); }
#define __jacl_catan(type, suf) static inline type complex catan##suf(type complex z) { return (I / (type)2) * clog##suf((I + z) / (I - z)); }
#define __jacl_ctan(type, suf) static inline type complex ctan##suf(type complex z) { \
	type complex s = csin##suf(z); \
	type complex c = ccos##suf(z); \
	type r = creal##suf(c), i = cimag##suf(c); \
	if (r == (type)0 && i == (type)0) return (type complex)(0.0 / 0.0); \
	return s / c; \
}

/* ================================================================ */
/* Hyperbolic functions                                             */
/* ================================================================ */

// ccosh, csinh, cacosh, casinh, catanh, ctanh
#define __jacl_ccosh(type, suf) static inline type complex ccosh##suf(type complex z) { type r = (type)creal##suf(z); type i = (type)cimag##suf(z); return cosh##suf(r) * cos##suf(i) + I * (sinh##suf(r) * sin##suf(i)); }
#define __jacl_csinh(type, suf) static inline type complex csinh##suf(type complex z) { type r = (type)creal##suf(z); type i = (type)cimag##suf(z); return sinh##suf(r) * cos##suf(i) + I * (cosh##suf(r) * sin##suf(i)); }
#define __jacl_cacosh(type, suf) static inline type complex cacosh##suf(type complex z) { return clog##suf(z + csqrt##suf(z + (type complex)1) * csqrt##suf(z - (type complex)1)); }
#define __jacl_casinh(type, suf) static inline type complex casinh##suf(type complex z) { return clog##suf(z + csqrt##suf(z * z + (type complex)1)); }
#define __jacl_catanh(type, suf) static inline type complex catanh##suf(type complex z) { return (type complex)0.5 * clog##suf(((type complex)1 + z) / ((type complex)1 - z)); }
#define __jacl_ctanh(type, suf) static inline type complex ctanh##suf(type complex z) { \
	type complex s = csinh##suf(z); type complex c = ccosh##suf(z); type r = creal##suf(c), i = cimag##suf(c); \
	if (r == (type)0 && i == (type)0) return (type complex)(0.0 / 0.0); \
	return s / c; \
}

/* ================================================================ */
/* Exponential functions                                            */
/* ================================================================ */

// cpow, csqrt, cexp, clog
#define __jacl_cpow(type, suf) static inline type complex cpow##suf(type complex z, type complex w) { return cexp##suf(w * clog##suf(z)); }
#define __jacl_csqrt(type, suf) static inline type complex csqrt##suf(type complex z) { return cexp##suf((type complex)0.5 * clog##suf(z)); }
#define __jacl_cexp(type, suf) static inline type complex cexp##suf(type complex z) { \
	type r = (type)creal##suf(z); \
	type i = (type)cimag##suf(z); \
	type e = exp##suf(r); \
	return e * cos##suf(i) + I * (e * sin##suf(i)); \
}
#define __jacl_clog(type,suf) static inline type complex clog##suf(type complex z) { \
	type r = creal##suf(z), i = cimag##suf(z); \
	if (r == (type)0 && i == (type)0) return (type)(-INFINITY) + I * carg##suf(z); \
	return log##suf(cabs##suf(z)) + I * carg##suf(z); \
}

__jacl_cmath(creal)
__jacl_cmath(cimag)
__jacl_cmath(conj)
__jacl_cmath(cabs)
__jacl_cmath(carg)
__jacl_cmath(cproj)

__jacl_cmath(cexp)
__jacl_cmath(clog)
__jacl_cmath(csqrt)
__jacl_cmath(cpow)

__jacl_cmath(ccos)
__jacl_cmath(csin)
__jacl_cmath(ctan)
__jacl_cmath(cacos)
__jacl_cmath(casin)
__jacl_cmath(catan)

__jacl_cmath(ccosh)
__jacl_cmath(csinh)
__jacl_cmath(ctanh)
__jacl_cmath(cacosh)
__jacl_cmath(casinh)
__jacl_cmath(catanh)

#ifdef __cplusplus
}
#endif

#endif /* JACL_HAS_C99 */
#endif /* COMPLEX_H */
