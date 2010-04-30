# . bash_bits.sh

hex2bits () {
	echo  'obase=2;ibase=16;'$1 | bc
}
