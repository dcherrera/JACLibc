# Converts Linux syscall.tbl or syscall_table.c to X(SYS_name, num, "desc")

# Skip comments and blank lines
/^[[:space:]]*#/ { next }
/^[[:space:]]*$/ { next }

# Handle .tbl format: "number abi name entry ..."
/^[0-9]+[[:space:]]/ {
	nr   = $1
	abi  = $2
	name = $3

	# Filter by arch ABI
	if (arch == "x64" && abi != "64" && abi != "common") next
	if (arch == "x86" && abi != "i386" && abi != "common") next
	if (arch == "arm64" && abi != "64" && abi != "common") next
	if (arch == "arm32" && abi != "common" && abi != "eabi") next

	# Skip empty or placeholder entries
	if (name == "") next

	printf "X(SYS_%s, %d, %s)\n", name, nr, name
	next
}

# Handle C table format: "[ number ] = { .entry = sys_name ..."
/^\[/ {
	num = $1
	gsub(/[\[\]]/, "", num)

	for (i = 1; i <= NF; i++) {
		if ($i == ".entry" && $(i+1) == "=") {
			name = $(i+2)
			gsub(/[,;]/, "", name)
			sub(/^sys_/, "", name)
			printf "X(SYS_%s, %d, %s)\n", name, num, name
			break
		}
	}
}
