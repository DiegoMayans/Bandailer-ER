#ifndef GRAPHVIZ_GENERATOR_HEADER
#define GRAPHVIZ_GENERATOR_HEADER

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include <stdbool.h>
#include <stdio.h>

/** Initialize module's internal state. */
ModuleDestructor initializeGraphvizGeneratorModule();

/**
 * Generates a Graphviz DOT file from the program AST.
 *
 * @param program The program AST containing schemas, entities, and
 * relationships
 * @param outputFilename The path to the output .dot file
 * @return true if generation succeeded, false otherwise
 */
bool generateGraphviz(Program *program, const char *outputFilename);

#endif
