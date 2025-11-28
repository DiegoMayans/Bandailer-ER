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
      ctx->hasErrors = true;
      ok = false;
    }
    entity = entity->next;
  }

  // Add all relationships
  Relationship *rel = schema->relationships;
  while (rel) {
    if (!addRelationship(symbols, rel->name, rel)) {
      logError(_logger, "Duplicate relationship name: %s", rel->name);
      ctx->hasErrors = true;
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
                           .hasErrors = false,
                           .currentSchema = program->schema};

  if (!buildSymbolTable(&ctx, program->schema)) {
    return SEMANTIC_ERROR;
  }
  
  validateAllTypes(&ctx);
  validateEntities(&ctx);
  validateRelationships(&ctx);

  if (ctx.hasErrors) {
    logError(_logger, "Semantic analysis failed with errors");
    return SEMANTIC_ERROR;
  }

  return SEMANTIC_OK;
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
