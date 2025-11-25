#ifndef TYPE_INFERENCE_H
#define TYPE_INFERENCE_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../SymbolTable.h"
#include <stdbool.h>
#include <stddef.h>

#define TYPE_ERROR (-1)

/**
 * Infers the type of an expression within a given scope.
 * Returns the TypeKind of the expression or TYPE_ERROR if inference fails.
 */
TypeKind inferExpressionType(Expression *expr, Scope scope);

/**
 * Checks if a type is numeric (INTEGER or DECIMAL).
 */
bool isNumeric(TypeKind k);

/**
 * Checks if a type is boolean.
 */
bool isBoolean(TypeKind k);

/**
 * Checks if two types are comparable.
 * Same types are comparable, and numeric types are comparable with each other.
 */
bool isComparable(TypeKind left, TypeKind right);

/**
 * Coerces two numeric types to their common type.
 * Returns TYPE_ERROR if either type is not numeric.
 */
TypeKind coerceNumeric(TypeKind left, TypeKind right);

#endif // TYPE_INFERENCE_H
