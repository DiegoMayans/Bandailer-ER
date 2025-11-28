#ifndef ABSTRACT_SYNTAX_TREE_HEADER
#define ABSTRACT_SYNTAX_TREE_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include <stdbool.h>
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeAbstractSyntaxTreeModule();

/**
 * Forward declarations for self-referencing types
 */
typedef enum ExpressionType ExpressionType;
typedef enum LiteralType LiteralType;
typedef enum TypeKind TypeKind;
typedef enum ModifierType ModifierType;
typedef enum ArithmeticOperator ArithmeticOperator;
typedef enum RelationalOperator RelationalOperator;
typedef enum LogicalOperator LogicalOperator;

typedef struct Program Program;
typedef struct Schema Schema;
typedef struct Entity Entity;
typedef struct Relationship Relationship;
typedef struct Attribute Attribute;
typedef struct AttributeList AttributeList;
typedef struct RelationshipList RelationshipList;
typedef struct Participant Participant;
typedef struct ParticipantList ParticipantList;
typedef struct Expression Expression;
typedef struct Literal Literal;
typedef struct Type Type;
typedef struct Modifier Modifier;
typedef struct ModifierList ModifierList;
typedef struct PrimaryKey PrimaryKey;
typedef struct Assertion Assertion;
typedef struct Participation Participation;
typedef struct IdentifierList IdentifierList;
typedef struct QualifiedIdentifier QualifiedIdentifier;
typedef struct QualifiedIdentifierList QualifiedIdentifierList;

/**
 * Enumeration types
 */
enum ExpressionType {
  LITERAL_EXPR,
  IDENTIFIER_EXPR,
  ARITHMETIC_EXPR,
  RELATIONAL_EXPR,
  LOGICAL_EXPR,
  LOGICAL_NOT_EXPR,
  CONDITIONAL_EXPR,
  PARENTHESIZED_EXPR
};

enum LiteralType {
  INTEGER_LITERAL,
  DECIMAL_LITERAL,
  STRING_LITERAL,
  BOOLEAN_LITERAL
};

enum TypeKind {
  TYPE_INTEGER,
  TYPE_DECIMAL,
  TYPE_STRING,
  TYPE_BOOL,
  TYPE_DATE,
  TYPE_DATETIME,
  TYPE_UUID,
  TYPE_ENUM
};

enum ModifierType {
  MOD_PRIMARY,
  MOD_UNIQUE,
  MOD_NOT_NULL,
  MOD_DEFAULT,
  MOD_DERIVED
};

enum ArithmeticOperator { ADDITION, SUBTRACTION, MULTIPLICATION, DIVISION };

enum RelationalOperator {
  EQUAL,
  NOT_EQUAL,
  GREATER,
  LESS,
  GREATER_EQUAL,
  LESS_EQUAL
};

enum LogicalOperator { AND, OR };

typedef enum CardinalityType {
  ONE,
  MANY,
} CardinalityType;

typedef enum ParticipationType {
  PARTICIPATION_TOTAL,
  PARTICIPATION_PARTIAL
} ParticipationType;

/**
 * AST Node structures
 */
struct Program {
  Schema *schema;
};

struct Schema {
  char *name;
  Entity *entities;
  Entity *entities_last;
  Relationship *relationships;
  Relationship *relationships_last;
};

struct Entity {
  char *name;
  char *parent; // For inheritance
  bool weak;
  AttributeList *attributes;
  AttributeList *attributes_last;
  PrimaryKey *primaryKey;
  Assertion *assertions;
  Assertion *assertions_last;
  Entity *next;
};

struct Relationship {
  char *name;
  ParticipantList *participants;
  ParticipantList *participants_last;
  AttributeList *attributes;
  AttributeList *attributes_last;
  Relationship *next;
};

struct Attribute {
  char *name;
  Type *type;
  ModifierList *modifiers;
  Attribute *next;
};

struct AttributeList {
  Attribute *attribute;
  AttributeList *next;
  AttributeList *last;  // Tail pointer for O(1) append
};

struct RelationshipList {
  Relationship *relationship;
  RelationshipList *next;
};

struct Participant {
  char *entityName;
  CardinalityType cardinality;
  Participation *participation;
  Participant *next;
};

struct ParticipantList {
  Participant *participant;
  ParticipantList *next;
  ParticipantList *last;  // Tail pointer for O(1) append
};

struct Expression {
  ExpressionType type;
  union {
    Literal *literal;
    char *identifier;
    struct {
      Expression *left;
      Expression *right;
      ArithmeticOperator arithmeticOp;
    } arithmetic;
    struct {
      Expression *left;
      Expression *right;
      RelationalOperator relationalOp;
    } relational;
    struct {
      Expression *left;
      Expression *right;
      LogicalOperator logicalOp;
    } logical;
    struct {
      Expression *operand;
    } logicalNot;
    struct {
      Expression *condition;
      Expression *thenExpr;
      Expression *elseExpr;
    } conditional;
    struct {
      Expression *expression;
    } parenthesized;
  };
};

struct Literal {
  LiteralType type;
  union {
    int integer;
    double decimal;
    char *string;
    bool boolean;
  };
};

struct Type {
  TypeKind kind;
  bool isArray;
};

struct Modifier {
  ModifierType type;
  Literal *defaultValue; // For default modifier
  Modifier *next;
};

struct ModifierList {
  Modifier *modifier;
  ModifierList *next;
  ModifierList *last;  // Tail pointer for O(1) append
};

struct PrimaryKey {
  QualifiedIdentifierList *attributes;
};

struct Assertion {
  Expression *condition;
  Assertion *next;
};

struct Participation {
  ParticipationType type; // PARTICIPATION_TOTAL or PARTICIPATION_PARTIAL
};

struct IdentifierList {
  char *identifier;
  IdentifierList *next;
};

struct QualifiedIdentifier {
  char *entity;
  char *attribute;
};

struct QualifiedIdentifierList {
  QualifiedIdentifier *qid;
  struct QualifiedIdentifierList *next;
  struct QualifiedIdentifierList *last;  // Tail pointer for O(1) append
};

/**
 * Node destructors
 */
void destroyProgram(Program *program);
void destroySchema(Schema *schema);
void destroyEntity(Entity *entity);
void destroyRelationship(Relationship *relationship);
void destroyAttribute(Attribute *attribute);
void destroyAttributeList(AttributeList *attributeList);
void destroyRelationshipList(RelationshipList *relationshipList);
void destroyParticipant(Participant *participant);
void destroyParticipantList(ParticipantList *participantList);
void destroyExpression(Expression *expression);
void destroyLiteral(Literal *literal);
void destroyType(Type *type);
void destroyModifier(Modifier *modifier);
void destroyModifierList(ModifierList *modifierList);
void destroyPrimaryKey(PrimaryKey *primaryKey);
void destroyAssertion(Assertion *assertion);
void destroyParticipation(Participation *participation);
void destroyIdentifierList(IdentifierList *identifierList);
void destroyQualifiedIdentifierList(QualifiedIdentifierList *qidList);
void destroyQualifiedIdentifier(QualifiedIdentifier *qid);

#endif
