#ifndef VALIDATION_CONTEXT_H
#define VALIDATION_CONTEXT_H

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/type/CompilerState.h"
#include "../../support/logging/Logger.h"
#include <stdbool.h>

typedef struct SymbolTable SymbolTable;

typedef struct ValidationContext {
    CompilerState* compilerState; 
    Schema* currentSchema;
    Entity* currentEntity;
    bool hasErrors;
    int errorCount;
    int warningCount;
} ValidationContext;

#endif // VALIDATION_CONTEXT_H
