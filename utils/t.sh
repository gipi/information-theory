#!/bin/bash

. test-lib.sh

test_expect_to_succed 'readbits from buffer' '
	diff -Nur <( echo -ne "\xaa" | ./test-bits  1 1 1 1 4 ) <( echo -ne "00000001 00000000 00000001 00000000 00001010 ")
'

test_expect_to_succed 'readbits walk through bytes' '
	diff -Nur <( echo -ne "\x55\x55" | ./test-bits  4 4 4 ) <( echo -ne "00000101 00000101 00000101 ")
'
