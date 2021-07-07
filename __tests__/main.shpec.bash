OG_IFS="$IFS"

IFS=$'\n'

set -o nounset

source "$(dirname "$(readlink -f "$BASH_SOURCE")")"/util.bash

# test vars
EXECUTED="Executedstatement#"

# man, these tests are a fucking mess
# make it a BIG todo to rewrite these in node

# base cases
describe "pageboy"
  it "inserts and retrieves a row"
  (
    _shpec_failures=0
    result=$(run_command_sequence 'insert 1 user user@username.com' 'select')
    assert equal "#$EXECUTED(1,user,user@username.com)$EXECUTED" "$result"
    return "$_shpec_failures"
  ); (( _shpec_failures += $? ))
  end

  it "outputs an error message when the table is at capacity"
  (
    rc=""
    for _ in {1..1401}; do
      rc+='insert 1 a a>'
    done

    _shpec_failures=0
    result=$((run_command_sequence $rc) 2>&1)

    result=${result//#Executedstatement/}
    echo $result
    assert equal "Table memory full\n##" "$result"
    return "$_shpec_failures"
  ); (( _shpec_failures += $? ))
  end

	it "handles arbitrary, non-alphanumeric characters"
  (
    _shpec_failures=0
    result=$(run_command_sequence 'insert 1 user user...' 'select')
    assert equal "#$EXECUTED(1,user,user...)$EXECUTED" "$result"
    return "$_shpec_failures"
  ); (( _shpec_failures += $? ))
  end
end



IFS=$OG_IFS
