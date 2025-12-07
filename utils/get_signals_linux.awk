# linux signals

BEGIN {
	# Value buckets
	sigs_count = 0
	sa_count = 0
	mask_count = 0
	sigev_count = 0
	si_count = 0
	ill_count = 0
	fpe_count = 0
	segv_count = 0
	bus_count = 0
	trap_count = 0
	cld_count = 0
	poll_count = 0
}

# Ignore blank and comment lines
/^[[:space:]]*$/ { next }
/^[[:space:]]*\/\*/ { next }
/^[[:space:]]*\*/ { next }

# #define lines
/^#define[ \t]+[A-Z_][A-Z0-9_]+[ \t]+/ {
	name = $2
	val  = $3

	# Save literal numeric macros for substitution (e.g., _NSIG 64)
	if (val ~ /^[0-9]+$/) {
		macros[name] = val
	}

	# Skip macro expressions
	if (val ~ /[(+*/)-]/) next

	# Resolve one-level macro references (e.g., SIGRTMAX _NSIG)
	if (val ~ /^[A-Z_][A-Z0-9_]*$/ && val in macros) {
		val = macros[val]
	}

	# SIGEV_* event types (check BEFORE SIG* to avoid being caught)
	if (name ~ /^SIGEV_[A-Z0-9_]+$/ && val ~ /^[0-9]+$/) {
		sigev[name] = val
		sigev_count++
		next
	}

	# SIGNAL_NUMBERS: SIG* (not SIGEV, not control, not mask ops)
	if (name ~ /^SIG[A-Z][A-Z0-9_]*$/) {
		if (name ~ /^SIG(STKSZ|MINSIGSTKSZ|DFL|IGN|ERR|HOLD)$/) next
		if (name ~ /^SIG_(BLOCK|UNBLOCK|SETMASK)$/) next

		# numeric definition: first-wins per value
		if (val ~ /^-?[0-9]+$/) {
			if (!(val in seen_sig_val)) {
				seen_sig_val[val] = name
				sigs[name] = val
				sigs_count++
			}
		}
		# textual aliases (SIGIOT SIGABRT) ignored in first-wins mode
		next
	}

	# SA_* flags
	if (name ~ /^SA_[A-Z0-9_]+$/ && val ~ /^(0x)?[0-9A-Fa-f]+$/) {
		sa[name] = val
		sa_count++
		next
	}

	# SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK
	if (name ~ /^SIG_(BLOCK|UNBLOCK|SETMASK)$/ && val ~ /^[0-9]+$/) {
		mask_ops[name] = val
		mask_count++
		next
	}

	# SI_* info codes
	if (name ~ /^SI_[A-Z0-9_]+$/ && val ~ /^(0x)?-?[0-9A-Fa-f]+$/) {
		si[name] = val
		si_count++
		next
	}

	# Detail codes grouped by prefix
	if (name ~ /^ILL_[A-Z0-9_]+$/ && val ~ /^[0-9]+$/)  { ill[name]   = val; ill_count++ }
	if (name ~ /^FPE_[A-Z0-9_]+$/ && val ~ /^[0-9]+$/)  { fpe[name]   = val; fpe_count++ }
	if (name ~ /^SEGV_[A-Z0-9_]+$/ && val ~ /^[0-9]+$/) { segv[name]  = val; segv_count++ }
	if (name ~ /^BUS_[A-Z0-9_]+$/ && val ~ /^[0-9]+$/)  { bus[name]   = val; bus_count++ }
	if (name ~ /^TRAP_[A-Z0-9_]+$/ && val ~ /^[0-9]+$/) { trapc[name] = val; trap_count++ }
	if (name ~ /^CLD_[A-Z0-9_]+$/ && val ~ /^[0-9]+$/)  { cld[name]   = val; cld_count++ }
	if (name ~ /^POLL_[A-Z0-9_]+$/ && val ~ /^[0-9]+$/) { pollc[name] = val; poll_count++ }
}

function emit_family(title, macro, arr,    sorted, n, i, trail) {
	if (length(arr) == 0) return
	print "/* " title " */"
	printf "#define %s(X) \\\n", macro
	n = asorti(arr, sorted, "by_value_then_name")
	for (i = 1; i <= n; i++) {
		trail = (i == n) ? "" : " \\"
		printf "\tX(%s, %s)%s\n", sorted[i], arr[sorted[i]], trail
	}
	print ""
}

function by_value_then_name(i1, v1, i2, v2,    n1, n2) {
	if (v1 ~ /^0x/) n1 = strtonum(v1); else n1 = v1 + 0
	if (v2 ~ /^0x/) n2 = strtonum(v2); else n2 = v2 + 0
	if (n1 < n2) return -1
	if (n1 > n2) return 1
	return (i1 < i2) ? -1 : (i1 > i2)
}

END {
	# Signal numbers (first-wins, no aliases)
	if (sigs_count > 0) {
		print "/* Signal numbers */"
		print "#define SIGNAL_NUMBERS(X) \\"
		n = asorti(sigs, sorted, "by_value_then_name")
		for (i = 1; i <= n; i++) {
			trail = (i == n) ? "" : " \\"
			printf "\tX(%s, %s)%s\n", sorted[i], sigs[sorted[i]], trail
		}
		print ""
	}

	# Synthesize core POSIX SI_* as negative sentinels when missing
	core_si["SI_USER"]    = 1
	core_si["SI_QUEUE"]   = 2
	core_si["SI_TIMER"]   = 3
	core_si["SI_ASYNCIO"] = 4
	core_si["SI_MESGQ"]   = 5

	minsi = 0
	for (k in si) {
		v = si[k] + 0
		if (v < minsi) minsi = v
	}
	off = minsi - 1
	for (k in core_si) {
		if (!(k in si)) {
			si[k] = off--
			si_count++
		}
	}

	emit_family("Signal action flags", "SIGNAL_ACTION_FLAGS", sa)
	emit_family("Signal mask operations", "SIGNAL_MASK_OPS", mask_ops)
	emit_family("Signal event notification types", "SIGNAL_EVENT_TYPES", sigev)
	emit_family("Signal info codes", "SIGNAL_INFO_CODES", si)

	emit_family("Signal detail codes (ILL_*)", "SIGNAL_ILL_CODES", ill)
	emit_family("Signal detail codes (FPE_*)", "SIGNAL_FPE_CODES", fpe)
	emit_family("Signal detail codes (SEGV_*)", "SIGNAL_SEGV_CODES", segv)
	emit_family("Signal detail codes (BUS_*)", "SIGNAL_BUS_CODES", bus)
	emit_family("Signal detail codes (TRAP_*)", "SIGNAL_TRAP_CODES", trapc)
	emit_family("Signal detail codes (CLD_*)", "SIGNAL_CLD_CODES", cld)
	emit_family("Signal detail codes (POLL_*)", "SIGNAL_POLL_CODES", pollc)
}
