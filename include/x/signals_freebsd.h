/**
 * FREEBSD SIGNALS IMPORTED  //  last updated: 2025-12-01 15:16:58 UTC
 * https://cgit.freebsd.org/src/plain/sys/sys/signal.h
 */

/* Signal numbers */
#define SIGNAL_NUMBERS(X) \
	X(SIGHUP, 1) \
	X(SIGINT, 2) \
	X(SIGQUIT, 3) \
	X(SIGILL, 4) \
	X(SIGTRAP, 5) \
	X(SIGABRT, 6) \
	X(SIGEMT, 7) \
	X(SIGFPE, 8) \
	X(SIGKILL, 9) \
	X(SIGBUS, 10) \
	X(SIGSEGV, 11) \
	X(SIGSYS, 12) \
	X(SIGPIPE, 13) \
	X(SIGALRM, 14) \
	X(SIGTERM, 15) \
	X(SIGURG, 16) \
	X(SIGSTOP, 17) \
	X(SIGTSTP, 18) \
	X(SIGCONT, 19) \
	X(SIGCHLD, 20) \
	X(SIGTTIN, 21) \
	X(SIGTTOU, 22) \
	X(SIGIO, 23) \
	X(SIGXCPU, 24) \
	X(SIGXFSZ, 25) \
	X(SIGVTALRM, 26) \
	X(SIGPROF, 27) \
	X(SIGWINCH, 28) \
	X(SIGINFO, 29) \
	X(SIGUSR1, 30) \
	X(SIGUSR2, 31) \
	X(SIGTHR, 32) \
	X(SIGLIBRT, 33) \
	X(SIGRTMIN, 65) \
	X(SIGRTMAX, 126)

/* Signal action flags */
#define SIGNAL_ACTION_FLAGS(X) \
	X(SA_ONSTACK, 0x0001) \
	X(SA_RESTART, 0x0002) \
	X(SA_RESETHAND, 0x0004) \
	X(SA_NOCLDSTOP, 0x0008) \
	X(SA_NODEFER, 0x0010) \
	X(SA_NOCLDWAIT, 0x0020) \
	X(SA_SIGINFO, 0x0040)

/* Signal mask operations */
#define SIGNAL_MASK_OPS(X) \
	X(SIG_BLOCK, 1) \
	X(SIG_UNBLOCK, 2) \
	X(SIG_SETMASK, 3)

/* Signal event notification types */
#define SIGNAL_EVENT_TYPES(X) \
	X(SIGEV_NONE, 0) \
	X(SIGEV_SIGNAL, 1) \
	X(SIGEV_THREAD, 2) \
	X(SIGEV_KEVENT, 3) \
	X(SIGEV_THREAD_ID, 4)

/* Signal info codes */
#define SIGNAL_INFO_CODES(X) \
	X(SI_NOINFO, 0) \
	X(SI_UNDEFINED, 0) \
	X(SI_USER, 0x10001) \
	X(SI_QUEUE, 0x10002) \
	X(SI_TIMER, 0x10003) \
	X(SI_ASYNCIO, 0x10004) \
	X(SI_MESGQ, 0x10005) \
	X(SI_KERNEL, 0x10006) \
	X(SI_LWP, 0x10007)

/* Signal detail codes (ILL_*) */
#define SIGNAL_ILL_CODES(X) \
	X(ILL_ILLOPC, 1) \
	X(ILL_ILLOPN, 2) \
	X(ILL_ILLADR, 3) \
	X(ILL_ILLTRP, 4) \
	X(ILL_PRVOPC, 5) \
	X(ILL_PRVREG, 6) \
	X(ILL_COPROC, 7) \
	X(ILL_BADSTK, 8)

/* Signal detail codes (FPE_*) */
#define SIGNAL_FPE_CODES(X) \
	X(FPE_INTOVF, 1) \
	X(FPE_INTDIV, 2) \
	X(FPE_FLTDIV, 3) \
	X(FPE_FLTOVF, 4) \
	X(FPE_FLTUND, 5) \
	X(FPE_FLTRES, 6) \
	X(FPE_FLTINV, 7) \
	X(FPE_FLTSUB, 8) \
	X(FPE_FLTIDO, 9)

/* Signal detail codes (SEGV_*) */
#define SIGNAL_SEGV_CODES(X) \
	X(SEGV_MAPERR, 1) \
	X(SEGV_ACCERR, 2) \
	X(SEGV_PKUERR, 100)

/* Signal detail codes (BUS_*) */
#define SIGNAL_BUS_CODES(X) \
	X(BUS_ADRALN, 1) \
	X(BUS_ADRERR, 2) \
	X(BUS_OBJERR, 3) \
	X(BUS_OOMERR, 100)

/* Signal detail codes (TRAP_*) */
#define SIGNAL_TRAP_CODES(X) \
	X(TRAP_BRKPT, 1) \
	X(TRAP_TRACE, 2) \
	X(TRAP_DTRACE, 3) \
	X(TRAP_CAP, 4)

/* Signal detail codes (CLD_*) */
#define SIGNAL_CLD_CODES(X) \
	X(CLD_EXITED, 1) \
	X(CLD_KILLED, 2) \
	X(CLD_DUMPED, 3) \
	X(CLD_TRAPPED, 4) \
	X(CLD_STOPPED, 5) \
	X(CLD_CONTINUED, 6)

/* Signal detail codes (POLL_*) */
#define SIGNAL_POLL_CODES(X) \
	X(POLL_IN, 1) \
	X(POLL_OUT, 2) \
	X(POLL_MSG, 3) \
	X(POLL_ERR, 4) \
	X(POLL_PRI, 5) \
	X(POLL_HUP, 6)

