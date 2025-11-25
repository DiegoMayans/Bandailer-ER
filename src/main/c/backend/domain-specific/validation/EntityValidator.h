#ifndef ENTITY_VALIDATOR_H
#define ENTITY_VALIDATOR_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../../support/type/ModuleDestructor.h"
#include "../SymbolTable.h"
#include "../ValidationContext.h"
#include <stdbool.h>

// Validates all entities in the validation context.
bool validateEntities(ValidationContext *ctx);

#endif // ENTITY_VALIDATOR_H
