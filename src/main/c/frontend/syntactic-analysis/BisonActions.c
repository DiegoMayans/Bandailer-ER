#include "BisonActions.h"
#include <string.h>

/* MODULE INTERNAL STATE */

static CompilerState * _compilerState = NULL;
static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownBisonActionsModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: BisonActions...");
		destroyLogger(_logger);
		_logger = NULL;
	}
	_compilerState = NULL;
}

ModuleDestructor initializeBisonActionsModule(CompilerState * compilerState) {
	_compilerState = compilerState;
	_logger = createLogger("BisonActions");
	return _shutdownBisonActionsModule;
}

/* PRIVATE FUNCTIONS */

static void _logSyntacticAnalyzerAction(const char * functionName);

/**
 * Logs a syntactic-analyzer action in DEBUGGING level.
 */
static void _logSyntacticAnalyzerAction(const char * functionName) {
	logDebugging(_logger, "%s", functionName);
}

/* PUBLIC FUNCTIONS - Program and Schema actions */

Program * SchemaProgramSemanticAction(Schema * schema) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Program * program = calloc(1, sizeof(Program));
	program->schema = schema;
	_compilerState->abstractSyntaxtTree = program;
	return program;
}

Program * AppendSchemaProgramSemanticAction(Program * program, Schema * schema) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (program == NULL) {
		return SchemaProgramSemanticAction(schema);
	}
	
	Schema * current = program->schema;
	if (current == NULL) {
		program->schema = schema;
	} else {
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = schema;
	}
	return program;
}

Schema * SchemaSemanticAction(char *identifier, Schema * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Schema * schema = calloc(1, sizeof(Schema));
	schema->name = strdup(identifier);
	if (body != NULL) {
		schema->entities = body->entities;
		schema->relationships = body->relationships;
	}
	return schema;
}

Schema * EmptySchemaBodySemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return calloc(1, sizeof(Schema));
}

Schema * AppendEntitySchemaBodySemanticAction(Schema * schema, Entity * entity) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (schema == NULL) {
		schema = EmptySchemaBodySemanticAction();
	}
	
	if (schema->entities == NULL) {
		schema->entities = entity;
	} else {
		Entity * current = schema->entities;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = entity;
	}
	return schema;
}

Schema * AppendRelationshipSchemaBodySemanticAction(Schema * schema, Relationship * relationship) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (schema == NULL) {
		schema = EmptySchemaBodySemanticAction();
	}
	
	if (schema->relationships == NULL) {
		schema->relationships = relationship;
	} else {
		Relationship * current = schema->relationships;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = relationship;
	}
	return schema;
}

/* Entity actions */

Entity * EntitySemanticAction(char * name, char * parent, Entity * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Entity * entity = calloc(1, sizeof(Entity));
	entity->name = strdup(name);
	if (parent != NULL) {
		entity->parent = strdup(parent);
	}
	if (body != NULL) {
		entity->attributes = body->attributes;
		entity->primaryKey = body->primaryKey;
		entity->assertions = body->assertions;
	}
	return entity;
}

Entity * EmptyEntityBodySemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return calloc(1, sizeof(Entity));
}

Entity * AppendAttributeEntityBodySemanticAction(Entity * entity, Attribute * attribute) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (entity == NULL) {
		entity = EmptyEntityBodySemanticAction();
	}
	
	if (entity->attributes == NULL) {
		entity->attributes = calloc(1, sizeof(AttributeList));
		entity->attributes->attribute = attribute;
	} else {
		AttributeList * current = entity->attributes;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = calloc(1, sizeof(AttributeList));
		current->next->attribute = attribute;
	}
	return entity;
}

Entity * SetPrimaryKeyEntityBodySemanticAction(Entity * entity, PrimaryKey * primaryKey) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (entity == NULL) {
		entity = EmptyEntityBodySemanticAction();
	}
	entity->primaryKey = primaryKey;
	return entity;
}

Entity * AppendAssertionEntityBodySemanticAction(Entity * entity, Assertion * assertion) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (entity == NULL) {
		entity = EmptyEntityBodySemanticAction();
	}
	
	if (entity->assertions == NULL) {
		entity->assertions = assertion;
	} else {
		Assertion * current = entity->assertions;
		while (current->next != NULL) {
			current = current->next;
		}
		current->next = assertion;
	}
	return entity;
}

/* Attribute actions */

Attribute * AttributeSemanticAction(char * name, Type * type, ModifierList * modifiers) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Attribute * attribute = calloc(1, sizeof(Attribute));
	attribute->name = strdup(name);
	attribute->type = type;
	attribute->modifiers = modifiers;
	return attribute;
}

AttributeList * AttributeListSemanticAction(Attribute * attribute) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	AttributeList * list = calloc(1, sizeof(AttributeList));
	list->attribute = attribute;
	return list;
}

AttributeList * AppendAttributeListSemanticAction(AttributeList * list, Attribute * attribute) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (list == NULL) {
		return AttributeListSemanticAction(attribute);
	}
	
	AttributeList * current = list;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = calloc(1, sizeof(AttributeList));
	current->next->attribute = attribute;
	return list;
}

/* Type actions */

Type * TypeSemanticAction(TypeKind typeToken) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = typeToken;
	return type;
}

Type * EnumTypeSemanticAction(IdentifierList * enumValues) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	type->kind = TYPE_ENUM;
	type->enumValues = enumValues;
	return type;
}

/* Modifier actions */

ModifierList * EmptyModifierListSemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return NULL; // Empty list
}

ModifierList * AppendModifierSemanticAction(ModifierList * list, Modifier * modifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ModifierList * newNode = calloc(1, sizeof(ModifierList));
	newNode->modifier = modifier;
	
	if (list == NULL) {
		return newNode;
	}
	
	ModifierList * current = list;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = newNode;
	return list;
}

Modifier * ModifierSemanticAction(ModifierType modifierType) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Modifier * modifier = calloc(1, sizeof(Modifier));
	modifier->type = modifierType;
	return modifier;
}

Modifier * ModifierDefaultSemanticAction(Literal * defaultValue) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Modifier * modifier = calloc(1, sizeof(Modifier));
	modifier->type = MOD_DEFAULT;
	modifier->defaultValue = defaultValue;
	return modifier;
}

/* Primary key actions */

PrimaryKey * PrimaryKeySemanticAction(IdentifierList * attributes) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	PrimaryKey * primaryKey = calloc(1, sizeof(PrimaryKey));
	primaryKey->attributes = attributes;
	return primaryKey;
}

/* Identifier list actions */

IdentifierList * IdentifierListSemanticAction(char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	IdentifierList * list = calloc(1, sizeof(IdentifierList));
	list->identifier = strdup(identifier);
	return list;
}

IdentifierList * AppendIdentifierListSemanticAction(IdentifierList * list, char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (list == NULL) {
		return IdentifierListSemanticAction(identifier);
	}
	
	IdentifierList * current = list;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = calloc(1, sizeof(IdentifierList));
	current->next->identifier = strdup(identifier);
	return list;
}

/* Assertion actions */

Assertion * AssertionSemanticAction(Expression * condition) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Assertion * assertion = calloc(1, sizeof(Assertion));
	assertion->condition = condition;
	return assertion;
}

/* Relationship actions */

Relationship * RelationshipSemanticAction(char * name, ParticipantList * participants, Relationship * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Relationship * relationship = calloc(1, sizeof(Relationship));
	relationship->name = strdup(name);
	relationship->participants = participants;
	if (body != NULL) {
		relationship->attributes = body->attributes;
	}
	return relationship;
}

ParticipantList * ParticipantListSemanticAction(Participant * participant) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	ParticipantList * list = calloc(1, sizeof(ParticipantList));
	list->participant = participant;
	return list;
}

ParticipantList * AppendParticipantListSemanticAction(ParticipantList * list, Participant * participant) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (list == NULL) {
		return ParticipantListSemanticAction(participant);
	}
	
	ParticipantList * current = list;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = calloc(1, sizeof(ParticipantList));
	current->next->participant = participant;
	return list;
}

Participant * ParticipantSemanticAction(char * fromEntity, char * toEntity, Participation * participation) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Participant * participant = calloc(1, sizeof(Participant));
	participant->fromEntity = strdup(fromEntity);
	participant->toEntity = strdup(toEntity);
	participant->participation = participation;
	return participant;
}

Participation * ParticipationSemanticAction(TokenLabel participationType) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Participation * participation = calloc(1, sizeof(Participation));
	participation->type = participationType;
	return participation;
}

Relationship * EmptyRelationshipBodySemanticAction() {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	return calloc(1, sizeof(Relationship));
}

Relationship * RelationshipBodySemanticAction(AttributeList * attributes) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Relationship * relationship = calloc(1, sizeof(Relationship));
	relationship->attributes = attributes;
	return relationship;
}

/* Expression actions */

Expression * LiteralExpressionSemanticAction(Literal * literal) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = LITERAL_EXPR;
	expression->literal = literal;
	return expression;
}

Expression * IdentifierExpressionSemanticAction(char * identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = IDENTIFIER_EXPR;
	expression->identifier = strdup(identifier);
	return expression;
}

Expression * ArithmeticExpressionSemanticAction(Expression * left, Expression * right, ArithmeticOperator op) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = ARITHMETIC_EXPR;
	expression->arithmetic.left = left;
	expression->arithmetic.right = right;
	expression->arithmetic.arithmeticOp = op;
	return expression;
}

Expression * RelationalExpressionSemanticAction(Expression * left, Expression * right, RelationalOperator op) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = RELATIONAL_EXPR;
	expression->relational.left = left;
	expression->relational.right = right;
	expression->relational.relationalOp = op;
	return expression;
}

Expression * LogicalExpressionSemanticAction(Expression * left, Expression * right, LogicalOperator op) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = LOGICAL_EXPR;
	expression->logical.left = left;
	expression->logical.right = right;
	expression->logical.logicalOp = op;
	return expression;
}

Expression * LogicalNotExpressionSemanticAction(Expression * operand) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = LOGICAL_NOT_EXPR;
	expression->logicalNot.operand = operand;
	return expression;
}

Expression * ConditionalExpressionSemanticAction(Expression * condition, Expression * thenExpr, Expression * elseExpr) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = CONDITIONAL_EXPR;
	expression->conditional.condition = condition;
	expression->conditional.thenExpr = thenExpr;
	expression->conditional.elseExpr = elseExpr;
	return expression;
}

Expression * ParenthesizedExpressionSemanticAction(Expression * expression) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * result = calloc(1, sizeof(Expression));
	result->type = PARENTHESIZED_EXPR;
	result->parenthesized.expression = expression;
	return result;
}

/* Literal actions */

Literal * IntegerLiteralSemanticAction(int value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Literal * literal = calloc(1, sizeof(Literal));
	literal->type = INTEGER_LITERAL;
	literal->integer = value;
	return literal;
}

Literal * DecimalLiteralSemanticAction(double value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Literal * literal = calloc(1, sizeof(Literal));
	literal->type = DECIMAL_LITERAL;
	literal->decimal = value;
	return literal;
}

Literal * StringLiteralSemanticAction(char * token) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Literal * literal = calloc(1, sizeof(Literal));
	literal->type = STRING_LITERAL;
	literal->string = strdup(token);
	return literal;
}

Literal * BooleanLiteralSemanticAction(bool value) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Literal * literal = calloc(1, sizeof(Literal));
	literal->type = BOOLEAN_LITERAL;
	literal->boolean = value;
	return literal;
}
