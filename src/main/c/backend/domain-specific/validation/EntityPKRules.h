#ifndef ENTITY_PK_RULES_H
#define ENTITY_PK_RULES_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../ValidationContext.h"
#include <stdbool.h>

/**
 * Validates the primary key definition of an entity.
 * Checks for:
 * - Inline vs explicit primary key conflicts
 * - Multiple inline primary keys
 * - Inheritance constraints (inherited entities cannot declare PKs)
 * - Empty or invalid primary key clauses
 * - Duplicate attributes in composite primary keys
 */
bool validateEntityPrimaryKey(ValidationContext *ctx, Entity *entity);

#endif // ENTITY_PK_RULES_H
