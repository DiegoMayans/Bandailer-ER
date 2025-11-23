#include "TypeChecker.h"
#include "SymbolTable.h"
#include "../../support/logging/Logger.h"

static Logger* _logger = NULL;

static void initializeLogger() {
    if (!_logger)
        _logger = createLogger("TypeChecker");
}

static TypeKind infer(Expression* expr, Scope scope);
static bool isNumeric(TypeKind k);
static bool isComparable(TypeKind left, TypeKind right);
static bool isBoolean(TypeKind k);
static TypeKind coerceNumeric(TypeKind left, TypeKind right);
static TypeKind inferLiteralType(Literal* lit);

TypeCheckStatus validateAllTypes(Program* program, SymbolTable* symbolTable) {
    initializeLogger();

    if (!program || !program->schema) {
        logError(_logger, "Program has no schemas.");
        return TYPECHECK_ERROR;
    }

    bool hasErrors = false;

    // Validate expressions in entities
    for (Schema* s = program->schema; s; s = s->next) {
        for (Entity* e = s->entities; e; e = e->next) {
            
            // Validate default expressions in modifiers
            for (AttributeList* attrList = e->attributes; attrList; attrList = attrList->next) {
                Attribute* a = attrList->attribute;

                for (ModifierList* modList = a->modifiers; modList; modList = modList->next) {
                    Modifier* m = modList->modifier;
                    if (m->type == MOD_DEFAULT && m->defaultValue) {
                        TypeKind exprType = inferLiteralType(m->defaultValue);

                        if (exprType == TYPE_ERROR || exprType != a->type->kind) {
                            logError(_logger, "Type mismatch in default value for attribute '%s'", a->name);
                            hasErrors = true;
                        }
                    }
                }
            }            

            // Entity Assertions
            for (Assertion* as = e->assertions; as; as = as->next) {
                TypeKind condType = inferExpressionType(as->condition, createEntityScope(symbolTable, e));
                if (condType != TYPE_BOOL) {
                    logError(_logger, "Assertion condition must be boolean in entity '%s'", e->name);
                    hasErrors = true;
                }
            }
        }

        // Relationship attributes
        for (Relationship* r = s->relationships; r; r = r->next) {
            for (AttributeList* attrList = r->attributes; attrList; attrList = attrList->next) {
                Attribute* a = attrList->attribute;
                for (ModifierList* modList = a->modifiers; modList; modList = modList->next) {
                    Modifier* m = modList->modifier;
                    if (m->type == MOD_DEFAULT && m->defaultValue) {
                        TypeKind exprType = inferExpressionType((Expression*)m->defaultValue, createRelationshipScope(symbolTable, r));
                        if (exprType == TYPE_ERROR || exprType != a->type->kind) {
                            logError(_logger, "Type mismatch in default value for relationship attribute '%s'", a->name);
                            hasErrors = true;
                        }
                    }
                }
            }

            // TODO: Implementar asserts en relaciones
            // for (Assertion* as = r->assertions; as; as = as->next) {
            //     TypeKind condType = inferExpressionType(as->condition, createRelationshipScope(symbolTable, r));
            //     if (condType != TYPE_BOOL) {
            //         logError(_logger, "Assertion condition must be boolean in relationship '%s'", r->name);
            //         hasErrors = true;
            //     }
            // }
        }
    }

    return hasErrors ? TYPECHECK_ERROR : TYPECHECK_OK;
}

TypeKind inferExpressionType(Expression* expr, Scope scope) {
    initializeLogger();
    return infer(expr, scope);
}

/** Helpers */

static bool isNumeric(TypeKind k) {
    return k == TYPE_INTEGER || k == TYPE_DECIMAL;
}

static bool isBoolean(TypeKind k) {
    return k == TYPE_BOOL;
}

static TypeKind coerceNumeric(TypeKind left, TypeKind right) {
    int rl = (left == TYPE_INTEGER ? 1 : left == TYPE_DECIMAL ? 2 : TYPE_ERROR);
    int rr = (right == TYPE_INTEGER ? 1 : right == TYPE_DECIMAL ? 2 : TYPE_ERROR);

    if (rl < 0 || rr < 0)
        return TYPE_ERROR;

    return (rl > rr) ? left : right;
}

/** Inference */

static TypeKind infer(Expression* expr, Scope scope) {
    if (!expr) return TYPE_ERROR;

    switch (expr->type) {

    case LITERAL_EXPR:
        return inferLiteralType(expr->literal);

    case IDENTIFIER_EXPR:
        if (scope.entity) {
            Attribute* a = lookupAttributeInEntity(scope.entity, expr->identifier);
            if (a) return a->type->kind;

            logError(_logger, "Unknown identifier '%s' in entity '%s'.",
                     expr->identifier, scope.entity->name);
            return TYPE_ERROR;
        }

        if (scope.relationship) {
            Attribute* a = lookupAttributeInRelationship(scope.relationship, expr->identifier);
            if (a) return a->type->kind;

            logError(_logger, "Unknown identifier '%s' in relationship '%s'.",
                     expr->identifier, scope.relationship->name);
            return TYPE_ERROR;
        }

        logError(_logger, "Identifier '%s' used in global scope (not allowed).",
                 expr->identifier);
        return TYPE_ERROR;

    case ARITHMETIC_EXPR: {
        TypeKind L = infer(expr->arithmetic.left, scope);
        TypeKind R = infer(expr->arithmetic.right, scope);

        if (!isNumeric(L) || !isNumeric(R)) {
            logError(_logger, "Arithmetic operator with non-numeric operands.");
            return TYPE_ERROR;
        }

        return coerceNumeric(L, R);
    }

    case RELATIONAL_EXPR: {
        TypeKind L = infer(expr->relational.left, scope);
        TypeKind R = infer(expr->relational.right, scope);

        if (!isComparable(L, R)) {
            logError(_logger, "Relational operator with incompatible types.");
            return TYPE_ERROR;
        }

        return TYPE_BOOL;
    }

    case LOGICAL_EXPR: {
        TypeKind L = infer(expr->logical.left, scope);
        TypeKind R = infer(expr->logical.right, scope);

        if (!isBoolean(L) || !isBoolean(R)) {
            logError(_logger, "Logical operator requires booleans.");
            return TYPE_ERROR;
        }

        return TYPE_BOOL;
    }

    case LOGICAL_NOT_EXPR: {
        TypeKind T = infer(expr->logicalNot.operand, scope);
        if (!isBoolean(T)) {
            logError(_logger, "NOT operator requires boolean operand.");
            return TYPE_ERROR;
        }
        return TYPE_BOOL;
    }

    case CONDITIONAL_EXPR: {
        TypeKind C = infer(expr->conditional.condition, scope);
        if (!isBoolean(C)) {
            logError(_logger, "IF condition must be boolean.");
            return TYPE_ERROR;
        }

        TypeKind T1 = infer(expr->conditional.thenExpr, scope);
        TypeKind T2 = infer(expr->conditional.elseExpr, scope);

        if (T1 == T2) return T1;

        TypeKind Cc = coerceNumeric(T1, T2);
        if (Cc != TYPE_ERROR) return Cc;

        logError(_logger, "Conditional expression has incompatible branches.");
        return TYPE_ERROR;
    }

    case PARENTHESIZED_EXPR:
        return infer(expr->parenthesized.expression, scope);
    }

    return TYPE_ERROR;
}

static TypeKind inferLiteralType(Literal* lit) {
    if (!lit) return TYPE_ERROR;

    switch (lit->type) {
        case INTEGER_LITERAL:  return TYPE_INTEGER;
        case DECIMAL_LITERAL:  return TYPE_DECIMAL;
        case STRING_LITERAL:   return TYPE_STRING;
        case BOOLEAN_LITERAL:  return TYPE_BOOL;
    }
    return TYPE_ERROR;
}


/** Comparable */

static bool isComparable(TypeKind L, TypeKind R) {
    if (L == R) return true;

    // numeric comparable
    if (isNumeric(L) && isNumeric(R)) return true;

    return false;
}
