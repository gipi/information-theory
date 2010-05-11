bold() {
	echo -en '\e[1m'"$1"'\e[0m'
}

ok() {
	bold '\e[32m'"$1"'\e[0m'
}

fail() {
	bold '\e[31m'"$1"'\e[0m'
}


die () {
	echo "$1"
	exit $?
}


# test_expect_to_succed 'description' 'cmds, one for line'
test_expect_to_succed() {
	echo "--- $1" 1>&2
	eval "$2" 1>&2
	if [ $? -eq 0 ];then ok 'Ok!'; else fail 'fail!';fi
	bold " $1\n"
}
