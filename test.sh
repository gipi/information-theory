#!/bin/bash
#set -e

. test-lib.sh

exec 2>.test.log

[[ -f hm ]] || die "hm not found"
[[ -f fbits ]] || die "fbits not found"

TMPDIR=$(mktemp -d )
bold "Temporary directory: "${TMPDIR}"\n"

# fbits #############################################################
# 8f = 10001111
echo -e '\x8f\x8f' > ${TMPDIR}/fbits-test.txt

test_expect_to_succed 'Single byte test' "
	diff <( echo -ne 'ftell: 1 10001111
') <( ./fbits ${TMPDIR}/fbits-test.txt 8 )"



test_expect_to_succed 'Test byte crossing' "
diff -Nur <( echo -ne 'ftell: 1 00000100
ftell: 1 00000011
ftell: 2 00000111
' ) <(./fbits ${TMPDIR}/fbits-test.txt 3 3 3)"

# hm ################################################################
TEST_TEXT='prima linea\n'

test_expect_to_succed 'All characters encode' "
	# this python snippet prints all code from 0 to 255
	python -c 'for i in range(256):print \"%c\" % i,' | ./hm > ${TMPDIR}/all.hu "

test_expect_to_succed 'All characters decompress' '
	./hm -d ${TMPDIR}/all.hu
'

test_expect_to_succed "huffman: canonical table" "
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
') <(echo -ne \"${TEST_TEXT}\" | ./hm -c 2>&-)"


test_expect_to_succed "huffman: encoding" "
	echo -ne \"${TEST_TEXT}\" | ./hm 2>&- > ${TMPDIR}/encoded.txt
	diff -Nur <( echo -ne '00000000  09 70 03 72 03 6d 03 6e  03 61 03 69 03 65 04 6c  |.p.r.m.n.a.i.e.l|
00000010  04 0a 04 20 04 06 a9 fb  5e 4e 00                 |... ....^N.|
0000001b
') <( cat ${TMPDIR}/encoded.txt | hexdump -C )
"


test_expect_to_succed "huffman: decoding" "
	diff -Nur <( echo -ne \"${TEST_TEXT}\") <(./hm -d ${TMPDIR}/encoded.txt)
"

test_expect_to_succed 'huffman: random' "
	head -c 4096 /dev/urandom > ${TMPDIR}/random.txt
	./hm ${TMPDIR}/random.txt > ${TMPDIR}/random.hu
	diff -Nur ${TMPDIR}/random.txt <( ./hm -d ${TMPDIR}/random.hu)
"

# clean all
#rm -fr ${TMPDIR}
