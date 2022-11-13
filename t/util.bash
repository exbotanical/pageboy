BIN_NAME=pageboy

# ensure binary exists
check_bin() {
	if [[ ! -f $BIN_NAME ]]; then
		echo 0
	else
		echo 1
	fi
}

# feed each line of input to interactive session;
# echo output from said session
run_command_sequence() {
	OG_IFS=$IFS
	IFS='>'

	data=$((
	cat <<END
$(for_each ${@})
.exit
END
) | ./$BIN_NAME test.db)

  data=${data//pageboy/#}

	echo $data | tr -d '[:space:]'

	IFS="$OG_IFS"
}

# exit immediately
panic () {
  local exit_status=$1

  shift

  echo "[-] ERROR ($(current_time)): $*" >&2
  exit $exit_status
}

# generate timestamp
current_time () {
  echo $(date +'%Y-%m-%dT%H:%M:%S%z')
}

# augment each item in an array :: "item" -> "item;"
for_each () {
  local -a arr=($@)

  for item in "${arr[@]}"; do
    echo $item;
  done
}
