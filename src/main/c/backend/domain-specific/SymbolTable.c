#include "SymbolTable.h"
#include <stdlib.h>
#include <string.h>

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
    EntitySymbol* entities;
    RelationshipSymbol* relationships;
};

SymbolTable* createSymbolTable(void) {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    table->entities = NULL;
    table->relationships = NULL;
    return table;
}

void destroySymbolTable(SymbolTable* table) {
    if (!table) return;
    
    EntitySymbol* e = table->entities;
    while (e) {
        EntitySymbol* next = e->next;
        free(e->name);
        free(e);
        e = next;
    }
    
    RelationshipSymbol* r = table->relationships;
    while (r) {
        RelationshipSymbol* next = r->next;
        free(r->name);
        free(r);
        r = next;
    }
    
    free(table);
}

bool addEntity(SymbolTable* table, const char* name, Entity* entity) {
    if (hasEntity(table, name)) {
        return false;
    }
    
    EntitySymbol* symbol = (EntitySymbol*)malloc(sizeof(EntitySymbol));
    symbol->name = strdup(name);
    symbol->entity = entity;
    symbol->next = table->entities;
    table->entities = symbol;
    return true;
}

Entity* lookupEntity(SymbolTable* table, const char* name) {
    EntitySymbol* current = table->entities;
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
    
    RelationshipSymbol* symbol = (RelationshipSymbol*)malloc(sizeof(RelationshipSymbol));
    symbol->name = strdup(name);
    symbol->relationship = rel;
    symbol->next = table->relationships;
    table->relationships = symbol;
    return true;
}

Relationship* lookupRelationship(SymbolTable* table, const char* name) {
    RelationshipSymbol* current = table->relationships;
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
    EntitySymbol* e = table->entities;
    while (e) {
        count++;
        e = e->next;
    }
    return count;
}

int getRelationshipCount(SymbolTable* table) {
    int count = 0;
    RelationshipSymbol* r = table->relationships;
    while (r) {
        count++;
        r = r->next;
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
