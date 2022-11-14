# shellcheck disable=SC2086

OG_IFS="$IFS"
IFS=$'\n'

# shellcheck disable=SC2128
source "$(dirname "$(readlink -f "$BASH_SOURCE")")"/util.bash

shopt -s expand_aliases

# These aliases ensure we're able to run each test case
# in a subshell (i.e. its own closure) so as not to pollute the
# global namespace; we also pass along shpec variables that
# must be extant in both contexts
alias it='(_shpec_failures=0; alias setup &>/dev/null && { setup; unalias setup; alias teardown &>/dev/null && trap teardown EXIT ;}; it'
# shellcheck disable=SC2154
alias ti='return "$_shpec_failures"); (( _shpec_failures += $?, _shpec_examples++ ))'
alias end_describe='end; unalias setup teardown 2>/dev/null'

#############
### Tests ###
#############

DB_FILE='test.db'

# This is a little trick we use to capture the
# program's output by framing stdout
EXECUTED="Executedstatement#"

MAX_CAPACITY=13
EMAIL='user@username.com'
USERNAME='user'

describe 'pageboy'

  alias setup="rm $DB_FILE &> /dev/null && touch $DB_FILE"
  alias teardown="rm $DB_FILE"

  it 'inserts and retrieves a row'
    result=$(run_command_sequence "insert 1 $USERNAME $EMAIL" 'select')
    assert equal "#$EXECUTED(1,$USERNAME,$EMAIL)$EXECUTED" "$result"
  ti

  # it 'outputs an error message when the table is at capacity'
  #   rc=""
  #   for (( c=1; c <= MAX_CAPACITY + 1; c++ )); do
  #     rc+="insert $c a a>"
  #   done

  #   result=$( (run_command_sequence $rc) 2>&1)
  #   result=${result//#Executedstatement/}
  #   assert equal 'Table memory full\n##' "$result"
  # ti

	it 'supports the insertion of arbitrary, non-alphanumeric characters'
    username_w_nonalpha='user...'

    result=$(run_command_sequence "insert 1 $USERNAME $username_w_nonalpha" 'select')
    assert equal "#$EXECUTED(1,$USERNAME,$username_w_nonalpha)$EXECUTED" "$result"
  ti

  it 'supports the insertion of maximum length values'
    long_username=$(printf 'x%.0s' {1..32})
    long_email=$(printf 'y%.0s' {1..255})

    result=$(run_command_sequence "insert 1 $long_username $long_email" 'select')
    assert equal "#$EXECUTED(1,$long_username,$long_email)$EXECUTED" "$result"
  ti

  it 'prints an error message when trying to insert a username that is too long'
    long_username=$(printf 'x%.0s' {1..33})

    result=$( (run_command_sequence "insert 1 $long_username $EMAIL") 2>&1)
    assert equal "Provided input was too long\n##" "$result"
  ti

  it 'prints an error message when trying to insert an email that is too long'
    long_email=$(printf 'y%.0s' {1..256})

    result=$( (run_command_sequence "insert 1 $USERNAME $long_email") 2>&1)
    assert equal "Provided input was too long\n##" "$result"
  ti

  it 'prints an error message when not provided an id'
    result=$( (run_command_sequence "insert  $USERNAME $EMAIL") 2>&1)
    assert equal "Syntax error. Could not parse statement\n##" "$result"
  ti

  it 'prints an error message when not provided a username'
    result=$( (run_command_sequence "insert 1  $EMAIL") 2>&1)
    assert equal "Syntax error. Could not parse statement\n##" "$result"
  ti

  it 'prints an error message when not provided an email'
    result=$( (run_command_sequence "insert 1 $USERNAME") 2>&1)
    assert equal "Syntax error. Could not parse statement\n##" "$result"
  ti

  it 'prints an error message when provided a negative id'
    result=$( (run_command_sequence "insert -1 $USERNAME $EMAIL") 2>&1)
    assert equal "Provided negative id\n##" "$result"
  ti

  it 'persists data between executions'
    run_command_sequence "insert 1 $USERNAME $EMAIL"
    result=$(run_command_sequence 'select')
    assert equal "#(1,$USERNAME,$EMAIL)$EXECUTED" "$result"
  ti

  it 'prints the btree structure via the meta command .btree'
    result=$(run_command_sequence '.btree')
    assert equal "#TODO#" "$result"
  ti

  it 'prints internal configurations and settings via the meta command .settings'
    result=$(run_command_sequence '.settings')
    assert equal "#TODO#" "$result"
  ti


end_describe

IFS=$OG_IFS
