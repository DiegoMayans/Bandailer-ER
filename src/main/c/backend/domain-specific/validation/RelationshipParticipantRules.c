#include "../ValidationConstants.h"
#include "RelationshipParticipantRules.h"
#include "../../../support/logging/Logger.h"
#include "../SymbolTable.h"
#include <string.h>

static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("RelationshipParticipantRules");
}

bool validateParticipantEntitiesExist(ValidationContext *ctx,
                                      Relationship *relationship) {
  initializeLogger();

  if (!relationship->participants) {
    return true; // Will be caught by minimum participants check
  }

  bool ok = true;
  SymbolTable *symbols = (SymbolTable *)ctx->compilerState->symbolTable;

  for (ParticipantList *pList = relationship->participants; pList != NULL;
       pList = pList->next) {
    Participant *p = pList->participant;
    if (!p || !p->entityName)
      continue;

    if (!hasEntity(symbols, p->entityName)) {
      logError(_logger,
               "In relationship '%s': referenced entity '%s' does not exist",
               relationship->name, p->entityName);
      ctx->hasErrors = true;
      ok = false;
    }
  }

  return ok;
}

bool validateUniqueParticipants(ValidationContext *ctx,
                                Relationship *relationship) {
  initializeLogger();

  if (!relationship->participants) {
    return true;
  }

  bool ok = true;
  const int MAX_PARTICIPANTS = MAX_RELATIONSHIP_PARTICIPANTS;
  char *seen[MAX_PARTICIPANTS];
  int count = 0;

  for (ParticipantList *pList = relationship->participants; pList != NULL;
       pList = pList->next) {
    Participant *p = pList->participant;
    if (!p || !p->entityName)
      continue;

    // Check if we've already seen this entity
    for (int i = 0; i < count; i++) {
      if (strcmp(seen[i], p->entityName) == 0) {
        logError(_logger,
                 "In relationship '%s': entity '%s' appears more than once - "
                 "each entity can only participate once",
                 relationship->name, p->entityName);
        ctx->hasErrors = true;
        ok = false;
        break;
      }
    }

    if (count < MAX_PARTICIPANTS) {
      seen[count++] = p->entityName;
    }
  }

  return ok;
}

bool validateMinimumParticipants(ValidationContext *ctx,
                                 Relationship *relationship) {
  initializeLogger();

  int participantCount = 0;

  for (ParticipantList *pList = relationship->participants; pList != NULL;
       pList = pList->next) {
    if (pList->participant && pList->participant->entityName) {
      participantCount++;
    }
  }

  if (participantCount < 2) {
    logError(_logger,
             "Relationship '%s' has only %d participant(s) - relationships "
             "must have at least 2 entities",
             relationship->name, participantCount);
    ctx->hasErrors = true;
    return false;
  }

  return true;
}

bool validateWeakEntityParticipation(ValidationContext *ctx,
                                     Relationship *relationship) {
  initializeLogger();

  if (!relationship->participants) {
    return true;
  }

  bool ok = true;
  SymbolTable *symbols = (SymbolTable *)ctx->compilerState->symbolTable;

  for (ParticipantList *pList = relationship->participants; pList != NULL;
       pList = pList->next) {
    Participant *p = pList->participant;
    if (!p || !p->entityName)
      continue;

    Entity *entity = lookupEntity(symbols, p->entityName);
    if (!entity)
      continue; // (validated by validateParticipantEntitiesExist)

    // If entity is weak, check that participation is total
    if (entity->weak) {
      if (!p->participation || p->participation->type != PARTICIPATION_TOTAL) {
        logError(_logger,
                 "In relationship '%s': weak entity '%s' must have TOTAL "
                 "participation",
                 relationship->name, p->entityName);
        ctx->hasErrors = true;
        ok = false;
      }
    }
  }

  return ok;
}
