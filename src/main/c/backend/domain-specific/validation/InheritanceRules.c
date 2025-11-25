#include "InheritanceRules.h"
#include "../../../support/logging/Logger.h"
#include "../SymbolTable.h"
#include <string.h>


static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("InheritanceRules");
}

bool validateInheritanceParentExists(ValidationContext *ctx, Entity *entity) {
  initializeLogger();

  if (!entity->parent) {
    return true; // No parent, nothing to validate
  }

  SymbolTable *symbols = (SymbolTable *)ctx->compilerState->symbolTable;
  Entity *parentEntity = lookupEntity(symbols, entity->parent);

  if (!parentEntity) {
    logError(_logger,
             "Entity '%s' inherits from '%s' but parent entity does not exist",
             entity->name, entity->parent);
    ctx->hasErrors = true;
    return false;
  }

  return true;
}

bool validateInheritanceCycles(ValidationContext *ctx, Entity *entity) {
  initializeLogger();

  if (!entity->parent) {
    return true; // No parent, no cycle possible
  }

  SymbolTable *symbols = (SymbolTable *)ctx->compilerState->symbolTable;

  // Use a simple counter to detect cycles - if we traverse more entities
  // than exist in the schema, we must have a cycle
  const int MAX_INHERITANCE_DEPTH = 100;
  int depth = 0;

  const char *currentName = entity->parent;
  Entity *current = entity;

  while (currentName != NULL) {
    depth++;

    if (depth > MAX_INHERITANCE_DEPTH) {
      logError(_logger,
               "Inheritance cycle detected involving entity '%s' - inheritance "
               "chain is too deep or contains a cycle",
               entity->name);
      ctx->hasErrors = true;
      return false;
    }

    // Check if we've cycled back to the original entity
    if (strcmp(currentName, entity->name) == 0) {
      logError(_logger,
               "Inheritance cycle detected: entity '%s' inherits from itself "
               "(directly or indirectly)",
               entity->name);
      ctx->hasErrors = true;
      return false;
    }

    // Look up the parent entity
    current = lookupEntity(symbols, currentName);
    if (!current) {
      // Parent doesn't exist - (validated by validateInheritanceParentExists)
      break;
    }

    currentName = current->parent;
  }

  return true;
}

bool validateInheritanceAttributeRedefinition(ValidationContext *ctx,
                                              Entity *entity) {
  initializeLogger();

  if (!entity->parent) {
    return true; // No parent, no redefinition possible
  }

  SymbolTable *symbols = (SymbolTable *)ctx->compilerState->symbolTable;
  bool ok = true;

  // Collect all attribute names from the current entity
  for (AttributeList *attrList = entity->attributes; attrList != NULL;
       attrList = attrList->next) {
    Attribute *attr = attrList->attribute;
    if (!attr || !attr->name)
      continue;

    // Traverse the inheritance chain to check if this attribute exists in any
    // parent
    const char *parentName = entity->parent;
    while (parentName != NULL) {
      Entity *parentEntity = lookupEntity(symbols, parentName);
      if (!parentEntity) {
        // Parent doesn't exist - (validated by validateInheritanceParentExists)
        break;
      }

      // Check if the parent has an attribute with the same name
      Attribute *parentAttr = lookupAttributeInEntity(parentEntity, attr->name);
      if (parentAttr) {
        logError(_logger,
                 "Entity '%s' redefines attribute '%s' which is already "
                 "defined in parent entity '%s'",
                 entity->name, attr->name, parentEntity->name);
        ctx->hasErrors = true;
        ok = false;
        break; // No need to check further up the chain for this attribute
      }

      parentName = parentEntity->parent;
    }
  }

  return ok;
}
