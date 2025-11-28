#include "SymbolTable.h"
#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE 128

typedef struct EntitySymbol {
    char* name;
    Entity* entity;
    struct EntitySymbol* next;
} EntitySymbol;

typedef struct RelationshipSymbol {
    char* name;
    Relationship* relationship;
    struct RelationshipSymbol* next;
} RelationshipSymbol;

struct SymbolTable {
    EntitySymbol* entities[HASH_TABLE_SIZE];
    RelationshipSymbol* relationships[HASH_TABLE_SIZE];
};

/**
 * Hash function (djb2 algorithm)
 * 
 * https://gist.github.com/MohamedTaha98/ccdf734f13299efb73ff0b12f7ce429f
 */
static unsigned int hash(const char* str) {
    unsigned int hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    
    return hash % HASH_TABLE_SIZE;
}

SymbolTable* createSymbolTable(void) {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    
    /* Initialize all buckets to NULL */
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        table->entities[i] = NULL;
        table->relationships[i] = NULL;
    }
    
    return table;
}

void destroySymbolTable(SymbolTable* table) {
    if (!table) return;
    
    /* Free all entity buckets */
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        EntitySymbol* e = table->entities[i];
        while (e) {
            EntitySymbol* next = e->next;
            free(e->name);
            free(e);
            e = next;
        }
    }
    
    /* Free all relationship buckets */
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        RelationshipSymbol* r = table->relationships[i];
        while (r) {
            RelationshipSymbol* next = r->next;
            free(r->name);
            free(r);
            r = next;
        }
    }
    
    free(table);
}

bool addEntity(SymbolTable* table, const char* name, Entity* entity) {
    if (hasEntity(table, name)) {
        return false;
    }
    
    unsigned int bucket = hash(name);
    
    EntitySymbol* symbol = (EntitySymbol*)malloc(sizeof(EntitySymbol));
    symbol->name = strdup(name);
    symbol->entity = entity;
    symbol->next = table->entities[bucket];
    table->entities[bucket] = symbol;
    return true;
}

Entity* lookupEntity(SymbolTable* table, const char* name) {
    unsigned int bucket = hash(name);
    EntitySymbol* current = table->entities[bucket];
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->entity;
        }
        current = current->next;
    }
    return NULL;
}

bool hasEntity(SymbolTable* table, const char* name) {
    return lookupEntity(table, name) != NULL;
}

bool addRelationship(SymbolTable* table, const char* name, Relationship* rel) {
    if (hasRelationship(table, name)) {
        return false;
    }
    
    unsigned int bucket = hash(name);
    
    RelationshipSymbol* symbol = (RelationshipSymbol*)malloc(sizeof(RelationshipSymbol));
    symbol->name = strdup(name);
    symbol->relationship = rel;
    symbol->next = table->relationships[bucket];
    table->relationships[bucket] = symbol;
    return true;
}

Relationship* lookupRelationship(SymbolTable* table, const char* name) {
    unsigned int bucket = hash(name);
    RelationshipSymbol* current = table->relationships[bucket];
    
    while (current) {
        if (strcmp(current->name, name) == 0) {
            return current->relationship;
        }
        current = current->next;
    }
    return NULL;
}

bool hasRelationship(SymbolTable* table, const char* name) {
    return lookupRelationship(table, name) != NULL;
}

int getEntityCount(SymbolTable* table) {
    int count = 0;
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        EntitySymbol* e = table->entities[i];
        while (e) {
            count++;
            e = e->next;
        }
    }
    
    return count;
}

int getRelationshipCount(SymbolTable* table) {
    int count = 0;
    
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        RelationshipSymbol* r = table->relationships[i];
        while (r) {
            count++;
            r = r->next;
        }
    }
    
    return count;
}

/* Scope creation functions */
Scope createEntityScope(SymbolTable* table, Entity* entity) {
    Scope scope;
    scope.table = table;
    scope.entity = entity;
    scope.relationship = NULL;
    return scope;
}

Scope createRelationshipScope(SymbolTable* table, Relationship* relationship) {
    Scope scope;
    scope.table = table;
    scope.entity = NULL;
    scope.relationship = relationship;
    return scope;
}

/* Attribute lookup functions */
Attribute* lookupAttributeInEntity(Entity* entity, const char* name) {
    if (!entity || !entity->attributes) return NULL;
    
    AttributeList* current = entity->attributes;
    while (current) {
        if (current->attribute && strcmp(current->attribute->name, name) == 0) {
            return current->attribute;
        }
        current = current->next;
    }
    return NULL;
}

Attribute* lookupAttributeInRelationship(Relationship* relationship, const char* name) {
    if (!relationship || !relationship->attributes) return NULL;
    
    AttributeList* current = relationship->attributes;
    while (current) {
        if (current->attribute && strcmp(current->attribute->name, name) == 0) {
            return current->attribute;
        }
        current = current->next;
    }
    return NULL;
}
