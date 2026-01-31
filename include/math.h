/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef MATH_H
#define MATH_H
#pragma once

#include <config.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <float.h>
#include <fenv.h>

#ifdef __cplusplus
extern "C" {
#endif

#if JACL_HAS_FLOAT
static_assert(sizeof(double) == 8, "Assumes 64-bit IEEE-754 double");
static_assert(FLT_MANT_DIG == 24, "Assumes IEEE-754 float mantissa");
static_assert(DBL_MANT_DIG == 53, "Assumes IEEE-754 double mantissa");
#endif

#define JACL_RND_EVEN 0
#define JACL_RND_DOWN 1
#define JACL_RND_HIGH 2
#define JACL_RND_ZERO 3
#define JACL_RND_BANK 4

#if JACL_HAS_C99

// Special Values
#define HUGE_VAL   ((double)(1.0/0.0))
#define HUGE_VALF  ((float)(1.0f/0.0f))
#define HUGE_VALL  ((long double)(1.0L/0.0L))
#define NAN        ((double)(0.0/0.0))
#define INFINITY ((float)(1.0f/0.0f))

// Constants
#define FP_INFINITE	   1
#define FP_NAN         2
#define FP_NORMAL      3
#define FP_SUBNORMAL   4
#define FP_ZERO        5
#define FP_ILOGB0      INT_MIN
#define FP_ILOGBNAN    INT_MIN

#endif /* JACL_HAS_C99 */

#if JACL_HAS_C23

#define FP_INT_UPWARD            JACL_RND_HIGH
#define FP_INT_DOWNWARD          JACL_RND_DOWN
#define FP_INT_TOWARDZERO        JACL_RND_ZERO
#define FP_INT_TONEARESTFROMZERO JACL_RND_BANK
#define FP_INT_TONEAREST         JACL_RND_EVEN

#endif /* JACL_HAS_C23 */

#define MATH_ERRNO		 1
#define MATH_ERREXCEPT 2
#define math_errhandling (MATH_ERRNO | MATH_ERREXCEPT)

#define M_E					2.71828182845904523536
#define M_LOG2E			1.44269504088896340736
#define M_LOG10E		0.43429448190325182765
#define M_LN2       0.69314718055994530942
#define M_LN2L 0.6931471805599453094172321214581765680755L
#define M_LN10			2.30258509299404568402
#define M_PI				3.14159265358979323846
#define M_PI_2			1.57079632679489661923
#define M_PI_4			0.78539816339744830962
#define M_1_PI			0.31830988618379067154
#define M_2_PI			0.63661977236758134308
#define M_2_SQRTPI	1.12837916709551257390
#define M_SQRT2			1.41421356237309504880
#define M_SQRT1_2		0.70710678118654752440

typedef union { double f; uint64_t u; } __jacl_f64u;

#ifdef FLT_EVAL_METHOD
#if FLT_EVAL_METHOD == 0
typedef float float_t;
typedef double double_t;
#elif FLT_EVAL_METHOD == 1
typedef double float_t;
typedef double double_t;
#elif FLT_EVAL_METHOD == 2
typedef long double float_t;
typedef long double double_t;
#endif
#else
typedef float float_t;
typedef double double_t;
#endif

// ======== CLASSIFICATION VIA MACROS =======
#define isnan(x)              ((x) != (x))
#define isinf(x)              (!isnan(x) && isnan((x)-(x)))
#define isfinite(x)	          (!isnan(x) && !isinf(x))
#define isunordered(x,y)      (isnan(x) || isnan(y))
#define isnormal(x)           (sizeof(x) == sizeof(float) ? __isnormalf(x) : sizeof(x) == sizeof(double) ? __isnormal(x) : __isnormall(x) )
#define isgreater(x, y)	      (!isunordered(x, y) && (x) >  (y))
#define isgreaterequal(x, y)  (!isunordered(x, y) && (x) >= (y))
#define isless(x, y)          (!isunordered(x, y) && (x) <  (y))
#define islessequal(x, y)     (!isunordered(x, y) && (x) <= (y))
#define islessgreater(x, y)   (!isunordered(x, y) && ((x) < (y) || (x) > (y)))
#define fpclassify(x)         ( isnan(x) ? FP_NAN : isinf(x) ? FP_INFINITE : (x) == 0.0 ? FP_ZERO : !isnormal(x) ? FP_SUBNORMAL : FP_NORMAL )
#define signbit(x)            ( sizeof(x) == sizeof(double) ? __signbit(x) : sizeof(x) == sizeof(float) ? __signbitf(x) : __signbitl(x) )
#define nan(s)                __nan(s)

#if JACL_HAS_C99
#define nanf(s)               __nanf(s)
#define nanl(s)               __nanl(s)
#endif

// ======== HELPERS MACRO GENERATOR =========
#if JACL_HAS_C99

#define __jacl_math(F) \
__jacl_##F(double,			 , DBL) \
__jacl_##F(float,				f, FLT) \
__jacl_##F(long double, l, LDBL)

#else

#define __jacl_math(F) \
__jacl_##F(double,			, DBL)

#endif /* JACL_HAS_C99 */

// ====== CHEBYSHEV POLYNOMIAL HELPERS ======
#define __jacl_cheb_even(type, c0, u4, c2, c4, c6) (((type)(c0)) + (u4) * (((type)(c2)) + (u4) * (((type)(c4)) + (u4) * ((type)(c6)))))
#define __jacl_cheb_odd(type, u2, u4, c1, c3, c5) ((u2) * (((type)(c1)) + (u4) * (((type)(c3)) + (u4) * ((type)(c5)))))

unsigned long strtoul(const char *str, char **endptr, int base);

// Bit Manipulation: isnormal, nan, fabs, copysign, signbit
#define __jacl_fabs(type,suf,PRE)             static inline type fabs##suf(type x){ return __jacl_signclr_##PRE(x); }
#define __jacl_copysign(type,suf,PRE)         static inline type copysign##suf(type x, type y){ return __jacl_signcpy_##PRE(x, y); }
#define __jacl_signbit(type,suf,PRE)          static inline int __signbit##suf(type x) { return __jacl_signget_##PRE(x); }
#define __jacl_isnormal(type,suf,PRE)         static inline int __isnormal##suf(type x) { type ax = __jacl_signclr_##PRE(x); return ax >= PRE##_MIN && ax <= PRE##_MAX; }
#define __jacl_nan(type,suf,PRE)              static inline type __nan##suf(const char *s){ \
if (!s || !*s) return __jacl_payloadset_##PRE(0, 0); \
char *p; \
PRE##_UTYPE payload = (PRE##_UTYPE)strtoul(s, &p, 0); \
if (p == s) return __jacl_payloadset_##PRE(0, 0); \
return __jacl_payloadset_##PRE(payload, 0); \
}

// Rounding: trunc, ceil, floor, round, rint, nearbyint, roundeven
#define __jacl_rmodes(type,suf,PRE)           static inline type __jacl_rmode##suf(type x, int mode, unsigned int width, int is_unsigned, int notify) { \
if (!isfinite(x)) { if (width > 0) { errno = EDOM; return 0; } return x; } \
if (width > 0 && (width > 64 || (is_unsigned && x < 0))) { errno = EDOM; return 0; } \
type t, diff, r; \
if (x == 0 || !(x == x)) { t = x; } \
else { \
	long long i = (long long)x; \
	t = (type)i; \
	if (t == (type)0) t = x * (type)0.0; \
} \
diff = x - t; \
switch(mode) { \
	case JACL_RND_HIGH: r = (t < x) ? t + (type)1 : t; break; \
	case JACL_RND_DOWN: r = (t > x) ? t - (type)1 : t; break; \
	case JACL_RND_ZERO: r = t; break; \
	case JACL_RND_BANK: \
		if (diff >= (type)0.5) r = t + (type)1; \
		else if (diff <= (type)-0.5) r = t - (type)1; \
		else r = t; break; \
	case JACL_RND_EVEN: \
		if (fabs##suf(diff) > (type)0.5) r = t + (diff > 0 ? 1 : -1); \
		else if (fabs##suf(diff) == (type)0.5 && ((long long)t & 1)) r = t + (diff > 0 ? 1 : -1); \
		else r = t; break; \
	default: \
		if (width > 0) errno = EDOM; \
		return x; \
} \
if (width > 0) { \
	if (is_unsigned) { \
		uintmax_t umax = (width == 64) ? UINTMAX_MAX : ((1ULL << width) - 1); \
		if (r > (type)umax) { errno = EDOM; return 0; } \
	} else { \
		intmax_t smin = -(1LL << (width - 1)), smax = (1LL << (width - 1)) - 1; \
		if (r < (type)smin || r > (type)smax) { errno = EDOM; return 0; } \
	} \
} \
if (notify && r != x) feraiseexcept(FE_INEXACT); \
return r; \
}
#define __jacl_trunc(type,suf,PRE)            static inline type trunc##suf(type x) { return __jacl_rmode##suf(x, JACL_RND_ZERO, 0, 0, 0); }
#define __jacl_ceil(type,suf,PRE)             static inline type ceil##suf(type x) { return __jacl_rmode##suf(x, JACL_RND_HIGH, 0, 0, 0); }
#define __jacl_floor(type,suf,PRE)            static inline type floor##suf(type x) { return __jacl_rmode##suf(x, JACL_RND_DOWN, 0, 0, 0); }
#define __jacl_round(type,suf,PRE)            static inline type round##suf(type x) { return __jacl_rmode##suf(x, JACL_RND_BANK, 0, 0, 0); }
#define __jacl_rint(type,suf,PRE)             static inline type rint##suf(type x) { return __jacl_rmode##suf(x, fegetround(), 0, 0, 1); }
#define __jacl_nearbyint(type,suf,PRE)        static inline type nearbyint##suf(type x) { return __jacl_rmode##suf(x, fegetround(), 0, 0, 0); }
#define __jacl_roundeven(type,suf,PRE)        static inline type roundeven##suf(type x) { return __jacl_rmode##suf(x, JACL_RND_EVEN, 0, 0, 0); }

// Integer Rounding: fromfp, ufromfp, fromfpx, ufromfpx
#define __jacl_fromfp(type,suf,PRE)           static inline intmax_t fromfp##suf(type x, int round, unsigned int width) { return (intmax_t)__jacl_rmode##suf(x, round, width, 0, 0); }
#define __jacl_ufromfp(type,suf,PRE)          static inline uintmax_t ufromfp##suf(type x, int round, unsigned int width) { return (uintmax_t)__jacl_rmode##suf(x, round, width, 1, 0); }
#define __jacl_fromfpx(type,suf,PRE)          static inline intmax_t fromfpx##suf(type x, int round, unsigned int width) { return (intmax_t)__jacl_rmode##suf(x, round, width, 0, 1); }
#define __jacl_ufromfpx(type,suf,PRE)         static inline uintmax_t ufromfpx##suf(type x, int round, unsigned int width) { return (uintmax_t)__jacl_rmode##suf(x, round, width, 1, 1); }

// Long Rounding: lround, llround, lrint, llrint
#define __jacl_lround(type,suf,PRE)           static inline long lround##suf(type x){ return (long)round##suf(x); }
#define __jacl_llround(type,suf,PRE)          static inline long long llround##suf(type x){ return (long long)round##suf(x); }
#define __jacl_lrint(type,suf,PRE)            static inline long lrint##suf(type x){ return (long)rint##suf(x); }
#define __jacl_llrint(type,suf,PRE)           static inline long long llrint##suf(type x){ return (long long)rint##suf(x); }

// Relation: fdim, fmax, fmin, fma
#define __jacl_fdim(type,suf,PRE)             static inline type fdim##suf(type x, type y){ \
JACL_SAFETY(isnan(x) || isnan(y), (type)NAN); \
return x > y ? x - y : (type)0; \
}
#define __jacl_fmax(type,suf,PRE)             static inline type fmax##suf(type x, type y){ return (x < y || isnan(x)) ? y : x; }
#define __jacl_fmin(type,suf,PRE)             static inline type fmin##suf(type x, type y){ return (x > y || isnan(x)) ? y : x; }
#define __jacl_fma(type,suf,PRE)              static inline type fma##suf(type x, type y, type z){ \
int ex, ey, ez; \
frexp##suf(x, &ex); frexp##suf(y, &ey); frexp##suf(z, &ez); \
if(ex > PRE##_MAX_EXP - 2 || ey > PRE##_MAX_EXP - 2) return ((x * (type)0.5) * y + (z * (type)0.5)) * (type)2.0; \
if(ez > ex + ey) return ((x * (type)2.0) * y + (z * (type)2.0)) * (type)0.5; \
return x * y + z; \
}

// Exponents: exp, exp2, expm1, log, log2, log10, log1p, pow
#define __jacl_exp(type,suf,PRE) static inline type exp##suf(type x) { \
	JACL_SAFETY(isnan(x), x); \
	JACL_SAFETY(isinf(x), x > 0? x : (type)0); \
	JACL_SAFETY(x == 0, (type)1); \
	JACL_SAFETY(x > PRE##_LOG_MAX - 2.0L, (type)INFINITY); \
	JACL_SAFETY(x < PRE##_LOG_TRUE_MIN, (type)0.0); \
	type k = x * M_LOG2E + (x >= 0 ? .5 : -.5); \
	int ki = (int)k; \
	type r = x - ki * M_LN2; \
	type r2 = r * r; \
	type r3 = r2 * r; \
	type result = 1.0 + r + r2 / 2.0 + r3 * (1.0 / 6.0 + r * (1.0 / 24.0 + r * (1.0 / 120.0 + r * (1.0 / 720.0 + r / 5040.0)))); \
	return __jacl_expscal_##PRE(result, ki); \
}
#define __jacl_exp2(type,suf,PRE) static inline type exp2##suf(type x) { \
	if (x == (type)(PRE##_MIN_EXP - PRE##_MANT_DIG)) return PRE##_TRUE_MIN; \
	if (x < (type)(PRE##_MIN_EXP - PRE##_MANT_DIG)) return (type)0.0; \
	if (x >= (type)(PRE##_MIN_EXP - PRE##_MANT_DIG) && x <= (type)PRE##_MAX_EXP && (type)(int)x == x) return ldexp##suf((type)1.0, (int)x); \
	JACL_SAFETY(x > (type)(PRE##_MAX_EXP - 1), (type)INFINITY); \
	return exp##suf(x * M_LN2); \
}
#define __jacl_exp10(type,suf,PRE)            static inline type exp10##suf(type x) { \
	if (x < (type)PRE##_MIN_10_EXP) return 0.0; \
	int i = (int)x; \
	if (x == (type)i) { \
		if (i >= 0 && i <= 15) return (type)POW10[i]; \
		if (i < 0 && i >= -15) return (type)1.0 / (type)POW10[-i]; \
	} \
	return exp##suf(x * M_LN10); \
}
#define __jacl_expm1(type,suf,PRE)            static inline type expm1##suf(type x){ return exp##suf(x)-1; }
#define __jacl_log(type, suf, PRE)            static inline type log##suf(type x) { \
	int e; type m, t, s; \
	JACL_SAFETY(x == 1, (type)0); \
	JACL_SAFETY(x < 0, (type)NAN); \
	JACL_SAFETY(x == 0, -((type)INFINITY)); \
	JACL_SAFETY(!isfinite(x), x); \
	m = frexp##suf(x, &e); \
	if (m < (type)M_SQRT1_2) { m *= (type)2.0; e--; } \
	m = m - (type)1.0; \
	s = m; t = m; \
	int i = 2, n = PRE##_MANT_DIG / 2; \
	for (; i <= n; i++) { t *= -m; s += t / i; } \
	return s + e * M_LN2L; \
}
#define __jacl_log2(type,suf,PRE)             static inline type log2##suf(type x){ return log##suf(x)/M_LN2; }
#define __jacl_log10(type,suf,PRE)            static inline type log10##suf(type x){ return log##suf(x)/M_LN10; }
#define __jacl_log1p(type,suf,PRE)            static inline type log1p##suf(type x){ return log##suf(1+x); }
#if JACL_HAS_C99
#define __jacl_pow(type,suf,PRE) static inline type pow##suf(type x, type y) { \
	JACL_SAFETY(isnan(x) || isnan(y), (type)NAN); \
	if (y == 0 || x == 1) return (type)1; \
	if (y == 1) return x; \
	if (y == 2) return x * x; \
	if (y == 3) return x * x * x; \
	if (y == -1) return (type)1 / x; \
	if (y == 0.5) return sqrt##suf(x); \
	if (x == 10.0 && y < (type)PRE##_MIN_10_EXP) return 0.0; \
	if (x < 0 && y != (type)(int)y) return (type)NAN; \
	type ay = fabs##suf(y); \
	int i = (int)ay; \
	if (ay == (type)i && i <= 64) { \
		type result = 1; \
		while (i--) result *= x; \
		return y < 0 ? 1 / result : result; \
	} \
	if (x == 2) return exp2##suf(y); \
	if (x == 10) return exp10##suf(y); \
	type t = y * log##suf(x); \
	JACL_SAFETY(t > PRE##_LOG_MAX, (type)INFINITY); \
	JACL_SAFETY(t < PRE##_LOG_TRUE_MIN, (type)0.0); \
	return exp##suf(t); \
}
#else
#define __jacl_pow(type,suf,PRE) static inline type pow##suf(type x, type y) { \
	JACL_SAFETY(isnan(x) || isnan(y), (type)NAN); \
	if (y == 0 || x == 1) return (type)1; \
	if (y == 1) return x; \
	if (y == 2) return x * x; \
	if (y == 3) return x * x * x; \
	if (y == -1) return (type)1 / x; \
	if (y == 0.5) return sqrt##suf(x); \
	if (x < 0 && y != (type)(int)y) return (type)NAN; \
	type ay = fabs##suf(y); \
	int i = (int)ay; \
	if (ay == (type)i && i <= 64) { \
		type result = 1; \
		while (i--) result *= x; \
		return y < 0 ? 1 / result : result; \
	} \
	type t = y * log##suf(x); \
	JACL_SAFETY(t > PRE##_LOG_MAX, (type)INFINITY); \
	JACL_SAFETY(t < PRE##_LOG_TRUE_MIN, (type)0.0); \
	return exp##suf(t); \
}
#endif /* JACL_HAS_C99 */

// Scaling: ldexp, scalbn, scalbln, logb, ilogb, llogb, frexp
#define __jacl_ldexp(type,suf,PRE)            static inline type ldexp##suf(type x, int n) { return __jacl_expscal_##PRE(x, n); }
#define __jacl_scalbn(type,suf,PRE)           static inline type scalbn##suf(type x, int n) { return ldexp##suf(x, n); }
#define __jacl_scalbln(type,suf,PRE)          static inline type scalbln##suf(type x, long n) { return ldexp##suf(x, (int)n); }
#define __jacl_logb(type,suf,PRE)             static inline type logb##suf(type x) { \
	if (isnan(x)) return (type)NAN; \
	if (x == (type)0) return (type)(-INFINITY); \
	if (isinf(x)) return (type)INFINITY; \
	int e; frexp##suf(x, &e); \
	return (type)(e - 1); \
}
#define __jacl_ilogb(type,suf,PRE)            static inline int ilogb##suf(type x) { \
	if (!(x == x)) return FP_ILOGBNAN; \
	if (x == (type)0) return FP_ILOGB0; \
	return (int)logb##suf(x); \
}
#define __jacl_llogb(type,suf,PRE)            static inline long llogb##suf(type x) { \
	if (!(x == x)) return FP_ILOGBNAN; \
	if (x == (type)0) return FP_ILOGB0; \
	return (long)logb##suf(x); \
}
#define __jacl_frexp(type,suf,PRE) static inline type frexp##suf(type x, int *e) { \
	if (x == 0 || !isfinite(x)) { *e = 0; return x; } \
	type ax = x < 0 ? -x : x; \
	if (ax < PRE##_MIN) { \
		x = __jacl_expscal_##PRE(x, PRE##_MANT_DIG); \
		int exp_normal = __jacl_expfind_##PRE(x) + 1; \
		*e = exp_normal - PRE##_MANT_DIG; \
		return __jacl_expscal_##PRE(x, -exp_normal); \
	} \
	*e = __jacl_expfind_##PRE(x) + 1; \
	return __jacl_expscal_##PRE(x, -(*e)); \
}

// Roots: sqrt, cbrt, hypot
#define __jacl_sqrt(type,suf,PRE)             static inline type sqrt##suf(type x){ \
	JACL_SAFETY(isnan(x), x); \
	JACL_SAFETY(isinf(x) && x > 0, x); \
	JACL_SAFETY(x < 0, (type)NAN); \
	if (x == 0 || x == 1) return x; \
	int e = __jacl_expfind_##PRE(x); \
	type g = __jacl_expscal_##PRE((type)1.0, e/2); \
	for(int i=0;i<4;++i)g=(g+x/g)/2; \
	return g; \
}
#define __jacl_cbrt(type,suf,PRE)             static inline type cbrt##suf(type x){ \
	JACL_SAFETY(isnan(x), x); \
	JACL_SAFETY(isinf(x), x); \
	if (x == 0 || x == 1) return x; \
	if (x < 0) return -exp##suf(log##suf(-x) / 3.0); \
	return exp##suf(log##suf(x) / 3.0); \
}
#define __jacl_hypot(type,suf,PRE)            static inline type hypot##suf(type x,type y){ \
	JACL_SAFETY(isnan(x) || isnan(y), (type)NAN); \
	JACL_SAFETY(isinf(x) || isinf(y), ((type)INFINITY)); \
	type ax = x < 0 ? -x : x, ay = y < 0 ? -y : y, r; \
	if (ax < ay) { type t = ax; ax = ay; ay = t; } \
	if (ax == 0) return 0; \
	if (ax == ay) return ax * M_SQRT2; \
	r = ay / ax; \
	return ax * sqrt##suf((type)1 + r * r); \
}

// Triganometry: sin, cos, tan, asin, acos, atan, atan2
#define __jacl_sin(type,suf,PRE)              static inline type sin##suf(type x) { \
	JACL_SAFETY(!isfinite(x), (type)NAN);       \
	if (x == 0) return x; \
	type ax = x < 0 ? -x : x; \
	if (ax > M_PI) x = fmod##suf(x, (type)(2.0 * M_PI)); \
	type t = x, s = x; int i; \
	for(i = 3; i < 20; i += 2){ t *= -x * x / (i * (i - 1)); s += t; } \
	return s; \
}
#define __jacl_cos(type,suf,PRE)              static inline type cos##suf(type x) { \
	JACL_SAFETY(!isfinite(x), (type)NAN);       \
	if (x == 0) return (type)1; \
	type ax = x < 0 ? -x : x; \
	if (ax > M_PI) x = fmod##suf(x, (type)(2.0 * M_PI)); \
	type t = 1.0, s = 1.0, x2 = x * x; int i; \
	for(i = 2; i < 20; i += 2){ t *= -x2 / ((type) i * (i - 1)); s += t; } \
	return s; \
}
#define __jacl_tan(type,suf,PRE)              static inline type tan##suf(type x){return sin##suf(x)/cos##suf(x);}
#define __jacl_asin(type,suf,PRE)             static inline type asin##suf(type x){ \
	int i; \
	JACL_SAFETY(fabs##suf(x) > 1.0, (type)NAN); \
	if(fabs##suf(x) > 0.7) { \
		type sign = (x < 0) ? -1.0 : 1.0, ax = sign * x, z = (1.0 - ax) / 2.0, y = sqrt##suf(z), s=y, t=y, y2=y*y; \
		for(i=1;i<15;i++){ t *= y2 * (2 * i - 1) / (type)(2 * i); s += t / (2 * i + 1); } \
		return sign * (M_PI_2 - 2.0 * s); \
	} \
	type s = x, t = x, x2 = x * x; \
	for(i = 1; i < 15; i++){ t *= x2 * (2 * i - 1) / (type)(2 * i); s += t / (2 * i + 1); } \
	return s; \
}
#define __jacl_acos(type,suf,PRE)              static inline type acos##suf(type x){ return M_PI_2 - asin##suf(x);}
#define __jacl_atan(type,suf,PRE)              static inline type atan##suf(type x) { \
	JACL_SAFETY(isinf(x), x > 0 ? (type)(M_PI_2) : (type)(-M_PI_2)); \
	if (x == 0) return 0; \
	static const type atanhi[] = { \
		4.63647609000806093515e-01, \
		7.85398163397448278999e-01, \
		9.82793723247329054082e-01, \
		1.57079632679489655800e+00  \
	}; \
	static const type coeffs[] = { \
		 3.33333333333329318027e-01, \
		-1.99999999998764832476e-01, \
		 1.42857142725034663711e-01, \
		-1.11111104054623557880e-01, \
		 9.09088713343650656196e-02, \
		-7.69187620504482999495e-02, \
		 6.66107313738753120669e-02 \
	}; \
	type ax = x < 0 ? -x : x; \
	int id = -1; \
	if (ax >= 2.4375) id = 3, ax = ax > 1e308 ? 0.0 : -1.0 / ax; \
	else if (ax >= 1.1875) id = 2, ax = (ax - 1.5) / (1.0 + 1.5 * ax); \
	else if (ax >= 0.6875) id = 1, ax = (ax - 1.0) / (ax + 1.0); \
	else if (ax >= 0.4375) id = 0, ax = (2.0 * ax - 1.0) / (2.0 + ax); \
	type z = ax * ax, w = z * z; \
	type s = z * (coeffs[0] + w * (coeffs[2] + w * (coeffs[4] + w * coeffs[6]))) + w * (coeffs[1] + w * (coeffs[3] + w * coeffs[5])); \
	type result = (id < 0) ? ax - ax * s : atanhi[id] - (ax * s - ax); \
	return x < 0 ? -result : result; \
}
#define __jacl_atan2(type,suf,PRE)             static inline type atan2##suf(type y,type x){ \
	if(x == 0) return (y == 0) ? (type)0 : (y > 0) ? M_PI_2 : -M_PI_2; \
	type a = atan##suf(y / x); \
	if(x < 0) a += __jacl_signget_##PRE(y) ? -M_PI : M_PI; \
	return a; \
}

// Hyperbolic: sinh, cosh, tanh, asinh, acosh, atanh
#define __jacl_sinh(type,suf,PRE)              static inline type sinh##suf(type x){ return (exp##suf(x)-exp##suf(-x)) / 2; }
#define __jacl_cosh(type,suf,PRE)              static inline type cosh##suf(type x){ return(exp##suf(x)+exp##suf(-x)) / 2; }
#define __jacl_tanh(type,suf,PRE)              static inline type tanh##suf(type x){ return sinh##suf(x) / cosh##suf(x); }
#define __jacl_asinh(type,suf,PRE)             static inline type asinh##suf(type x){ \
	JACL_SAFETY(isinf(x), x); \
	return log##suf(x+sqrt##suf(x * x + 1.0)); \
}
#define __jacl_acosh(type,suf,PRE)             static inline type acosh##suf(type x){ \
	JACL_SAFETY(isinf(x) && x > 0, x); \
	if (x < 1.0) return (type)NAN; \
	return log##suf(x + sqrt##suf(x * x - 1.0)); \
}
#define __jacl_atanh(type,suf,PRE)             static inline type atanh##suf(type x){ \
	type ax = x < 0 ? -x : x; \
	return (ax < 1.0) ? 0.5 * log##suf((1.0 + x) / (1.0 - x)) : (ax == 1.0) ? (x > 0 ? (type)INFINITY : -(type)INFINITY) : (type)NAN; \
}
#define __jacl_atanh(type,suf,PRE)             static inline type atanh##suf(type x){ \
	type ax = x < 0 ? -x : x; \
	return (ax < 1.0) \
		? 0.5 * log##suf((1.0 + x) / (1.0 - x)) \
		: (ax == 1.0) \
		? (x > 0) ? (type)INFINITY \
		: -(type)INFINITY \
		: (type)NAN; \
}

// Error and Gamma: erf, erfc, tgamma, lgamma
#define __jacl_erf(type,suf,PRE)               static inline type erf##suf(type x){ \
	type t = x, s = x; \
	int i; \
	for(i = 1; i < 15; ++i){ t *= -x * x / (type)i; s += t / (type)(2 * i + 1); } \
	return (2.0 / sqrt##suf(M_PI)) * s; \
}
#define __jacl_erfc(type,suf,PRE)              static inline type erfc##suf(type x){ return 1.0 - erf##suf(x); }
#define __jacl_tgamma(type,suf,PRE)            static inline type tgamma##suf(type x){ \
	JACL_SAFETY(x < 0, (type)NAN); \
	if(x == 1.0 || x == 2.0) return 1.0; \
	return sqrt##suf(2.0 * M_PI / x) * pow##suf(x / M_E, x) * exp##suf(1.0 / (12.0 * x)); \
}
#define __jacl_lgamma(type,suf,PRE) static inline type lgamma##suf(type x){ \
	if ((x) < 0 && (type)(long)(x) == (x)) return (type)INFINITY; \
	type gamma_val = tgamma##suf(x); \
	signgam = (gamma_val < 0) ? -1 : 1; \
	return log##suf(fabs##suf(gamma_val)); \
}


#define signgam (*__signgam_ptr())
static inline int* __signgam_ptr(void) { static _Thread_local int signgam_val = 1;  return &signgam_val; }

// Remainder and Quotient: fmod, remainder, remquo
#define __jacl_fmod(type,suf,PRE)              static inline type fmod##suf(type x, type y){ \
	if (!isfinite(x) || !isfinite(y) || y == 0) return (type)NAN; \
	if (fabs##suf(x) < fabs##suf(y)) return x; \
	type n = trunc##suf(x / y); \
	return x - n * y; \
}
#define __jacl_remainder(type,suf,PRE)         static inline type remainder##suf(type x,type y){ type n = round##suf(x / y); return x - y * n; }
#define __jacl_remquo(type,suf,PRE)            static inline type remquo##suf(type x, type y, int* q){		type qq=round##suf(x / y); *q=(int)qq; return x- y * qq; }

/* FP Manipulation: nextafter, nexttoward */
#define __jacl_nextafter(type,suf,PRE)         static inline type nextafter##suf(type x,type y){ if (x == y) return x; return __jacl_valnext_##PRE(x, (x < y) ? 1 : -1); }
#define __jacl_nexttoward(type, suf, PRE)      static inline type nexttoward##suf(type x, long double y) { if ((long double)x == y) return (type)y; return __jacl_valnext_##PRE(x, ((long double)x < y) ? 1 : -1); }

//	Decomposition: modf
#define __jacl_modf(type, suf, PRE)            static inline type modf##suf(type x, type* iptr) { \
	*iptr = x; \
	JACL_SAFETY(isnan(x), x); \
	JACL_SAFETY(!isfinite(x), (type)0.0); \
	*iptr = trunc##suf(x); \
	return x - *iptr; \
}

// pi-trig (argument * π or result / π)
#define __jacl_sinpi(type,suf,PRE)             static inline type sinpi##suf(type x){ return sin##suf(x * M_PI); }
#define __jacl_cospi(type,suf,PRE)             static inline type cospi##suf(type x){ return cos##suf(x * M_PI); }
#define __jacl_tanpi(type,suf,PRE)             static inline type tanpi##suf(type x){ return tan##suf(x * M_PI); }
#define __jacl_asinpi(type,suf,PRE)            static inline type asinpi##suf(type x){ return asin##suf(x) / M_PI; }
#define __jacl_acospi(type,suf,PRE)            static inline type acospi##suf(type x){ return acos##suf(x) / M_PI; }
#define __jacl_atanpi(type,suf,PRE)            static inline type atanpi##suf(type x){ return atan##suf(x) / M_PI; }
#define __jacl_atan2pi(type,suf,PRE)           static inline type atan2pi##suf(type y,type x){ return atan2##suf(y, x) / M_PI; }

// integer/fractional power
#define __jacl_pown(type,suf,PRE)              static inline type pown##suf(type x, int n){ return pow##suf(x, (type)n); }
#define __jacl_powr(type,suf,PRE)              static inline type powr##suf(type x, type y) { \
	if (x < 0) return (type)NAN; \
	if (x == 0 && y <= 0) return (type)NAN; \
	return pow##suf(x, y); \
}

// rootn/compoundn
#define __jacl_rootn(type,suf,PRE)             static inline type rootn##suf(type x, int n){ return pow##suf(x, (type)1.0/n); }
#define __jacl_compoundn(type,suf,PRE)         static inline type compoundn##suf(type x, int n){ return pow##suf((type)1.0 + x, n); }

// nextup/nextdown
#define __jacl_nextup(type,suf,PRE)            static inline type nextup##suf(type x){ return nextafter##suf(x, (type)INFINITY); }
#define __jacl_nextdown(type,suf,PRE)          static inline type nextdown##suf(type x){ return nextafter##suf(x, (type)(-INFINITY)); }

// minimum/maximum
#define __jacl_fmaximum(type,suf,PRE)          static inline type fmaximum##suf(type x, type y){ return isnan(y) ? y : isnan(x) ? x : fmax##suf(x, y); }
#define __jacl_fminimum(type,suf,PRE)          static inline type fminimum##suf(type x, type y){ return isnan(y) ? y : isnan(x) ? x : fmin##suf(x, y); }
#define __jacl_fmaximum_mag(type,suf,PRE)      static inline type fmaximum_mag##suf(type x, type y){ type ax = fabs##suf(x), ay = fabs##suf(y); return isnan(y) ? y : isnan(x) ? x : ax > ay ? x : ay > ax ? y : (x > 0 ? x : y); }
#define __jacl_fminimum_mag(type,suf,PRE)      static inline type fminimum_mag##suf(type x, type y){ type ax = fabs##suf(x), ay = fabs##suf(y); return isnan(y) ? y : isnan(x) ? x : ax < ay ? x : ay < ax ? y : (x > 0 ? x : y); }
#define __jacl_fmaximum_num(type,suf,PRE)      static inline type fmaximum_num##suf(type x, type y){ return isnan(x) ? y : isnan(y) ? x : fmax##suf(x, y); }
#define __jacl_fminimum_num(type,suf,PRE)      static inline type fminimum_num##suf(type x, type y){ return isnan(x) ? y : isnan(y) ? x : fmin##suf(x, y); }
#define __jacl_fmaximum_mag_num(type,suf,PRE)  static inline type fmaximum_mag_num##suf(type x, type y){ return isnan(x) ? y : isnan(y) ? x : fmaximum_mag##suf(x, y); }
#define __jacl_fminimum_mag_num(type,suf,PRE)  static inline type fminimum_mag_num##suf(type x, type y){ return isnan(x) ? y : isnan(y) ? x : fminimum_mag##suf(x, y); }

// NAN Payload: canonicalize, getpayload, setpayload, setpayloadsig
#define __jacl_canonicalize(type,suf,PRE)      static inline int canonicalize##suf(type *cx, const type *x) { if (!cx || !x) return 1; *cx = *x; return 0; }
#define __jacl_getpayload(type,suf,PRE)        static inline type getpayload##suf(const type *x) { if (!x || !isnan(*x)) return (type)-1.0; return (type)__jacl_payloadget_##PRE(*x); }
#define __jacl_setpayload(type,suf,PRE)        static inline int setpayload##suf(type *res, type pl) { \
	if (!res || pl < 0) return 1; \
	uint64_t payload = (uint64_t)pl; \
	if ((type)payload != pl) return 1; \
	*res = __jacl_payloadset_##PRE(payload, 0); \
	return 0; \
}
#define __jacl_setpayloadsig(type,suf,PRE)     static inline int setpayloadsig##suf(type *res, type pl) { \
	if (!res || pl < 0) return 1; \
	uint64_t payload = (uint64_t)pl; \
	if ((type)payload != pl) return 1; \
	*res = __jacl_payloadset_##PRE(payload, 1); \
	return 0; \
}

#define JACL_OP_add +
#define JACL_OP_sub -
#define JACL_OP_mul *
#define JACL_OP_div /

// Narrow Arithmatic: faddf, fsubf, fmulf, faddfl, fsubf;, fmulfl, fdivfl, daddl, dsubl, dmull, ddivl
#define __jacl_narrow_fn(name, fn) \
	__jacl_narrow_##fn(name, float, f, f, FLT, double) \
	__jacl_narrow_##fn(name, float, f, l, FLT, long double) \
	__jacl_narrow_##fn(name, double, d, l, DBL, long double)
#define __jacl_narrow_op(name, type, pre, suf, PRE, arg) static inline type pre##name##suf(arg x, arg y) { \
	arg r = x JACL_OP_##name y; \
	if (!isfinite(r)) return (type)r; \
	if (r > (arg)PRE##_MAX) return (type)INFINITY; \
	if (r < (arg)(-PRE##_MAX)) return (type)(-INFINITY); \
	return (type)r; \
}
#define __jacl_narrow_sqrt(name, type, pre, suf, PRE, arg) static inline type pre##name##suf(arg x) { \
	arg r = sqrt##suf(x); \
	if (!isfinite(r)) return (type)r; \
	if (r > (arg)PRE##_MAX) return (type)INFINITY; \
	return (type)r; \
}
#define __jacl_narrow_fma(name, type, pre, suf, PRE, arg) \
static inline type pre##name##suf(arg x, arg y, arg z) { \
	arg r = fma##suf(x, y, z); \
	if (!isfinite(r)) return (type)r; \
	if (r > (arg)PRE##_MAX) return (type)INFINITY; \
	if (r < (arg)(-PRE##_MAX)) return (type)(-INFINITY); \
	return (type)r; \
}
#define __jacl_msign(name) __jacl_narrow_fn(name, op)
#define __jacl_mnop(name) __jacl_narrow_fn(name, name)

// Array Reductions: reduc_sum, reduce_sumsq, reduc_sumabs, reduc_sumsqmag, reduce_dot
#define __jacl_reduc(name, type, suf, loop_check, operation) static inline type reduc_##name##suf(size_t n, const type p[static n]) { \
	if (n == 0) return (type)0; \
	type sum = (type)0; \
	for (size_t i = 0; i < n; i++) { loop_check; sum += operation; } \
	return sum; \
}
#define __jacl_reduc_sum(type,suf,PRE)           __jacl_reduc(sum, type, suf, if (isinf(p[i])) return p[i]; if (isnan(p[i])) return (type)NAN;, p[i])
#define __jacl_reduc_sumsq(type,suf,PRE)         __jacl_reduc(sumsq, type, suf, if (isinf(p[i])) return (type)INFINITY; if (isnan(p[i])) return (type)NAN;, p[i] * p[i])
#define __jacl_reduc_sumabs(type,suf,PRE)        __jacl_reduc(sumabs, type, suf, if (isinf(p[i])) return (type)INFINITY; if (isnan(p[i])) return (type)NAN;, fabs##suf(p[i]))
#define __jacl_reduc_sumsqmag(type,suf,PRE)      static inline type reduc_sumsqmag##suf(size_t n, const type p[static n]) { return reduc_sumsq##suf(n, p); }
#define __jacl_reduc_dot(type,suf,PRE)           static inline type reduc_dot##suf(size_t n, const type p[static n], const type q[static n]) { \
	if (n == 0) return (type)0; \
	type sum = (type)0; \
	for (size_t i = 0; i < n; i++) { \
		type prod = p[i] * q[i]; \
		if (isinf(prod) && !(isinf(p[i]) || isinf(q[i]))) return (type)NAN; \
		if (isnan(prod)) return (type)NAN; \
		sum += prod; \
	} \
	return sum; \
}

#if JACL_HAS_IMMINTRIN
#include <immintrin.h>

// Square root - hardware is always faster
#undef __jacl_sqrt
#define __jacl_sqrt(type,suf,PRE)                static inline type sqrt##suf(type x){ \
	if (sizeof(type) == sizeof(float)) return (type)_mm_cvtss_f32(_mm_sqrt_ps(_mm_set_ss((float)x))); \
	else if (sizeof(type) == sizeof(double)) return (type)_mm_cvtsd_f64(_mm_sqrt_pd(_mm_set_sd((double)x))); \
	else { \
		type g; int i, e; \
		JACL_SAFETY(x<0, (type)NAN); \
		JACL_SAFETY(x==0||x==1||!isfinite(x), x); \
		e=__jacl_expfind_##PRE(x); \
		g=__jacl_expscal_##PRE((type)1.0,e/2); \
		for(i=0;i<4;++i)g=(g+x/g)/2; \
		return g; \
	} \
}

// Rounding functions (SSE4.1+ or NEON)
#if JACL_HAS_SSE4_1 || JACL_HAS_NEON
#undef __jacl_ceil
#define __jacl_ceil(type,suf,PRE)                static inline type ceil##suf(type x){ \
	if (sizeof(type) == sizeof(float)) return (type)_mm_cvtss_f32(_mm_ceil_ps(_mm_set_ss((float)x))); \
	else if (sizeof(type) == sizeof(double)) return (type)_mm_cvtsd_f64(_mm_ceil_pd(_mm_set_sd((double)x))); \
	else { \
		long long i; type t; \
		if (!(x == x)) return x; \
		i = (long long)x; t = (type)i; \
		if (t < x) t += (type)1; \
		return t; \
	} \
}

#undef __jacl_floor
#define __jacl_floor(type,suf,PRE)               static inline type floor##suf(type x){ \
	if (sizeof(type) == sizeof(float)) return (type)_mm_cvtss_f32(_mm_floor_ps(_mm_set_ss((float)x))); \
	else if (sizeof(type) == sizeof(double)) return (type)_mm_cvtsd_f64(_mm_floor_pd(_mm_set_sd((double)x))); \
	else { \
		long long i; type t; \
		if (!(x == x)) return x; \
		i = (long long)x; t = (type)i; \
		if (t > x) t -= (type)1; \
		return t; \
	} \
}

#undef __jacl_trunc
#define __jacl_trunc(type,suf,PRE)               static inline type trunc##suf(type x){ \
	if (sizeof(type) == sizeof(float)) return (type)_mm_cvtss_f32(_mm_trunc_ps(_mm_set_ss((float)x))); \
	else if (sizeof(type) == sizeof(double)) return (type)_mm_cvtsd_f64(_mm_trunc_pd(_mm_set_sd((double)x))); \
	else { \
		long long i; type ax, t; \
		if(!(x == x) || x == 0) return x; \
		ax = x < (type)0 ? -x : x; \
		if(ax >= (type)(1ULL<<(PRE##_MANT_DIG-1)))return x; \
		i=(long long)x; t = (type)i; \
		return t == ((type)0) ? x * (type)0.0 : t; \
	} \
}

#undef __jacl_round
#define __jacl_round(type,suf,PRE)               static inline type round##suf(type x){ \
	if (sizeof(type) == sizeof(float)) return (type)_mm_cvtss_f32(_mm_round_ps(_mm_set_ss((float)x))); \
	else if (sizeof(type) == sizeof(double)) return (type)_mm_cvtsd_f64(_mm_round_pd(_mm_set_sd((double)x))); \
	else { \
		type t = truncl(x); type diff = x - t; \
		if (diff >= (type)0.5) return t + (type)1; \
		if (diff <= (type)-0.5) return t - (type)1; \
		return t; \
	} \
}
#endif /* SSE4.1 or NEON */

#endif /* JACL_HAS_IMMINTRIN */

// Now declare
__jacl_math(isnormal)
__jacl_math(signbit)
__jacl_math(fabs)

__jacl_math(rmodes)
__jacl_math(ceil)
__jacl_math(floor)
__jacl_math(trunc)  // Moved here: depends on rmodes, required by fmod
__jacl_math(fmod)   // Depends on trunc

__jacl_math(exp)
__jacl_math(ldexp)
__jacl_math(frexp)

__jacl_math(log)
__jacl_math(log10)
__jacl_math(sqrt)

#if JACL_HAS_C99
	__jacl_math(exp2)
	__jacl_math(exp10)
#endif /* JACL_HAS_C99 */

__jacl_math(pow)
__jacl_math(sin)
__jacl_math(cos)
__jacl_math(tan)

__jacl_math(asin)
__jacl_math(acos)
__jacl_math(atan)
__jacl_math(atan2)

__jacl_math(sinh)
__jacl_math(cosh)
__jacl_math(tanh)
__jacl_math(modf)

#if JACL_HAS_C99
	__jacl_math(nan)
	__jacl_math(copysign)
	__jacl_math(expm1)
	__jacl_math(log2)
	__jacl_math(logb)
	__jacl_math(ilogb)
	__jacl_math(log1p)

	__jacl_math(cbrt)
	__jacl_math(hypot)
	// trunc moved earlier (before fmod which depends on it)

	__jacl_math(round)
	__jacl_math(lround)
	__jacl_math(llround)
	__jacl_math(rint)
	__jacl_math(lrint)
	__jacl_math(llrint)
	__jacl_math(nearbyint)

	__jacl_math(fdim)
	__jacl_math(fmax)
	__jacl_math(fmin)
	__jacl_math(fma)

	__jacl_math(scalbn)
	__jacl_math(scalbln)
	__jacl_math(asinh)
	__jacl_math(acosh)
	__jacl_math(atanh)

	__jacl_math(erf)
	__jacl_math(erfc)

	__jacl_math(tgamma)
	__jacl_math(lgamma)

	__jacl_math(remainder)
	__jacl_math(remquo)
	__jacl_math(nextafter)
	__jacl_math(nexttoward)
#endif /* JACL_HAS_C99 */

#if JACL_HAS_C23
	__jacl_math(fromfp)
	__jacl_math(ufromfp)
	__jacl_math(fromfpx)
	__jacl_math(ufromfpx)

	__jacl_math(llogb)
	__jacl_math(sinpi)
	__jacl_math(cospi)
	__jacl_math(tanpi)

	__jacl_math(asinpi)
	__jacl_math(acospi)
	__jacl_math(atanpi)
	__jacl_math(atan2pi)

	__jacl_math(pown)
	__jacl_math(powr)
	__jacl_math(rootn)
	__jacl_math(compoundn)

	__jacl_math(nextup)
	__jacl_math(nextdown)
	__jacl_math(roundeven)

	__jacl_math(fmaximum)
	__jacl_math(fminimum)
	__jacl_math(fmaximum_mag)
	__jacl_math(fminimum_mag)
	__jacl_math(fmaximum_num)
	__jacl_math(fminimum_num)
	__jacl_math(fmaximum_mag_num)
	__jacl_math(fminimum_mag_num)

	__jacl_math(canonicalize)
	__jacl_math(getpayload)
	__jacl_math(setpayload)
	__jacl_math(setpayloadsig)

	__jacl_msign(add)
	__jacl_msign(sub)
	__jacl_msign(mul)
	__jacl_msign(div)
	__jacl_mnop(sqrt)
	__jacl_mnop(fma)

	__jacl_math(reduc_sum)
	__jacl_math(reduc_sumsq)
	__jacl_math(reduc_dot)
	__jacl_math(reduc_sumabs)
	__jacl_math(reduc_sumsqmag)
#endif /* JACL_HAS_C23 */

// Bessel Prototypes
static inline double j0(double x) {
	JACL_SAFETY(isnan(x), x);
	JACL_SAFETY(isinf(x), 0.0);

	double ax = fabs(x);

	if (ax < 8.0) {
		double y = x * x, n = 57568490574.0 + y * (-13362590354.0 + y * (651619640.7 + y * (-11214424.18 + y * (77392.33017 + y * (-184.9052456)))));
		double d = 57568490411.0 + y * (1029532985.0 + y * (9494680.718 + y * (59272.64853 + y * (267.8532712 + y))));

		return n / d;
	} else {
		double z = 8.0 / ax, y = z * z, xx = ax - 0.785398164, p = 1.0 + y * (-0.1098628627e-2 + y * (0.2734510407e-4 + y * (-0.2073370639e-5 + y * 0.2093887211e-6)));
		double q = -0.1562499995e-1 + y * (0.1430488765e-3 + y * (-0.6911147651e-5 + y * (0.7621095161e-6 - y * 0.934945152e-7)));

		return sqrt(2.0 / M_PI / ax) * (p * cos(xx) - z * q * sin(xx));
	}
}
static inline double j1(double x) {
	JACL_SAFETY(isnan(x), x);
	JACL_SAFETY(isinf(x), 0.0);

	double ax = fabs(x);

	if (ax < 8.0) {
		double y = x * x, n = x * (72362614232.0 + y * (-7895059235.0 + y * (242396853.1 + y * (-2972611.439 + y * (15704.48260 + y * (-30.16036606))))));
		double d = 144725228442.0 + y * (2300535178.0 + y * (18583304.74 + y * (99447.43394 + y * (376.9991397 + y * 1.0))));

		return n / d;
	} else {
		double z = 8.0 / ax, y = z * z, xx = ax - 2.356194491, p = 1.0 + y * (0.183105e-2 + y * (-0.3516396496e-4 + y * (0.2457520174e-5 + y * (-0.240337019e-6))));
		double q = 0.04687499995 + y * (-0.2002690873e-3 + y * (0.8449199096e-5 + y * (-0.88228987e-6 + y * 0.105787412e-6))), ans = sqrt(2.0 / M_PI / ax) * (p * cos(xx) - z * q * sin(xx));

		return x < 0.0 ? -ans : ans;
	}
}
static inline double jn(int n, double x) {
	JACL_SAFETY(isnan(x), x);
	JACL_SAFETY(isinf(x), 0.0);
	if (n == 0) return j0(x);
	if (n == 1) return j1(x);

	double tox = 2.0 / fabs(x), bjm = j0(x), bj = j1(x), bjp;

	for (int j = 1; j < n; j++) { bjp = j * tox * bj - bjm; bjm = bj; bj = bjp; }

	return bj;
}

static inline double y0(double x) {
	JACL_SAFETY(isnan(x), x);
	JACL_SAFETY(x < 0.0, NAN);
	JACL_SAFETY(x == 0.0, -INFINITY);
	JACL_SAFETY(isinf(x), 0.0);

	if (x < 8.0) {
		double j0val = j0(x), y = x * x, num = -2957821389.0 + y * (7062834065.0 + y * (-512359803.6 + y * (10879881.29 + y * (-86327.92757 + y * 228.4622733))));
		double den = 40076544269.0 + y * (745249964.8 + y * (7189466.438 + y * (47447.26470 + y * (226.1030244 + y * 1.0))));

		return num / den + 2.0 / M_PI * log(x) * j0val;
	} else {
		double z = 8.0/x, y = z * z, xx = x - 0.785398164, p = 1.0 + y * (-0.1098628627e-2 + y * (0.2734510407e-4 + y * (-0.2073370639e-5 + y * 0.2093887211e-6)));
		double q = -0.1562499995e-1 + y * (0.1430488765e-3 + y * (-0.6911147651e-5 + y * (0.7621095161e-6 - y * 0.934945152e-7)));

		return sqrt(2.0 / M_PI / x) * (p * sin(xx) + z*q*cos(xx));
	}
}
static inline double y1(double x) {
	JACL_SAFETY(isnan(x), x);
	JACL_SAFETY(x < 0.0, NAN);
	JACL_SAFETY(x == 0.0, -INFINITY);
	JACL_SAFETY(isinf(x), 0.0);

	if (x < 8.0) {
		double j1val = j1(x), y = x * x, num = x * (-0.4900604943e13 + y * (0.1275274390e13 + y * (-0.5153438139e11 + y * (0.7349264551e9 + y * (-0.4237922726e7 + y * 0.8511937935e4)))));
		double den = 0.2499580570e14 + y * (0.4244419664e12 + y * (0.3733650367e10 + y * (0.2245904002e8 + y * (0.1020426050e6 + y * (0.3549632885e3 + y * 1.0)))));

		return num / den + 2.0 / M_PI * (j1val * log(x) - 1.0/x);
	} else {
		double z = 8.0/x, y = z * z, xx = x - 2.356194491, p = 1.0 + y * (0.183105e-2 + y * (-0.3516396496e-4 + y * (0.2457520174e-5 + y * (-0.240337019e-6))));
		double q = 0.04687499995 + y * (-0.2002690873e-3 + y * (0.8449199096e-5 + y * (-0.88228987e-6 + y * 0.105787412e-6)));

		return sqrt(2.0/M_PI/x) * (p*sin(xx) + z*q*cos(xx));
	}
}
static inline double yn(int n, double x) {
	JACL_SAFETY(isnan(x), x);
	JACL_SAFETY(x < 0.0, NAN);
	JACL_SAFETY(x == 0.0, -INFINITY);
	JACL_SAFETY(isinf(x), 0.0);
	if (n == 0) return y0(x);
	if (n == 1) return y1(x);

	double tox = 2.0/x, by = y1(x), bym = y0(x), byp;

	for (int j = 1; j < n; j++) { byp = j * tox * by - bym; bym = by; by = byp; }

	return by;
}

#ifdef __cplusplus
}
#endif

#endif /* MATH_H */
