#ifndef INHERITANCE_RULES_H
#define INHERITANCE_RULES_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../ValidationContext.h"
#include <stdbool.h>

/**
 * Validates that the parent entity specified in inheritance exists.
 */
bool validateInheritanceParentExists(ValidationContext *ctx, Entity *entity);

/**
 * Validates that there are no cycles in the inheritance chain.
 * Detects both direct and indirect cycles.
 */
bool validateInheritanceCycles(ValidationContext *ctx, Entity *entity);

/**
 * Validates that child entities do not redefine attributes from parent
 * entities. Checks the entire inheritance chain for attribute name conflicts.
 */
bool validateInheritanceAttributeRedefinition(ValidationContext *ctx,
                                              Entity *entity);

#endif // INHERITANCE_RULES_H
