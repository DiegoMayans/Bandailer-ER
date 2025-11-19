#include "SemanticAnalyzer.h"
#include "SymbolTable.h"
#include "ValidationContext.h"
#include "../../support/logging/Logger.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

static bool buildSymbolTable(ValidationContext* ctx, Schema* schema) {
    bool ok = true;
    SymbolTable* symbols = (SymbolTable*)ctx->compilerState->symbolTable;
    
    // Add all entities
    Entity* entity = schema->entities;
    while (entity) {
        if (!addEntity(symbols, entity->name, entity)) {
            logError(_logger, "Duplicate entity name: %s", entity->name);
            ok = false;
        }
        entity = entity->next;
    }
    
    // Add all relationships
    Relationship* rel = schema->relationships;
    while (rel) {
        if (!addRelationship(symbols, rel->name, rel)) {
            logError(_logger, "Duplicate relationship name: %s", rel->name);
            ok = false;
        }
        rel = rel->next;
    }
    
    return ok;
}

/** PUBLIC FUNCTIONS */

ComputationStatus executeSemanticAnalysis(CompilerState* state) {
    Program* program = state->abstractSyntaxtTree;
    if (!program || !program->schema) {
        logError(_logger, "No program to analyze");
        return SEMANTIC_ERROR;
    }
    
    if (!state->symbolTable) {
        state->symbolTable = createSymbolTable();
    }

        ValidationContext ctx = {
        .compilerState = state,
        .currentSchema = program->schema,
        .currentEntity = NULL,
        .hasErrors = false,
        .errorCount = 0,
        .warningCount = 0
    };

   bool ok = true;
    
    logInformation(_logger, "Starting semantic analysis...");
    
    logDebugging(_logger, "Pass 1: Building symbol table...");
    ok &= buildSymbolTable(&ctx, program->schema);
    // logDebugging(_logger, "Pass 2: Validating entities...");
    // ok &= validateEntities(&ctx);
    // logDebugging(_logger, "Pass 3: Validating relationships...");
    // ok &= validateRelationships(&ctx);
    // logDebugging(_logger, "Pass 4: Validating assertions...");
    // ok &= validateAssertions(&ctx);
    
    if (ok) {
        logInformation(_logger, "Semantic analysis completed successfully");
    } else {
        logError(_logger, "Semantic analysis failed with errors");
    }

    return ok ? SEMANTIC_OK : SEMANTIC_ERROR;
}

/** Shutdown module's internal state. */
static void _shutdownSemanticModule() {
    if (_logger != NULL) {
        logDebugging(_logger, "Destroying module: SemanticAnalyzer...");
        destroyLogger(_logger);
        _logger = NULL;
    }
}

/** Initialize module's internal state. */
ModuleDestructor initializeSemanticModule() {
    _logger = createLogger("SemanticAnalyzer");
    return _shutdownSemanticModule;
}
