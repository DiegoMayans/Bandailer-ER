#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../../frontend/syntactic-analysis/AbstractSyntaxTree.h"
#include <stdbool.h>

typedef struct SymbolTable SymbolTable;

typedef struct {
    SymbolTable* table;
    Entity* entity;         
    Relationship* relationship; 
} Scope;

/* Creation and destruction */
SymbolTable* createSymbolTable(void);
void destroySymbolTable(SymbolTable* table);

/* Scope operations */
Scope createEntityScope(SymbolTable* table, Entity* entity);
Scope createRelationshipScope(SymbolTable* table, Relationship* relationship);

/* Entity operations */
bool addEntity(SymbolTable* table, const char* name, Entity* entity);
Entity* lookupEntity(SymbolTable* table, const char* name);
bool hasEntity(SymbolTable* table, const char* name);

/* Relationship operations */
bool addRelationship(SymbolTable* table, const char* name, Relationship* rel);
Relationship* lookupRelationship(SymbolTable* table, const char* name);
bool hasRelationship(SymbolTable* table, const char* name);

/* Attribute lookup within scope */
Attribute* lookupAttributeInEntity(Entity* entity, const char* name);
Attribute* lookupAttributeInRelationship(Relationship* relationship, const char* name);

/* Utility functions */
int getEntityCount(SymbolTable* table);
int getRelationshipCount(SymbolTable* table);

#endif // SYMBOL_TABLE_H
