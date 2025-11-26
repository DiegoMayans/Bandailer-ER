#include "RelationshipValidator.h"
#include "../../../support/logging/Logger.h"
#include "RelationshipAttributeRules.h"
#include "RelationshipParticipantRules.h"
#include <string.h>


static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("RelationshipValidator");
}

static bool validateSingleRelationship(ValidationContext *ctx,
                                       Relationship *relationship) {
  bool ok = true;

  // 1. Participant validations
  ok &= validateParticipantEntitiesExist(ctx, relationship);
  ok &= validateUniqueParticipants(ctx, relationship);
  ok &= validateMinimumParticipants(ctx, relationship);
  ok &= validateWeakEntityParticipation(ctx, relationship);

  // 2. Attribute validations
  ok &= validateRelationshipAttributeModifiers(ctx, relationship);

  return ok;
}

bool validateRelationships(ValidationContext *ctx) {
  initializeLogger();

  if (!ctx || !ctx->currentSchema) {
    logError(_logger, "Validation context or current schema is missing.");
    return false;
  }

  bool ok = true;

  Schema *schema = ctx->currentSchema;
  for (Relationship *rel = schema->relationships; rel != NULL;
       rel = rel->next) {
    ok &= validateSingleRelationship(ctx, rel);
  }

  return ok;
}
