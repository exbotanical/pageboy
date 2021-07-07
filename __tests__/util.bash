# env
BIN_NAME=pageboy

# util
check_bin() {
	if [[ ! -f $BIN_NAME ]]; then
		echo 0
	else
		echo 1
	fi
}

run_command_sequence() {
	OG_IFS=$IFS
	IFS='>'

	data=$((
	cat <<END
$(for_each ${@})
.exit
END
) | ./$BIN_NAME)

	data=${data//pageboy/#}

	echo $data | tr -d '[:space:]'

	IFS="$OG_IFS"
}


# meta util
panic () {
  local exit_status=$1

  shift

  echo "[-] ERROR ($(current_time)): $*" >&2
  exit $exit_status
}

current_time () {
  echo $(date +'%Y-%m-%dT%H:%M:%S%z')
}

for_each () {
  local -a arr=($@)

  for item in "${arr[@]}"; do
    echo $item;
  done
}
