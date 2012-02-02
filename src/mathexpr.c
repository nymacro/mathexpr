/*
 * mathexpr -- Mathematical Expression Evaluator
 * Copyright (C) 2008-2010 Aaron Marks. All Rights Reserved.
 */
#include "mathexpr.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdint.h>

void init_genrand(unsigned long s);
unsigned long genrand_int32(void);

MATHEXPR_T Math_expression(MathState *state);

#ifdef MATHEXPR_FUNCTIONS

void Math_addFunction(MathState *state, char *name, MathExprFunction func) {
    if (state->functionCount >= MATHEXPR_FUNCTIONS_MAX) {
        fprintf(stderr, "ERROR: tried to add function to MathExpr when max is reached!\n");
        return;
    }
	strncpy(state->functions[state->functionCount].name, name, MATHEXPR_NAME_MAX);
    state->functions[state->functionCount++].func = func;
}

static void Math_clearFunctions(MathState *state) {
    int i;
    state->functionCount = 0;
    for (i = 0; i < MATHEXPR_FUNCTIONS_MAX; i++) {
		strcpy(state->functions[i].name, "");
        state->functions[i].func = NULL;
    }
}

static MATHEXPR_T Math_Std_sin(int argc, MATHEXPR_T argv[]) {
    if (argc != 1) {
        return 0;
    } else {
        return sin(argv[0]);
    }
}

static MATHEXPR_T Math_Std_seed(int argc, MATHEXPR_T argv[]) {
    if (argc != 1)
        return 0;
	init_genrand(argv[0]);
    //srand(argv[0]);
    return 1;
}

static MATHEXPR_T Math_Std_rand(int argc, MATHEXPR_T argv[]) {
    if (argc != 0)
        return 0;
	return genrand_int32();
    //return rand();
}

static void Math_registerStdFunctions(MathState *state) {
    Math_addFunction(state, "sin", Math_Std_sin);
    Math_addFunction(state, "seed", Math_Std_seed);
    Math_addFunction(state, "rand", Math_Std_rand);
}

 #ifdef MATHEXPR_VARIABLES
void Math_addVariable(MathState *state, char *name, MATHEXPR_T value) {
	if (state->variableCount >= MATHEXPR_VARIABLES_MAX) {
        fprintf(stderr, "ERROR: tried to add variable to MathExpr when max is reached!\n");
        return;
    }
	strncpy(state->variables[state->variableCount].name, name, MATHEXPR_NAME_MAX);
    state->variables[state->variableCount++].value = value;
}

void Math_setVariable(MathState *state, char *name, MATHEXPR_T value) {
	int i;
	for (i = 0; i < state->variableCount; i++) {
		if (strcmp(name, state->variables[i].name) == 0) {
			state->variables[i].value = value;
			return;
		}
	}
	Math_addVariable(state, name, value);
}

static void Math_clearVariables(MathState *state) {
    int i;
    state->variableCount = 0;
    for (i = 0; i < MATHEXPR_VARIABLES_MAX; i++) {
        strcpy(state->variables[i].name, "");
        state->variables[i].value = 0;
    }
}
 #endif

#endif

/* Math_randomRange
 * Generate number within range (discarding skewed numbers)
 */
static int Math_randomRange(int low, int high) {
	unsigned int randomNumber;
	int range = high - low + 1; /* inclusive */
	
	if (low >= high)
		return 0;
	
	/* generate a random number which is equally distrubuted between the range wanted */
	do {
		randomNumber = genrand_int32();
	} while (randomNumber > INT32_MAX - (INT32_MAX % range));
	
	return low + randomNumber % range;
}

/**********************************************************************/

/* Math_expected:
 * Error output function for use internally
 */
void Math_expected(MathState *state, char *msg) {
    fprintf(stderr, "ERROR: %s expected\n", msg);
}

/* Math_look:
 * Look at the current character in the buffer
 */
char Math_look(MathState *state) {
    return state->p[0];
}

/* Math_peek:
 * Look at the next character in the buffer
 */
char Math_peek(MathState *state) {
    if (strlen(state->p) > 0)
        return state->p[1];
    return '\0';
}

/* Math_prev:
 * Get the previous character in the buffer
 */
char Math_prev(MathState *state) {
    if (state->p > state->buffer)
        return state->p[-1];
    return '\0';
}

/* Math_gotoNext:
 * Go to the next character in the buffer
 */
void Math_gotoNext(MathState *state) {
    state->p++;
}

/* Math_skipSpace:
 * Skip all the space characters beneath the cursor in buffer and after.
 */
void Math_skipSpace(MathState *state) {
    while (isspace(Math_look(state))) {
        Math_gotoNext(state);
    }
}

/* Math_match:
 * Match a character in the buffer
 */
void Math_match(MathState *state, char c) {
    while (*state->p) {
        if (*state->p == c) {
            state->p++;
            /* skip whitespace */
            Math_skipSpace(state);
            return;
        }
        state->p++;
    }
    Math_expected(state, "match");
}

/**********************************************************************/

/*
 * PRECEDENCE HEIRARCHY REFERENCE
 * http://www.difranco.net/cop2220/op-prec.htm
 */

/* Math_getFunction:
 * Evaluate a function and get it's result
 */
#ifdef MATHEXPR_FUNCTIONS
MATHEXPR_T Math_getFunction(MathState *state) {
    MATHEXPR_T value = 0;
    char functionName[33];
    int argc = 0;
    MATHEXPR_T args[32];
    int i;
    
    for (i = 0; i < 32; i++) {
        if (isalpha(Math_look(state))) {
            functionName[i] = Math_look(state);
            Math_gotoNext(state);
        } else {
            functionName[i] = '\0';
            break;
        }
    }
    functionName[32] = '\0';

    for (i = 0; i < state->functionCount; i++) {
        if (strcmp(functionName, state->functions[i].name) == 0) {
            if (Math_look(state) == '(') {
                int j;
                Math_match(state, '(');
                Math_skipSpace(state);
                if (Math_look(state) != ')') {
                    do {
                        value = Math_expression(state);
                        args[argc++] = value;
                        if (Math_look(state) == ',')
                            Math_match(state, ',');
                    } while (Math_look(state) != ')');
                }
                Math_match(state, ')');

                return state->functions[i].func(argc, args);
            }
        }
    }

 #ifdef MATHEXPR_VARIABLES
	for (i = 0; i < state->variableCount; i++) {
		if (strcmp(functionName, state->variables[i].name) == 0) {
			return state->variables[i].value;
		}
	}
 #endif

    return value;
}
#endif

/* Math_getNumber:
 * Get number from cursor position
 */
MATHEXPR_T Math_getNumber(MathState *state) {
    MATHEXPR_T value = 0;
    char numberBuf[64];
    int i;

    /* skip whitespace */
    Math_skipSpace(state);

    if (!isdigit(Math_look(state))) {
#ifdef MATHEXPR_FUNCTIONS
        value = Math_getFunction(state);
#else
        Math_expected(state, "Integer");
        return 0;
#endif
    } else {
	    while (isdigit(Math_look(state))) {
	        value = 10 * value + Math_look(state) - '0';
	        Math_gotoNext(state);
	    }
	}
    
    /* skip whitespace */
    Math_skipSpace(state);
    
    return value;
}

/* Math_getFactor:
 * Evaluate a factor (parentheses)
 */
MATHEXPR_T Math_getFactor(MathState *state) {
    if (Math_look(state) == '(') {
        MATHEXPR_T value = 0;
        Math_match(state, '(');
        value = Math_expression(state);
        Math_match(state, ')');
        return value;
    } else {
        return Math_getNumber(state);
    }
}

/* Math_getDiceUnary
 * Evaluate d notation
 */
MATHEXPR_T Math_getDiceUnary(MathState *state) {
    MATHEXPR_T value = 0;
    if (Math_look(state) != 'd') {
        value = Math_getFactor(state);
    } else {
		Math_match(state, 'd');
		value = Math_randomRange(1, Math_getFactor(state));
    }
    return value;
}

/* Math_getDice
 * Evalute d notation
 */
MATHEXPR_T Math_getDice(MathState *state) {
    MATHEXPR_T value = 0;
	MATHEXPR_T nextValue = 0;
	MATHEXPR_T times = 0;
	MATHEXPR_T total = 0;
	MATHEXPR_T i = 0;
    value = Math_getDiceUnary(state);
    while (Math_look(state) == 'd') {
        Math_match(state, 'd');
		times = value;
		value = 0;
		nextValue = Math_getDiceUnary(state);
		for (i = 0; i < times; i++) {
	        value += Math_randomRange(1, nextValue);
		}
    }
    return value;
}

/* Math_getUnary:
 * Evaluate a unary expression
 */
MATHEXPR_T Math_getUnary(MathState *state) {
    MATHEXPR_T value = 0;
    if (Math_look(state) != '!' &&
        Math_look(state) != '-' &&
        Math_look(state) != '+' &&
        Math_look(state) != '~') {
        value = Math_getDice(state);
    } else {
        switch (Math_look(state)) {
            case '+':
                Math_match(state, '+');
                value = Math_getDice(state);
                break;
            case '-':
                Math_match(state, '-');
                value = -Math_getDice(state);
                break;
            case '!':
                Math_match(state, '!');
                value = !Math_getDice(state);
                break;
            case '~':
                Math_match(state, '~');
                value = ~Math_getDice(state);
                break;
        }
    }
    return value;
}

/* Math_getTerm:
 * Evaluate a term expression
 */
MATHEXPR_T Math_getTerm(MathState *state) {
    MATHEXPR_T value = 0;
	MATHEXPR_T tmp = 0;
    value = Math_getUnary(state);
    while (Math_look(state) == '*' || Math_look(state) == '/' || Math_look(state) == '%' ||
		(Math_look(state) == '*' && Math_peek(state) == '*')) {

        /* exponentiation '**' */
        if (Math_look(state) == '*' && Math_peek(state) == '*') {

        }
        switch (Math_look(state)) {
            case '*':
                if (Math_peek(state) == '*') {
                    Math_match(state, '*');
                    Math_match(state, '*');
                    value = pow(value, Math_getUnary(state));
                } else {
                    Math_match(state, '*');
                    value = value * Math_getUnary(state);
                }
                break;
            case '/':
                Math_match(state, '/');
				tmp = Math_getUnary(state);
				if (tmp == 0) {
					printf("attempt to divide by zero!\n");
					tmp = 1;
				}
                value = value / tmp;
                break;
            case '%':
                Math_match(state, '%');
                value = value % Math_getUnary(state);
                break;
        }
    }
    return value;
}

/* Math_getSum:
 * Evaluate a sum expression
 */
MATHEXPR_T Math_getSum(MathState *state) {
    MATHEXPR_T value = 0;
    value = Math_getTerm(state);
    while (Math_look(state) == '+' || Math_look(state) == '-') {
        switch (Math_look(state)) {
            case '+':
                Math_match(state, '+');
                value = value  + Math_getTerm(state);
                break;
            case '-':
                Math_match(state, '-');
                value = value - Math_getTerm(state);
                break;
        }
    }
    return value;
}

/* Math_getRelational:
 * Evaluate an relational expression
 */
MATHEXPR_T Math_getRelational(MathState *state) {
    MATHEXPR_T value = 0;
    value = Math_getSum(state);
    while (Math_look(state) == '>' || Math_look(state) == '<') {
        if (Math_look(state) == '>' && Math_peek(state) == '=') {
            Math_match(state, '>');
            Math_match(state, '=');
            value = value >= Math_getSum(state);
        } else if (Math_look(state) == '<' && Math_peek(state) == '=') {
            Math_match(state, '<');
            Math_match(state, '=');
            value = value <= Math_getSum(state);
        } else if (Math_look(state) == '>') {
            Math_match(state, '>');
            value = value > Math_getSum(state);
        } else if (Math_look(state) == '<') {
            Math_match(state, '<');
            value = value < Math_getSum(state);
        }
    }
    return value;
}

/* Math_getEquality:
 * Evaluate an equality expression
 */
MATHEXPR_T Math_getEquality(MathState *state) {
    MATHEXPR_T value = 0;
    value = Math_getRelational(state);
    while ((Math_look(state) == '=' && Math_peek(state) == '=') ||
           (Math_look(state) == '!' && Math_peek(state) == '=')) {
        switch (Math_look(state)) {
            case '=':
                Math_match(state, '=');
                Math_match(state, '=');
                value = value == Math_getRelational(state);
                break;
            case '!':
                Math_match(state, '!');
                Math_match(state, '=');
                value = value != Math_getRelational(state);
                break;
        }
    }
    return value;
}

/* Math_getBitwiseAnd:
 * Evaluate a bitwise and expression
 */
MATHEXPR_T Math_getBitwiseAnd(MathState *state) {
    MATHEXPR_T value = 0;
    value = Math_getEquality(state);
    while (Math_look(state) == '&' && Math_peek(state) != '&') {
        Math_match(state, '&');
        value = value & Math_getEquality(state);
    }
    return value;
}

/* Math_getBitwiseXor
 * Evaluate a bitwise xor
 */
MATHEXPR_T Math_getBitwiseXor(MathState *state) {
    MATHEXPR_T value = 0;
    value = Math_getBitwiseAnd(state);
    while (Math_look(state) == '^') {
        Math_match(state, '^');
        value = value ^ Math_getBitwiseAnd(state);
    }
    return value;
}

/* Math_getBitwiseOr
 * Evaluate a bitwise or expression
 */
MATHEXPR_T Math_getBitwiseOr(MathState *state) {
    MATHEXPR_T value = 0;
    value = Math_getBitwiseXor(state);
    while (Math_look(state) == '|' && Math_peek(state) != '|') {
        Math_match(state, '|');
        value = value | Math_getBitwiseXor(state);
    }
    return value;
}

/* Math_getLogicalAnd:
 * Evaluate a logical and expression
 */
MATHEXPR_T Math_getLogicalAnd(MathState *state) {
    MATHEXPR_T value = 0;
    value = Math_getBitwiseOr(state);
    while (Math_look(state) == '&' && Math_peek(state) == '&') {
        Math_match(state, '&');
        Math_match(state, '&');
        if (value) {
            value = Math_getBitwiseOr(state);
        } else {
            Math_getBitwiseOr(state);
        }
    }
    return value;
}

/* Math_getLogicalOr
 * Evaluate a logical or expression
 */
MATHEXPR_T Math_getLogicalOr(MathState *state) {
    MATHEXPR_T value = 0;
    value = Math_getLogicalAnd(state);
    while (Math_look(state) == '|' && Math_peek(state) == '|') {
        Math_match(state, '|');
        Math_match(state, '|');
        if (value) {
            /* discard next */
            Math_getLogicalAnd(state);
        } else {
            value = Math_getLogicalAnd(state);
        }
    }
    return value;
}

/* Math_getTernary
 * Evaluate a ternary expression
 */
MATHEXPR_T Math_getTernary(MathState *state) {
    MATHEXPR_T value = 0;
    value = Math_getLogicalOr(state);
    while (Math_look(state) == '?') {
        Math_match(state, '?');
        if (value) {
            value = Math_getLogicalOr(state);
            Math_match(state, ':');
            Math_getLogicalOr(state);
        } else {
            Math_match(state, ':');
            value = Math_getLogicalOr(state);
        }
    }
    return value;
}

#ifdef MATHEXPR_VARIABLES
MATHEXPR_T Math_getAssignment(MathState *state) {
	MATHEXPR_T value = 0;
	int i;
	char *p;
	char tmp[32] = "";
	
	/* NOTE:
	 * This is really, really inefficient -- as it will copy the unevaluated string for anything,
	 * even if it isn't actually a variable.
	 */
	
	/* get unevaluated string in case it is a variable */
	p = state->p;
    value = Math_getTernary(state);

	if (state->p - p >= 0 || state->p - p <= 32) {
		strncpy(tmp, p, state->p - p);
		tmp[state->p - p] = '\0';
	}
	
	/* get rid of any trailing spaces */
	for (i = strlen(tmp); i > 0; i--) {
		if (isspace(tmp[i]))
			tmp[i] = '\0';
	}
	
    while (Math_look(state) == '=') {
        Math_match(state, '=');
		value = Math_getTernary(state);
		Math_setVariable(state, tmp, value);
    }
    return value;
}
#endif

/* Math_expression:
 * Evaluate an expression
 */
MATHEXPR_T Math_expression(MathState *state) {
    Math_skipSpace(state);
#ifdef MATHEXPR_VARIABLES
	return Math_getAssignment(state);
#else
    return Math_getTernary(state);
#endif
}

/* MathStat_init:
 * Initialise the MathState structure
 */
MATHEXPR_T MathState_init(MathState *state) {
	state->buffer = NULL;
	state->p = NULL;
#ifdef MATHEXPR_FUNCTIONS
    Math_clearFunctions(state);
    Math_registerStdFunctions(state);
 #ifdef MATHEXPR_VARIABLES
	Math_clearVariables(state);
 #endif
#endif
    return 1;
}

/* Math_eval:
 * Set up state and evaluate an expression, returning the result.
 */
MATHEXPR_T Math_eval(char *expr) {
	MathState state;
	MathState_init(&state);
	state.buffer = expr;
	state.p = expr;
    return Math_expression(&state);
}

/* Math_evalWithState
 * Evaluate an expression with predefined state, returning the result.
 */
MATHEXPR_T Math_evalWithState(MathState *state, char *expr) {
	state->buffer = expr;
	state->p = expr;
	return Math_expression(state);
}
