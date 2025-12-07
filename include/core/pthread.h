/* (c) 2025 FRINKnet & Friends – MIT licence */
#ifndef CORE_PTHREAD_H
#define CORE_PTHREAD_H
#pragma once

#include <config.h>
#include <pthread.h>

#if PTHREAD_POSIX

_Atomic unsigned int __jacl_pthread_tls_key_counter = 0;
thread_local void *__jacl_pthread_tls_values[MAX_TLS_KEYS] = {0};
void (*__jacl_pthread_key_destructors[MAX_TLS_KEYS])(void *) = {0};
pthread_key_t __jacl_pthread_self_key = 0;
_Atomic int __jacl_pthread_inited = 0;

#endif

#endif /* CORE_PTHREAD_H */
