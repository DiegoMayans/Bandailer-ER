#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../SymbolTable.h"
#include <stdbool.h>
#include <stddef.h>


#define TYPE_ERROR (-1)

typedef enum { TYPECHECK_OK, TYPECHECK_ERROR } TypeCheckStatus;

/** Validates all expressions inside attributes, assertions, derivations, … */
TypeCheckStatus validateAllTypes(Program *program, SymbolTable *symbolTable);

#endif
