#include "EntityValidator.h"
#include "../../../support/logging/Logger.h"
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

  // 1. Validaciones de Jerarquía (Primero, porque define atributos heredados)
  if (entity->parent) {
    ok &= validateInheritanceParentExists(ctx, entity);
    ok &= validateInheritanceCycles(ctx, entity);
    ok &= validateInheritanceAttributeRedefinition(ctx, entity);
  }

  // 2. Validaciones de Claves Primarias
  ok &= validateEntityPrimaryKey(ctx, entity);

  // 3. Validaciones de Entidades Débiles
  if (entity->weak) {
    ok &= validateWeakEntityRelationships(ctx, entity);
  }

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

  return ok;
}
