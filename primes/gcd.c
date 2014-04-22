/**
 * Code for Greatest common divisor taken by "Cryptography engineering".
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


int gcd(int a, int b) {
    assert(a > 0);
    assert(b > 0);

    // this variable is used to swap (a, b) <- ( b mod a, a)
    int c;

    while (a > 0) {
        c = b % a;
        b = a;
        a = c;
    }

    return b;
}

void usage(char* command) {
    printf("usage: %s <a> <b>", command);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[2]);

    int c = gcd(a, b);

    printf("%d", c);

    return 0;
}
