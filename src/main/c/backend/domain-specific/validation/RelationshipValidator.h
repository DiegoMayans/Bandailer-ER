#ifndef RELATIONSHIP_VALIDATOR_H
#define RELATIONSHIP_VALIDATOR_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../../support/type/ModuleDestructor.h"
#include "../SymbolTable.h"
#include "../ValidationContext.h"
#include <stdbool.h>

// Validates all relationships in the validation context.
bool validateRelationships(ValidationContext *ctx);

#endif // RELATIONSHIP_VALIDATOR_H
