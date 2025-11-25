#include "WeakEntityRules.h"
#include "../../../support/logging/Logger.h"
#include <string.h>

static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("WeakEntityRules");
}

bool validateWeakEntityRelationships(ValidationContext *ctx, Entity *entity) {
  initializeLogger();

  if (!entity->primaryKey || !entity->primaryKey->attributes) {
    // No explicit primary key, nothing to validate
    return true;
  }

  Schema *schema = ctx->currentSchema;
  if (!schema || !schema->relationships) {
    // No relationships defined in schema
    logError(_logger,
             "Weak entity '%s' references other entities in its primary key "
             "but no relationships are defined in the schema",
             entity->name);
    ctx->hasErrors = true;
    return false;
  }

  bool ok = true;

  // Collect all entities referenced in the primary key
  const int MAX_REFERENCED_ENTITIES = 32;
  char *referencedEntities[MAX_REFERENCED_ENTITIES];
  int refCount = 0;

  for (QualifiedIdentifierList *qidList = entity->primaryKey->attributes;
       qidList != NULL; qidList = qidList->next) {
    QualifiedIdentifier *qid = qidList->qid;
    if (!qid || !qid->entity)
      continue; // Skip if no entity qualifier (it's a self-reference)

    // Check if we already have this entity in our list
    bool alreadyAdded = false;
    for (int i = 0; i < refCount; i++) {
      if (strcmp(referencedEntities[i], qid->entity) == 0) {
        alreadyAdded = true;
        break;
      }
    }

    if (!alreadyAdded && refCount < MAX_REFERENCED_ENTITIES) {
      referencedEntities[refCount++] = qid->entity;
    }
  }

  // For each referenced entity, check if there's a relationship
  for (int i = 0; i < refCount; i++) {
    char *refEntityName = referencedEntities[i];
    bool hasRelationship = false;

    // Search through all relationships in the schema
    for (Relationship *rel = schema->relationships; rel != NULL;
         rel = rel->next) {
      if (!rel->participants)
        continue;

      bool hasWeakEntity = false;
      bool hasReferencedEntity = false;

      // Check if this relationship involves both the weak entity and the
      // referenced entity
      for (ParticipantList *pList = rel->participants; pList != NULL;
           pList = pList->next) {
        Participant *p = pList->participant;
        if (!p || !p->entityName)
          continue;

        if (strcmp(p->entityName, entity->name) == 0) {
          hasWeakEntity = true;
        }
        if (strcmp(p->entityName, refEntityName) == 0) {
          hasReferencedEntity = true;
        }
      }

      if (hasWeakEntity && hasReferencedEntity) {
        hasRelationship = true;
        break;
      }
    }

    if (!hasRelationship) {
      logError(_logger,
               "Weak entity '%s' references entity '%s' in its primary key but "
               "no relationship exists between them",
               entity->name, refEntityName);
      ctx->hasErrors = true;
      ok = false;
    }
  }

  return ok;
}
