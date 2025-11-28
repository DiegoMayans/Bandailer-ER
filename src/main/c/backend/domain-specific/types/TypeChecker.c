#include "TypeChecker.h"
#include "../../../support/logging/Logger.h"
#include "../SymbolTable.h"
#include "TypeInference.h"

static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("TypeChecker");
}

TypeCheckStatus validateAllTypes(Program *program, SymbolTable *symbolTable) {
  initializeLogger();

  if (!program || !program->schema) {
    logError(_logger, "Program has no schema.");
    return TYPECHECK_ERROR;
  }

  bool hasErrors = false;
  Schema *schema = program->schema;

  // Validate expressions in entities
  for (Entity *e = schema->entities; e; e = e->next) {

    // Validate default expressions in modifiers
    for (AttributeList *attrList = e->attributes; attrList;
         attrList = attrList->next) {
      Attribute *a = attrList->attribute;

      for (ModifierList *modList = a->modifiers; modList;
           modList = modList->next) {
        Modifier *m = modList->modifier;
        if (m->type == MOD_DEFAULT && m->defaultValue) {
          TypeKind exprType = inferLiteralType(m->defaultValue);

          if (exprType == TYPE_ERROR || exprType != a->type->kind) {
            logError(_logger,
                     "Type mismatch in default value for attribute '%s'",
                     a->name);
            hasErrors = true;
          }
        }
      }
    }

    // Entity Assertions
    for (Assertion *as = e->assertions; as; as = as->next) {
      TypeKind condType = inferExpressionType(
          as->condition, createEntityScope(symbolTable, e));
      if (condType != TYPE_BOOL) {
        logError(_logger,
                 "Assertion condition must be boolean in entity '%s'",
                 e->name);
        hasErrors = true;
      }
    }
  }

  // Relationship attributes
  for (Relationship *r = schema->relationships; r; r = r->next) {
    for (AttributeList *attrList = r->attributes; attrList;
         attrList = attrList->next) {
      Attribute *a = attrList->attribute;
      for (ModifierList *modList = a->modifiers; modList;
           modList = modList->next) {
        Modifier *m = modList->modifier;
        if (m->type == MOD_DEFAULT && m->defaultValue) {
          TypeKind exprType =
              inferExpressionType((Expression *)m->defaultValue,
                                  createRelationshipScope(symbolTable, r));
          if (exprType == TYPE_ERROR || exprType != a->type->kind) {
            logError(_logger,
                     "Type mismatch in default value for relationship "
                     "attribute '%s'",
                     a->name);
            hasErrors = true;
          }
        }
      }
    }
  }

  return hasErrors ? TYPECHECK_ERROR : TYPECHECK_OK;
}
