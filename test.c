/*
 * mathexpr test suite
 * Copyright (C) 2008-2015 Aaron Marks. All Rights Reserved.
 */

#include <stdio.h>
#include <math.h>
#include "src/mathexpr.h"

MathState state;

int hadTest = 0;
int hadPass = 0;
int hadFail = 0;

/*
 * Test that expression evaluates to value
 */
int Test_assert(char *expr, MATHEXPR_T val) {
    hadTest++;
    printf("Test:   %24s%4s%-8i    ", expr, " => ", val);
    int actual = Math_evalWithState(&state, expr);
    if (actual == val) {
        printf("PASS!\n");
        hadPass++;
        return 1;
    } else {
        printf("FAIL! [ %i ]\n", actual);
        hadFail++;
        return 0;
    }
}

int main(int argc, char *argv[]) {
    printf("***** mathexpr test suite!\n");
    printf("***** Copyright (C) 2008-2015 Aaron Marks. All Rights Reserved.\n");

	MathState_init(&state);

    Test_assert("1 + 2", 3);
    Test_assert("2 - 1", 1);
    Test_assert("1 + 2 - 3", 0);

    Test_assert("+1", 1);
    Test_assert("-1", -1);
    Test_assert("1 - -1", 2);
    Test_assert("-3 + 5", 2);
    Test_assert("+3 + 5", 8);
    Test_assert("!1", 0);
    Test_assert("!0", 1);
    Test_assert("!0 + 1", 2);

    Test_assert("1 + 2 * 3", 7);
    Test_assert("2 * 3 / 2", 3);
    Test_assert("7 % 3", 1);

    Test_assert("(1 + 2) * 3", 9);
    Test_assert("2 * (2 + (2 + 2))", 12);

    Test_assert("1234 == 1234", 1);
    Test_assert("1234 == 321", 0);
    Test_assert("1234 != 321", 1);
    Test_assert("1234 != 1234", 0);

    Test_assert("10 < 21", 1);
    Test_assert("21 < 10", 0);
    Test_assert("21 > 10", 1);
    Test_assert("10 > 21", 0);

    Test_assert("10 <= 21", 1);
    Test_assert("10 <= 10", 1);
    Test_assert("21 <= 10", 0);
    Test_assert("10 >= 21", 0);
    Test_assert("10 >= 10", 1);
    Test_assert("21 >= 10", 1);

    Test_assert("1 ? 1 : 0", 1);
    Test_assert("0 ? 1 : 0", 0);
    Test_assert("(1 ? 1 : 0) ? 3 : 6", 3);
    Test_assert("(0 ? 1 : 0) ? 3 : 6", 6);

    Test_assert("20 || 10", 20);
    Test_assert("0 || 20", 20);
    Test_assert("20 && 30", 30);
    Test_assert("20 && 0", 0);

    Test_assert("1 & 2", 0);
    Test_assert("3 & 2", 2);
    Test_assert("1 | 2", 3);
    Test_assert("3 ^ 1", 2);

    /* these must be in this order -- otherwise the RNG sequence will be out */
    Test_assert("2d6", 6);
    Test_assert("d20", 6);
    Test_assert("-d6", -2);
    Test_assert("-2d6", -3);
    Test_assert("2 d 6", 7);
    Test_assert("2 d 6 + 3", 11);

#ifdef MATHEXPR_FUNCTIONS
    unsigned int rndVal;
    init_genrand(12);
    rndVal = genrand_int32();

    //Test_assert("sin(90)", 1); /* makes no sense to test this currently */
    Test_assert("seed(12)", 1);
    Test_assert("rand()", rndVal);

 #ifdef MATHEXPR_VARIABLES
    Test_assert("i = 32", 32);
    Test_assert("i", 32);
    Test_assert("(i == 32) ? 1 : 0", 1);
    Test_assert("i = i + 1", 33);
    Test_assert("i", 33);
    Test_assert("j = 10", 10);
    Test_assert("j", 10);
 #endif
#endif

    printf("%i tests, %i passes, %i fails\n", hadTest, hadPass, hadFail);
    if (hadFail)
        return 1;
    return 0;
}

