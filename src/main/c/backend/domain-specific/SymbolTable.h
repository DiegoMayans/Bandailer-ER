#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <stdbool.h>

typedef struct SymbolTable SymbolTable;

/* Creation and destruction */
SymbolTable* createSymbolTable(void);
void destroySymbolTable(SymbolTable* table);

/* Entity operations */
bool addEntity(SymbolTable* table, const char* name, Entity* entity);
Entity* lookupEntity(SymbolTable* table, const char* name);
bool hasEntity(SymbolTable* table, const char* name);

/* Relationship operations */
bool addRelationship(SymbolTable* table, const char* name, Relationship* rel);
Relationship* lookupRelationship(SymbolTable* table, const char* name);
bool hasRelationship(SymbolTable* table, const char* name);

/* Utility functions */
int getEntityCount(SymbolTable* table);
int getRelationshipCount(SymbolTable* table);

#endif // SYMBOL_TABLE_H
