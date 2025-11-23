#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include <stdbool.h>
#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "SymbolTable.h"

#define TYPE_ERROR (-1)

typedef enum {
    TYPECHECK_OK,
    TYPECHECK_ERROR
} TypeCheckStatus;

/** Returns the TypeKind of an expression or TYPE_ERROR. */
TypeKind inferExpressionType(Expression* expr, Scope scope);

/** Validates all expressions inside attributes, assertions, derivations, … */
TypeCheckStatus validateAllTypes(Program* program, SymbolTable* symbolTable);

#endif
