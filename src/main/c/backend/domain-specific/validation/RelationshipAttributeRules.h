#ifndef RELATIONSHIP_ATTRIBUTE_RULES_H
#define RELATIONSHIP_ATTRIBUTE_RULES_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../ValidationContext.h"
#include <stdbool.h>

// Validates that attributes in a relationship don't have primary key modifier.
bool validateRelationshipAttributeModifiers(ValidationContext *ctx,
                                            Relationship *relationship);

#endif // RELATIONSHIP_ATTRIBUTE_RULES_H
