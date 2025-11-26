#ifndef RELATIONSHIP_PARTICIPANT_RULES_H
#define RELATIONSHIP_PARTICIPANT_RULES_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../ValidationContext.h"
#include <stdbool.h>

// Validates that all participants in a relationship reference existing
// entities.
bool validateParticipantEntitiesExist(ValidationContext *ctx,
                                      Relationship *relationship);

// Validates that each entity appears only once in a relationship.
bool validateUniqueParticipants(ValidationContext *ctx,
                                Relationship *relationship);

// Validates that a relationship has at least 2 entities participating.
bool validateMinimumParticipants(ValidationContext *ctx,
                                 Relationship *relationship);

// Validates that if a weak entity participates in a relationship,
// its participation must be total.
bool validateWeakEntityParticipation(ValidationContext *ctx,
                                     Relationship *relationship);

#endif // RELATIONSHIP_PARTICIPANT_RULES_H
