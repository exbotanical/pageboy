OG_IFS="$IFS"
IFS=$'\n'

DB_FILE=test.db

source "$(dirname "$(readlink -f "$BASH_SOURCE")")"/util.bash

shopt -s expand_aliases

# These aliases ensure we're able to run each test case
# in a subshell (i.e. its own closure) so as not to pollute the
# global namespace; we also pass along shpec variables that
# must be extant in both contexts
alias it='(_shpec_failures=0; alias setup=setup &>/dev/null && { setup; unalias setup; alias teardown=teardown &>/dev/null && trap teardown EXIT ;}; it'
alias ti='return "$_shpec_failures"); (( _shpec_failures += $?, _shpec_examples++ ))'
alias end_describe='end; unalias setup teardown 2>/dev/null'

# This is a little trick we use to capture the
# program's output by framing stdout
EXECUTED="Executedstatement#"

setup () {
	rm $DB_FILE &> /dev/null
	touch $DB_FILE
}

teardown () {
	rm $DB_FILE
}

#############
### Tests ###
#############
email='user@username.com'
username='user'

describe 'pageboy'
  it 'inserts and retrieves a row'
    result=$(run_command_sequence "insert 1 $username $email" 'select')
    assert equal "#$EXECUTED(1,$username,$email)$EXECUTED" "$result"
  ti

  it 'outputs an error message when the table is at capacity'
    rc=""
    for _ in {1..1301}; do
      rc+='insert 1 a a>'
    done

    result=$((run_command_sequence $rc) 2>&1)
    result=${result//#Executedstatement/}
    assert equal 'Table memory full\n##' "$result"
  ti

	it 'supports the insertion of arbitrary, non-alphanumeric characters'
    username_w_nonalpha='user...'
    result=$(run_command_sequence "insert 1 $username $username_w_nonalpha" 'select')
    assert equal "#$EXECUTED(1,$username,$username_w_nonalpha)$EXECUTED" "$result"
  ti

  it 'supports the insertion of maximum length values'
    long_username=$(printf 'x%.0s' {1..32})
    long_email=$(printf 'y%.0s' {1..255})

    result=$(run_command_sequence "insert 1 $long_username $long_email" 'select')
    assert equal "#$EXECUTED(1,$long_username,$long_email)$EXECUTED" "$result"
  ti

  it 'prints an error message when trying to insert a username that is too long'
    long_username=$(printf 'x%.0s' {1..33})

    result=$((run_command_sequence "insert 1 $long_username $email") 2>&1)
    assert equal "Provided input was too long\n##" "$result"
  ti

  it 'prints an error message when trying to insert an email that is too long'
    long_email=$(printf 'y%.0s' {1..256})

    result=$((run_command_sequence "insert 1 $username $long_email") 2>&1)
    assert equal "Provided input was too long\n##" "$result"
  ti

  it 'prints an error message when not provided an id'
    result=$((run_command_sequence "insert  $username $email") 2>&1)
    assert equal "Syntax error. Could not parse statement\n##" "$result"
  ti

  it 'prints an error message when not provided a username'
    result=$((run_command_sequence "insert 1  $email") 2>&1)
    assert equal "Syntax error. Could not parse statement\n##" "$result"
  ti

  it 'prints an error message when not provided an email'
    result=$((run_command_sequence "insert 1 $username") 2>&1)
    assert equal "Syntax error. Could not parse statement\n##" "$result"
  ti

  it 'prints an error message when provided a negative id'
    result=$((run_command_sequence "insert -1 $username $email") 2>&1)
    assert equal "Provided negative id\n##" "$result"
  ti

  it 'persists data between executions'
    run_command_sequence "insert 1 $username $email"
    result=$(run_command_sequence 'select')
    assert equal "#(1,$username,$email)$EXECUTED" "$result"
  ti

end_describe

IFS=$OG_IFS
