#include <stdio.h>

int compute(int a, int b) {

    int d = a * 8 + b; // Strength reduction (mul -> shl)
    int e = d / 16; // Strength reduction (div -> shr)
    int c = a + b;
    int f = a + 1; // Dead code
    e = e * (a + b); // CSE -> (a + b)
    e = c + e; // Make sure 'c' isnt dead
    return e;
}

int main() {

    int a = 100;
    int b = 2;
    printf("%d\n", compute(a, b));

    return 0;
}