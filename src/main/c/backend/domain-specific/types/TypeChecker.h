#ifndef TYPE_CHECKER_H
#define TYPE_CHECKER_H

#include "../../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../ValidationContext.h"
#include <stdbool.h>
#include <stddef.h>

/** Validates all expressions inside attributes, assertions, derivations, … */
void validateAllTypes(ValidationContext *ctx);

#endif
