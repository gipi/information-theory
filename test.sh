#!/bin/bash
set -e

exec 2>.test.log

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

[[ -f hm ]] || die "hm not found"
[[ -f fbits ]] || die "fbits not found"

TMPDIR=$(mktemp -d )

# fbits #############################################################
# 8f = 10001111
echo -e '\x8f\x8f' > ${TMPDIR}/fbits-test.txt

bold "Single byte test"
diff <( echo -ne 'ftell: 1 10001111
') <( ./fbits ${TMPDIR}/fbits-test.txt 8 ) && ok ' OK\n'



bold "Test byte crossing"
diff -Nur <( echo -ne 'ftell: 1 00000100
ftell: 1 00000011
ftell: 2 00000111
' ) <(./fbits ${TMPDIR}/fbits-test.txt 3 3 3) && ok ' OK\n'

# hm ################################################################
TEST_TEXT='prima linea\n'


bold "huffman: all character"
# this python snippet prints all code from 0 to 255
python -c 'for i in range(256):print "%c" % i,' | ./hm >/dev/null && ok ' Ok\n'

bold "huffman: canonical table"
diff -Nur <( echo -ne 'p	000
r	001
m	010
n	011
a	100
i	101
e	1100
l	1101

	1110
 	1111
') <(echo -ne "${TEST_TEXT}" | ./hm -c 2>&-) && ok ' OK\n'


bold "huffman: encoding"
echo -ne "${TEST_TEXT}" | ./hm 2>&- > ${TMPDIR}/encoded.txt || \
	die " something goes wrong"

diff -Nur <( echo -ne '00000000  09 70 03 72 03 6d 03 6e  03 61 03 69 03 65 04 6c  |.p.r.m.n.a.i.e.l|
00000010  04 0a 04 20 04 06 a9 fb  5e 4e 00                 |... ....^N.|
0000001b
') <( cat ${TMPDIR}/encoded.txt | hexdump -C ) && ok ' OK\n'


bold "huffman: decoding"
diff -Nur <( echo -ne "${TEST_TEXT}") <(./hm -d ${TMPDIR}/encoded.txt) && ok ' OK'

# use the return code to test for error
# use the string returned for debug (is it possible?)
if [ $? != 0 ]
then
	fail " fail\n"
fi

# clean all
#rm -fr ${TMPDIR}
