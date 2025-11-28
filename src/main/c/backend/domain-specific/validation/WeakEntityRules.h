#ifndef WEAK_ENTITY_RULES_H
#define WEAK_ENTITY_RULES_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../ValidationContext.h"
#include <stdbool.h>

// Validates that weak entities have identifying relationships.
bool validateWeakEntityRelationships(ValidationContext *ctx, Entity *entity);

#endif // WEAK_ENTITY_RULES_H
