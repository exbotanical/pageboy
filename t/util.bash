BIN_NAME=pageboy

# ensure binary exists
check_bin() {
	if [[ ! -f $BIN_NAME ]]; then
		echo 0
	else
		echo 1
	fi
}

run_command() {
  ./$BIN_NAME test.db
}

# feed each line of input to interactive session;
# echo output from said session
run_command_sequence() {
	OG_IFS=$IFS
	IFS='>'

	data=$( (
	cat <<END
$(for_each "${@}")
.exit
END
) | ./$BIN_NAME test.db)

  data=${data//pageboy/#}

  # shellcheck disable=SC2086
	echo $data | tr -d '[:space:]'

	IFS="$OG_IFS"
}

# augment each item in an array :: "item" -> "item;"
for_each () {
  local -a arr=("$@")

  for item in "${arr[@]}"; do
    echo "$item";
  done
}
