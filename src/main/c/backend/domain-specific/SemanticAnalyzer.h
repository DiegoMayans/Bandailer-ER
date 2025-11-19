#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"

/** Initialize module's internal state. */
ModuleDestructor initializeSemanticModule();

typedef enum {
    SEMANTIC_OK,
    SEMANTIC_ERROR
} ComputationStatus;

/** Computes the program value using the current compiler state. */
ComputationStatus executeSemanticAnalysis(CompilerState* state);

#endif // SEMANTIC_ANALYZER_H
