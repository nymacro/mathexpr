/*
 * mathexpr -- Mathematical Expression Evaluator
 * Copyright (C) 2008-2015 Aaron Marks
 */
#ifndef MATHEXPR_MATHEXPR_H
#define MATHEXPR_MATHEXPR_H

typedef int MATHEXPR_T;

#define MATHEXPR_FUNCTIONS
#define MATHEXPR_VARIABLES

#ifdef MATHEXPR_FUNCTIONS
#define MATHEXPR_FUNCTIONS_MAX		32
#define MATHEXPR_NAME_MAX			32
typedef MATHEXPR_T (*MathExprFunction)(int argc, MATHEXPR_T argv[]);

struct MathFunction {
    char name[MATHEXPR_NAME_MAX];
    MathExprFunction func;
};
typedef struct MathFunction MathFunction;

 #ifdef MATHEXPR_VARIABLES
 #define MATHEXPR_VARIABLES_MAX		32

struct MathVariable {
	char name[MATHEXPR_NAME_MAX];
	MATHEXPR_T value;
};
typedef struct MathVariable MathVariable;

 #endif

#endif

struct MathState {
#ifdef MATHEXPR_FUNCTIONS
    int functionCount;
    MathFunction functions[MATHEXPR_FUNCTIONS_MAX];
 #ifdef MATHEXPR_VARIABLES
	int variableCount;
	MathVariable variables[MATHEXPR_VARIABLES_MAX];
 #endif
#endif
    char *buffer;   /* expression buffer */
    char *p;        /* current string pointer */
};
typedef struct MathState MathState;

MATHEXPR_T MathState_init(MathState *state);
MATHEXPR_T Math_eval(char *expr);
MATHEXPR_T Math_evalWithState(MathState *state, char *expr);

#ifdef MATHEXPR_FUNCTIONS
 #ifdef MATHEXPR_VARIABLES
void Math_addVariable(MathState *state, char *name, MATHEXPR_T value);
void Math_setVariable(MathState *state, char *name, MATHEXPR_T value);
void Math_clearVariables(MathState *state);
 #endif
void Math_addFunction(MathState *state, char *name, MathExprFunction func);
#endif

#endif

