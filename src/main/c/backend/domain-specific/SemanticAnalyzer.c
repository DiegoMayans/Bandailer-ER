#include "SemanticAnalyzer.h"
#include "../../support/logging/Logger.h"
#include "SymbolTable.h"
#include "ValidationContext.h"
#include "types/TypeChecker.h"
#include "validation/EntityValidator.h"
#include "validation/RelationshipValidator.h"

/* MODULE INTERNAL STATE */

static Logger *_logger = NULL;

static bool buildSymbolTable(ValidationContext *ctx, Schema *schema) {
  bool ok = true;
  SymbolTable *symbols = (SymbolTable *)ctx->compilerState->symbolTable;

  // Add all entities
  Entity *entity = schema->entities;
  while (entity) {
    if (!addEntity(symbols, entity->name, entity)) {
      logError(_logger, "Duplicate entity name: %s", entity->name);
      ok = false;
    }
    entity = entity->next;
  }

  // Add all relationships
  Relationship *rel = schema->relationships;
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

ComputationStatus executeSemanticAnalysis(CompilerState *state) {
  Program *program = state->abstractSyntaxtTree;
  if (!program || !program->schema) {
    logError(_logger, "No program to analyze");
    return SEMANTIC_ERROR;
  }

  if (!state->symbolTable) {
    state->symbolTable = createSymbolTable();
  }

  ValidationContext ctx = {.compilerState = state,
                           .currentSchema = program->schema,
                           .currentEntity = NULL,
                           .hasErrors = false,
                           .errorCount = 0,
                           .warningCount = 0};

  bool ok = true;

  logInformation(_logger, "Starting semantic analysis...");

  logDebugging(_logger, "Pass 1: Building symbol table...");
  ok &= buildSymbolTable(&ctx, program->schema);

  // Add type checking pass
  logDebugging(_logger, "Pass 2: Type checking...");
  TypeCheckStatus typeCheckResult =
      validateAllTypes(program, (SymbolTable *)state->symbolTable);
  ok &= (typeCheckResult == TYPECHECK_OK);

  // Add entity validation pass
  logDebugging(_logger, "Pass 3: Entity validation...");
  ok &= validateEntities(&ctx);

  // Add relationship validation pass
  logDebugging(_logger, "Pass 4: Relationship validation...");
  ok &= validateRelationships(&ctx);

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
