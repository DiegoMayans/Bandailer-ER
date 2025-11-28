#include "../ValidationConstants.h"
#include "EntityPKRules.h"
#include "../../../support/logging/Logger.h"
#include "../SymbolTable.h"
#include <string.h>


static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("EntityPKRules");
}

bool validatePKIdentifier(ValidationContext *ctx, Entity *entity,
                          QualifiedIdentifier *qid) {
  initializeLogger();

  const char *targetEntityName = qid->entity ? qid->entity : entity->name;
  const char *attributeName = qid->attribute;

  SymbolTable *symbols = (SymbolTable *)ctx->compilerState->symbolTable;

  Entity *targetEntity = NULL;

  if (strcmp(targetEntityName, entity->name) == 0) { // Self-reference
    targetEntity = entity;
  } else { // Validate referenced entity exists

    // Non-weak entities cannot reference attributes from other entities
    if (!entity->weak) {
      logError(
          _logger,
          "Non-weak entity '%s' cannot reference attributes from other "
          "entities ('%s') in PRIMARY KEY - only weak entities can do this",
          entity->name, targetEntityName);
      ctx->hasErrors = true;
      return false;
    }

    targetEntity = lookupEntity(symbols, targetEntityName);
    if (!targetEntity) {
      logError(_logger,
               "In PRIMARY KEY of entity '%s': referenced entity '%s' does not "
               "exist",
               entity->name, targetEntityName);
      ctx->hasErrors = true;
      return false;
    }
  }

  Attribute *attr = lookupAttributeInEntity(targetEntity, attributeName);

  if (!attr) {
    logError(_logger,
             "In PRIMARY KEY of entity '%s': attribute '%s' does not exist in "
             "entity '%s'",
             entity->name, attributeName, targetEntityName);
    ctx->hasErrors = true;
    return false;
  }

  // Validate referenced attribute is not derived
  for (ModifierList *list = attr->modifiers; list != NULL; list = list->next) {
    if (list->modifier->type == MOD_DERIVED) {
      logError(_logger,
               "Derived attribute '%s.%s' cannot be part of a PRIMARY KEY",
               targetEntityName, qid->attribute);
      ctx->hasErrors = true;
      return false;
    }
  }

  return true;
}

bool validateEntityPrimaryKey(ValidationContext *ctx, Entity *entity) {
  initializeLogger();

  PrimaryKey *pk = entity->primaryKey;

  // Check for inline primary modifiers and count them
  int inlinePrimaryCount = 0;
  for (AttributeList *attrList = entity->attributes; attrList != NULL;
       attrList = attrList->next) {
    Attribute *attr = attrList->attribute;
    if (!attr)
      continue;

    for (ModifierList *modList = attr->modifiers; modList != NULL;
         modList = modList->next) {
      if (modList->modifier && modList->modifier->type == MOD_PRIMARY) {
        inlinePrimaryCount++;
        break; // Only count once per attribute
      }
    }
  }

  bool hasInlinePrimary = inlinePrimaryCount > 0;

  // Entities with inheritance cannot declare primary keys
  if (entity->parent != NULL) {
    if (pk || hasInlinePrimary) {
      logError(_logger,
               "Entity '%s' inherits from '%s' and cannot declare its own "
               "primary key - it inherits the primary key from its parent",
               entity->name, entity->parent);
      ctx->hasErrors = true;
      return false;
    }
    // Entity with inheritance and no primary key declaration is valid
    return true;
  }

  // Check for conflicting primary key definitions
  if (pk && hasInlinePrimary) {
    logError(_logger,
             "Entity '%s' has both explicit PRIMARY KEY clause and inline "
             "primary modifier - only one method is allowed",
             entity->name);
    ctx->hasErrors = true;
    return false;
  }

  // Check for multiple inline primary keys
  if (inlinePrimaryCount > 1) {
    logError(_logger,
             "Entity '%s' has %d attributes with 'primary' modifier - only one "
             "is allowed for inline primary keys",
             entity->name, inlinePrimaryCount);
    ctx->hasErrors = true;
    return false;
  }

  // If no explicit primary key and no inline primary key
  if (!pk && !hasInlinePrimary) {
    logError(_logger, "Entity '%s' has no primary key defined", entity->name);
    ctx->hasErrors = true;
    return false;
  }

  // If only inline primary key, validation is complete
  if (!pk && hasInlinePrimary) {
    return true;
  }

  // Validate explicit primary key
  QualifiedIdentifierList *qids = pk->attributes;
  if (!qids) {
    logError(_logger, "Entity '%s' has an empty PRIMARY KEY clause",
             entity->name);
    ctx->hasErrors = true;
    return false;
  }

  bool ok = true;

  const int MAX_PK_ATTRS = MAX_PRIMARY_KEY_ATTRIBUTES;
  char *seen[MAX_PK_ATTRS];
  int count = 0;

  for (QualifiedIdentifierList *it = qids; it != NULL; it = it->next) {
    QualifiedIdentifier *qid = it->qid;
    if (!qid)
      continue;

    ok &= validatePKIdentifier(ctx, entity, qid);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%s.%s",
             qid->entity ? qid->entity : entity->name, qid->attribute);

    for (int i = 0; i < count; i++) {
      if (strcmp(buffer, seen[i]) == 0) {
        logError(_logger,
                 "Duplicate attribute '%s' in PRIMARY KEY of entity '%s'",
                 buffer, entity->name);
        ctx->hasErrors = true;
        ok = false;
      }
    }

    seen[count] = strdup(buffer);
    count++;
  }

  for (int i = 0; i < count; i++)
    free(seen[i]);

  return ok;
}
