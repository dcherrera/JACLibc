/**
 * NETBSD SIGNALS IMPORTED  //  last updated: 2025-12-01 15:17:01 UTC
 * http://cvsweb.netbsd.org/bsdweb.cgi/src/sys/sys/signal.h?rev=HEAD&content-type=text/plain
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
	X(SIGPWR, 32) \
	X(SIGRTMIN, 33) \
	X(SIGRTMAX, 63)

/* Signal action flags */
#define SIGNAL_ACTION_FLAGS(X) \
	X(SA_ONSTACK, 0x0001) \
	X(SA_RESTART, 0x0002) \
	X(SA_RESETHAND, 0x0004) \
	X(SA_NOCLDSTOP, 0x0008) \
	X(SA_NODEFER, 0x0010) \
	X(SA_NOCLDWAIT, 0x0020) \
	X(SA_SIGINFO, 0x0040) \
	X(SA_NOKERNINFO, 0x0080) \
	X(SA_ALLBITS, 0x00ff)

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
	X(SIGEV_SA, 3)

/* Signal info codes */
#define SIGNAL_INFO_CODES(X) \
	X(SI_ASYNCIO, -5) \
	X(SI_QUEUE, -4) \
	X(SI_USER, -3) \
	X(SI_TIMER, -2) \
	X(SI_MESGQ, -1)

