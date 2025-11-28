#ifndef ENTITY_PK_RULES_H
#define ENTITY_PK_RULES_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../ValidationContext.h"
#include <stdbool.h>

// Validates the primary key definition of an entity.
bool validateEntityPrimaryKey(ValidationContext *ctx, Entity *entity);

#endif // ENTITY_PK_RULES_H
