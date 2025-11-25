#include "TypeInference.h"
#include "../../../support/logging/Logger.h"
#include "../SymbolTable.h"


static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("TypeInference");
}

static TypeKind inferLiteralType(Literal *lit);

bool isNumeric(TypeKind k) { return k == TYPE_INTEGER || k == TYPE_DECIMAL; }

bool isBoolean(TypeKind k) { return k == TYPE_BOOL; }

TypeKind coerceNumeric(TypeKind left, TypeKind right) {
  int rl = (left == TYPE_INTEGER ? 1 : left == TYPE_DECIMAL ? 2 : TYPE_ERROR);
  int rr = (right == TYPE_INTEGER ? 1 : right == TYPE_DECIMAL ? 2 : TYPE_ERROR);

  if (rl < 0 || rr < 0)
    return TYPE_ERROR;

  return (rl > rr) ? left : right;
}

bool isComparable(TypeKind L, TypeKind R) {
  if (L == R)
    return true;

  // numeric comparable
  if (isNumeric(L) && isNumeric(R))
    return true;

  return false;
}

static TypeKind inferLiteralType(Literal *lit) {
  if (!lit)
    return TYPE_ERROR;

  switch (lit->type) {
  case INTEGER_LITERAL:
    return TYPE_INTEGER;
  case DECIMAL_LITERAL:
    return TYPE_DECIMAL;
  case STRING_LITERAL:
    return TYPE_STRING;
  case BOOLEAN_LITERAL:
    return TYPE_BOOL;
  }
  return TYPE_ERROR;
}

TypeKind inferExpressionType(Expression *expr, Scope scope) {
  initializeLogger();

  if (!expr)
    return TYPE_ERROR;

  switch (expr->type) {

  case LITERAL_EXPR:
    return inferLiteralType(expr->literal);

  case IDENTIFIER_EXPR:
    if (scope.entity) {
      Attribute *a = lookupAttributeInEntity(scope.entity, expr->identifier);
      if (a)
        return a->type->kind;

      logError(_logger, "Unknown identifier '%s' in entity '%s'.",
               expr->identifier, scope.entity->name);
      return TYPE_ERROR;
    }

    if (scope.relationship) {
      Attribute *a =
          lookupAttributeInRelationship(scope.relationship, expr->identifier);
      if (a)
        return a->type->kind;

      logError(_logger, "Unknown identifier '%s' in relationship '%s'.",
               expr->identifier, scope.relationship->name);
      return TYPE_ERROR;
    }

    logError(_logger, "Identifier '%s' used in global scope (not allowed).",
             expr->identifier);
    return TYPE_ERROR;

  case ARITHMETIC_EXPR: {
    TypeKind L = inferExpressionType(expr->arithmetic.left, scope);
    TypeKind R = inferExpressionType(expr->arithmetic.right, scope);

    if (!isNumeric(L) || !isNumeric(R)) {
      logError(_logger, "Arithmetic operator with non-numeric operands.");
      return TYPE_ERROR;
    }

    return coerceNumeric(L, R);
  }

  case RELATIONAL_EXPR: {
    TypeKind L = inferExpressionType(expr->relational.left, scope);
    TypeKind R = inferExpressionType(expr->relational.right, scope);

    if (!isComparable(L, R)) {
      logError(_logger, "Relational operator with incompatible types.");
      return TYPE_ERROR;
    }

    return TYPE_BOOL;
  }

  case LOGICAL_EXPR: {
    TypeKind L = inferExpressionType(expr->logical.left, scope);
    TypeKind R = inferExpressionType(expr->logical.right, scope);

    if (!isBoolean(L) || !isBoolean(R)) {
      logError(_logger, "Logical operator requires booleans.");
      return TYPE_ERROR;
    }

    return TYPE_BOOL;
  }

  case LOGICAL_NOT_EXPR: {
    TypeKind T = inferExpressionType(expr->logicalNot.operand, scope);
    if (!isBoolean(T)) {
      logError(_logger, "NOT operator requires boolean operand.");
      return TYPE_ERROR;
    }
    return TYPE_BOOL;
  }

  case CONDITIONAL_EXPR: {
    TypeKind C = inferExpressionType(expr->conditional.condition, scope);
    if (!isBoolean(C)) {
      logError(_logger, "IF condition must be boolean.");
      return TYPE_ERROR;
    }

    TypeKind T1 = inferExpressionType(expr->conditional.thenExpr, scope);
    TypeKind T2 = inferExpressionType(expr->conditional.elseExpr, scope);

    if (T1 == T2)
      return T1;

    TypeKind Cc = coerceNumeric(T1, T2);
    if (Cc != TYPE_ERROR)
      return Cc;

    logError(_logger, "Conditional expression has incompatible branches.");
    return TYPE_ERROR;
  }

  case PARENTHESIZED_EXPR:
    return inferExpressionType(expr->parenthesized.expression, scope);
  }

  return TYPE_ERROR;
}
