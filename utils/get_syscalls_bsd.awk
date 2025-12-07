# Converts FreeBSD/NetBSD multi-line syscalls.master to X(SYS_name, num, "desc")

# Skip comments and blank lines
/^[[:space:]]*#/ { next }
/^[[:space:]]*$/ { next }

# State machine for multi-line blocks
{
	if (inblock) {
		block = block "\n" $0
		# End of block?
		if ($0 ~ /};[[:space:]]*$/ || $0 ~ /}[[:space:]]*$/) {
			process_block(cur_num, block)
			inblock = 0
			block = ""
		}
		next
	}
}

# First line of a syscall entry: starts with number
/^[0-9]+[[:space:]]/ {
	cur_num = $1

	# Check if this line has a complete single-line block
	if ($0 ~ /\{.*};[[:space:]]*$/) {
		block = $0
		process_block(cur_num, block)
		inblock = 0
		block = ""
	} else if ($0 ~ /\{/) {
		# Start of multi-line prototype block
		inblock = 1
		block = $0
	} else {
		# OBSOL/RESERVED/etc without a prototype block
		inblock = 0
		block = ""
	}
	next
}

function process_block(num, blk,    m, sig, name) {
	# Extract between first '{' and final '}' or '};'
	if (!match(blk, /\{([^}]*)};?[[:space:]]*$/, m))
		return

	sig = m[1]

	# Look for the function name: last identifier before '('
	# This handles "ssize_t read(", "void *mmap(", etc.
	if (!match(sig, /([A-Za-z_][A-Za-z0-9_]*)[[:space:]]*\(/, m))
		return

	name = m[1]

	# Strip sys_ prefix if present
	sub(/^sys_/, "", name)

	printf "X(SYS_%s, %d, %s)\n", name, num, name
}

