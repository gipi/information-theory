#!/bin/bash


set -e

die () {
	echo "$1"
	exit $?
}

[[ -f hm ]] || die "hm not found"

TMPDIR=$(mktemp -d )

cat <<EOF >${TMPDIR}/huffman-c-output.txt
c	00
d	01
a	10
b	11
EOF

diff ${TMPDIR}/huffman-c-output.txt <(echo -n 'abcd' | ./hm -c) || \
	die "test failed"

rm -fr ${TMPDIR}
