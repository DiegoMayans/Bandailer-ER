#ifndef WEAK_ENTITY_RULES_H
#define WEAK_ENTITY_RULES_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../ValidationContext.h"
#include <stdbool.h>

/**
 * Validates that weak entities have identifying relationships.
 * For each entity referenced in the weak entity's primary key,
 * ensures a relationship exists between the weak entity and that entity.
 */
bool validateWeakEntityRelationships(ValidationContext *ctx, Entity *entity);

#endif // WEAK_ENTITY_RULES_H
