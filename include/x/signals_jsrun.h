/**
 * JSRUN/WASM SIGNALS
 * Signal definitions for WebAssembly running in JavaScript environment.
 * Uses Linux-compatible values for library compatibility.
 * WASM doesn't support traditional Unix signals, but these definitions
 * are needed for library compatibility.
 */

/* Signal numbers */
#define SIGNAL_NUMBERS(X) \
	X(SIGHUP, 1) \
	X(SIGINT, 2) \
	X(SIGQUIT, 3) \
	X(SIGILL, 4) \
	X(SIGTRAP, 5) \
	X(SIGABRT, 6) \
	X(SIGBUS, 7) \
	X(SIGFPE, 8) \
	X(SIGKILL, 9) \
	X(SIGUSR1, 10) \
	X(SIGSEGV, 11) \
	X(SIGUSR2, 12) \
	X(SIGPIPE, 13) \
	X(SIGALRM, 14) \
	X(SIGTERM, 15) \
	X(SIGSTKFLT, 16) \
	X(SIGCHLD, 17) \
	X(SIGCONT, 18) \
	X(SIGSTOP, 19) \
	X(SIGTSTP, 20) \
	X(SIGTTIN, 21) \
	X(SIGTTOU, 22) \
	X(SIGURG, 23) \
	X(SIGXCPU, 24) \
	X(SIGXFSZ, 25) \
	X(SIGVTALRM, 26) \
	X(SIGPROF, 27) \
	X(SIGWINCH, 28) \
	X(SIGIO, 29) \
	X(SIGPWR, 30) \
	X(SIGSYS, 31) \
	X(SIGRTMIN, 32) \
	X(SIGRTMAX, 64)

/* Signal action flags */
#define SIGNAL_ACTION_FLAGS(X) \
	X(SA_NOCLDSTOP, 0x00000001) \
	X(SA_NOCLDWAIT, 0x00000002) \
	X(SA_SIGINFO, 0x00000004) \
	X(SA_UNSUPPORTED, 0x00000400) \
	X(SA_EXPOSE_TAGBITS, 0x00000800) \
	X(SA_ONSTACK, 0x08000000) \
	X(SA_RESTART, 0x10000000) \
	X(SA_NODEFER, 0x40000000) \
	X(SA_RESETHAND, 0x80000000)

/* Signal mask operations */
#define SIGNAL_MASK_OPS(X) \
	X(SIG_BLOCK, 0) \
	X(SIG_UNBLOCK, 1) \
	X(SIG_SETMASK, 2)

/* Signal event notification types */
#define SIGNAL_EVENT_TYPES(X) \
	X(SIGEV_SIGNAL, 0) \
	X(SIGEV_NONE, 1) \
	X(SIGEV_THREAD, 2) \
	X(SIGEV_THREAD_ID, 4) \
	X(SIGEV_MAX_SIZE, 64)

/* Signal info codes */
#define SIGNAL_INFO_CODES(X) \
	X(SI_ASYNCIO, -4) \
	X(SI_QUEUE, -3) \
	X(SI_TIMER, -2) \
	X(SI_MESGQ, -1) \
	X(SI_USER, 0) \
	X(SI_KERNEL, 0x80) \
	X(SI_MAX_SIZE, 128)

/* Signal detail codes (ILL_*) */
#define SIGNAL_ILL_CODES(X) \
	X(ILL_ILLOPC, 1) \
	X(ILL_ILLOPN, 2) \
	X(ILL_ILLADR, 3) \
	X(ILL_ILLTRP, 4) \
	X(ILL_PRVOPC, 5) \
	X(ILL_PRVREG, 6) \
	X(ILL_COPROC, 7) \
	X(ILL_BADSTK, 8) \
	X(ILL_BADIADDR, 9)

/* Signal detail codes (FPE_*) */
#define SIGNAL_FPE_CODES(X) \
	X(FPE_INTDIV, 1) \
	X(FPE_INTOVF, 2) \
	X(FPE_FLTDIV, 3) \
	X(FPE_FLTOVF, 4) \
	X(FPE_FLTUND, 5) \
	X(FPE_FLTRES, 6) \
	X(FPE_FLTINV, 7) \
	X(FPE_FLTSUB, 8) \
	X(FPE_FLTUNK, 14) \
	X(FPE_CONDTRAP, 15)

/* Signal detail codes (SEGV_*) */
#define SIGNAL_SEGV_CODES(X) \
	X(SEGV_MAPERR, 1) \
	X(SEGV_ACCERR, 2) \
	X(SEGV_BNDERR, 3) \
	X(SEGV_ACCADI, 5) \
	X(SEGV_ADIDERR, 6) \
	X(SEGV_ADIPERR, 7) \
	X(SEGV_MTEAERR, 8) \
	X(SEGV_MTESERR, 9) \
	X(SEGV_CPERR, 10)

/* Signal detail codes (BUS_*) */
#define SIGNAL_BUS_CODES(X) \
	X(BUS_ADRALN, 1) \
	X(BUS_ADRERR, 2) \
	X(BUS_OBJERR, 3) \
	X(BUS_MCEERR_AR, 4) \
	X(BUS_MCEERR_AO, 5)

/* Signal detail codes (TRAP_*) */
#define SIGNAL_TRAP_CODES(X) \
	X(TRAP_BRKPT, 1) \
	X(TRAP_TRACE, 2) \
	X(TRAP_BRANCH, 3) \
	X(TRAP_HWBKPT, 4) \
	X(TRAP_UNK, 5) \
	X(TRAP_PERF, 6)

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
