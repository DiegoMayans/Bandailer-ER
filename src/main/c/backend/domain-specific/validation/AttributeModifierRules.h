#ifndef ATTRIBUTE_MODIFIER_RULES_H
#define ATTRIBUTE_MODIFIER_RULES_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../ValidationContext.h"
#include <stdbool.h>

// Validates that the types of attributes with DEFAULT modifiers are compatible
bool validateDefaultModifierTypes(ValidationContext *ctx, Entity *entity);

// Validates that the types of attributes with PRIMARY modifiers are compatible
bool validatePrimaryModifierTypes(ValidationContext *ctx, Entity *entity);

#endif // ATTRIBUTE_MODIFIER_RULES_H
