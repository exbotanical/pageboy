###################
### Environment ###
###################

OG_IFS="$IFS"

IFS=$'\n'

set -o nounset

source "$(dirname "$(readlink -f "$BASH_SOURCE")")"/util.bash

##########################
### Harness Extensions ###
##########################

EXECUTED="Executedstatement#"

before_each () {
	rm test.db &> /dev/null
	touch test.db
}

after_each () {
	rm test.db
}

#############
### Tests ###
#############

describe "pageboy"
	before_each
  it "inserts and retrieves a row"
  (
		# this whole rigmarole allows us to scope each test to its own closure
    _shpec_failures=0
    result=$(run_command_sequence 'insert 1 user user@username.com' 'select')
    assert equal "#$EXECUTED(1,user,user@username.com)$EXECUTED" "$result"
    return "$_shpec_failures"
  ); (( _shpec_failures += $? ))
  end
	after_each

	before_each
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
	after_each

	before_each
	it "handles arbitrary, non-alphanumeric characters"
  (
    _shpec_failures=0
    result=$(run_command_sequence 'insert 1 user user...' 'select')
    assert equal "#$EXECUTED(1,user,user...)$EXECUTED" "$result"
    return "$_shpec_failures"
  ); (( _shpec_failures += $? ))
  end
	after_each
end

IFS=$OG_IFS

# man, these tests are a fucking mess
# make it a BIG todo to rewrite these in node
