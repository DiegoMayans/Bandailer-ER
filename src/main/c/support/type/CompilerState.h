#ifndef COMPILER_STATE_HEADER
#define COMPILER_STATE_HEADER

/**
 * The global state of the compiler. Should transport every data structure
 * needed across the different phases of a compilation.
 */
typedef struct {
	/**
	 * The root node of the AST.
	 */
	void * abstractSyntaxtTree;

	/**
	 * Symbol table for entity and relationship lookup.
	 * Populated during semantic analysis, used by code generation.
	 */
	void * symbolTable;

	// TODO: Add a symbol table.
	// TODO: Add an stack to handle nested scopes.
	// TODO: Add more configuration.
	// TODO: Add whatever you need.
	// TODO: ...
} CompilerState;

#endif
