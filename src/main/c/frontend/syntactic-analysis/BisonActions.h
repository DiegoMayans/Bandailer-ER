#ifndef BISON_ACTIONS_HEADER
#define BISON_ACTIONS_HEADER

#include "../../support/logging/Logger.h"
#include "../../support/type/CompilerState.h"
#include "../../support/type/ModuleDestructor.h"
#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonParser.h"
#include <stdlib.h>

/** Initialize module's internal state. */
ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState);

/**
 * Bison semantic actions for ER modeling language
 */

/* Program and Schema actions */
Program * SchemaProgramSemanticAction(Schema * schema);
Program * AppendSchemaProgramSemanticAction(Program * program, Schema * schema);
Schema * SchemaSemanticAction(char * identifier, Schema * body);
Schema * EmptySchemaBodySemanticAction();
Schema * AppendEntitySchemaBodySemanticAction(Schema * schema, Entity * entity);
Schema * AppendRelationshipSchemaBodySemanticAction(Schema * schema, Relationship * relationship);

/* Entity actions */
Entity * EntitySemanticAction(char * name, char * parent, Entity * body);
Entity * EmptyEntityBodySemanticAction();
Entity * AppendAttributeEntityBodySemanticAction(Entity * entity, Attribute * attribute);
Entity * SetPrimaryKeyEntityBodySemanticAction(Entity * entity, PrimaryKey * primaryKey);
Entity * AppendAssertionEntityBodySemanticAction(Entity * entity, Assertion * assertion);

/* Attribute actions */
Attribute * AttributeSemanticAction(char * name, Type * type, ModifierList * modifiers);
AttributeList * AttributeListSemanticAction(Attribute * attribute);
AttributeList * AppendAttributeListSemanticAction(AttributeList * list, Attribute * attribute);

/* Type actions */
Type * TypeSemanticAction(TypeKind typeToken);
Type * EnumTypeSemanticAction(IdentifierList * enumValues);

/* Modifier actions */
ModifierList * EmptyModifierListSemanticAction();
ModifierList * AppendModifierSemanticAction(ModifierList * list, Modifier * modifier);
Modifier * ModifierSemanticAction(ModifierType modifierType);
Modifier * ModifierDefaultSemanticAction(Literal * defaultValue);

/* Primary key actions */
PrimaryKey * PrimaryKeySemanticAction(IdentifierList * attributes);

/* Identifier list actions */
IdentifierList * IdentifierListSemanticAction(char * identifier);
IdentifierList * AppendIdentifierListSemanticAction(IdentifierList * list, char * identifier);
char * ConcatenateIdentifiers(char * prefix, char * identifier);

/* Assertion actions */
Assertion * AssertionSemanticAction(Expression * condition);

/* Relationship actions */
Relationship * RelationshipSemanticAction(char * name, ParticipantList * participants, Relationship * body);
ParticipantList * ParticipantListSemanticAction(Participant * participant);
ParticipantList * AppendParticipantListSemanticAction(ParticipantList * list, Participant * participant);
Participant * ParticipantSemanticAction(char * entityName, Participation * participation, CardinalityType cardinality);
Participation * ParticipationSemanticAction(TokenLabel participationType);
Relationship * EmptyRelationshipBodySemanticAction();
Relationship * RelationshipBodySemanticAction(AttributeList * attributes);

/* Expression actions */
Expression * LiteralExpressionSemanticAction(Literal * literal);
Expression * IdentifierExpressionSemanticAction(char * identifier);
Expression * ArithmeticExpressionSemanticAction(Expression * left, Expression * right, ArithmeticOperator op);
Expression * RelationalExpressionSemanticAction(Expression * left, Expression * right, RelationalOperator op);
Expression * LogicalExpressionSemanticAction(Expression * left, Expression * right, LogicalOperator op);
Expression * LogicalNotExpressionSemanticAction(Expression * operand);
Expression * ConditionalExpressionSemanticAction(Expression * condition, Expression * thenExpr, Expression * elseExpr);
Expression * ParenthesizedExpressionSemanticAction(Expression * expression);

/* Literal actions */
Literal * IntegerLiteralSemanticAction(int value);
Literal * DecimalLiteralSemanticAction(double value);
Literal * StringLiteralSemanticAction(char * token);
Literal * BooleanLiteralSemanticAction(bool value);

#endif
