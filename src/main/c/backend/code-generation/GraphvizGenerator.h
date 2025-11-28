#ifndef GRAPHVIZ_GENERATOR_HEADER
#define GRAPHVIZ_GENERATOR_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../domain-specific/SymbolTable.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>
#include <stdio.h>

/** Initialize module's internal state. */
ModuleDestructor initializeGraphvizGeneratorModule();

// Generates a Graphviz DOT file from the program AST.
bool generateGraphviz(Program *program, SymbolTable *symbolTable, const char *outputFilename);

// Generates an image from a Graphviz DOT file.
bool generateImageFromDot(const char *dotFilename, const char *outputFilename,
                          const char *format);

#endif
