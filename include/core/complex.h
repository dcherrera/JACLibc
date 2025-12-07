/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef CORE_COMPLEX_H
#define CORE_COMPLEX_H
#pragma once

#include <config.h>

/**
 * ex  = sc3 / dc3 / xc3
 * type = float / double / long double
 * suf  = f /  / l  (reserved if you later want type-specific math)
 */

/* (a + bi) * (c + di) = (ac - bd) + (ad + bc)i */
#define __jacl_mulc(ex, type, suf) type complex __mul##ex(type a, type b, type c, type d) { \
	type real = a * c - b * d; \
	type imag = a * d + b * c; \
	return (type complex)(real + I * imag); \
}

/* (a + bi) / (c + di) = [(ac + bd) + (bc - ad)i] / (c² + d²) */
#define __jacl_divc(ex, type, suf) type complex __div##ex(type a, type b, type c, type d) { \
	type denom = c * c + d * d; \
	if (denom == (type)0) { \
		type real = (type)INFINITY; \
		type imag = (type)INFINITY; \
		return (type complex)(real + I * imag); \
	} \
	type real = (a * c + b * d) / denom; \
	type imag = (b * c - a * d) / denom; \
	return (type complex)(real + I * imag); \
}

/* Instantiate for the three precisions */
__jacl_mulc(sc3, float,       f)
__jacl_mulc(dc3, double,      )
__jacl_mulc(xc3, long double, l)

__jacl_divc(sc3, float,       f)
__jacl_divc(dc3, double,      )
__jacl_divc(xc3, long double, l)

#endif /* CORE_COMPLEX_H */
