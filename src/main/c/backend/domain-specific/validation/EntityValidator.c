#include "EntityValidator.h"
#include "../../../support/logging/Logger.h"
#include "AttributeModifierRules.h"
#include "EntityPKRules.h"
#include "InheritanceRules.h"
#include "WeakEntityRules.h"
#include <string.h>

static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("EntityValidator");
}

static bool validateSingleEntity(ValidationContext *ctx, Entity *entity) {
  bool ok = true;

  // 1. Inheritance validations
  if (entity->parent) {
    ok &= validateInheritanceParentExists(ctx, entity);
    ok &= validateInheritanceCycles(ctx, entity);
    ok &= validateInheritanceAttributeRedefinition(ctx, entity);
  }

  // 2. Primary Key validations
  ok &= validateEntityPrimaryKey(ctx, entity);

  // 3. Weak Entity validations
  if (entity->weak) {
    ok &= validateWeakEntityRelationships(ctx, entity);
  }

  // 4. Attribute modifier type validations
  ok &= validateDefaultModifierTypes(ctx, entity);
  ok &= validatePrimaryModifierTypes(ctx, entity);

  return ok;
}

bool validateEntities(ValidationContext *ctx) {
  initializeLogger();

  if (!ctx || !ctx->currentSchema) {
    logError(_logger, "Validation context or current schema is missing.");
    return false;
  }

  bool ok = true;

  Schema *schema = ctx->currentSchema;
  for (Entity *ent = schema->entities; ent != NULL; ent = ent->next) {
    ctx->currentEntity = ent;
    ok &= validateSingleEntity(ctx, ent);
  }

  if (!ok) {
    ctx->hasErrors = true;
  }

  return ok;
}
