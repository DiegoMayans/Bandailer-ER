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

/* Helper function to extract string from token */
static char* _extractTokenString(TokenLabel token) {
	// This would need to be implemented based on how tokens store their lexeme
	// For now, returning a placeholder
	return strdup("identifier");
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
	
	// Find the last schema in the list and append
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

Schema * SchemaSemanticAction(TokenLabel identifier, Schema * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Schema * schema = calloc(1, sizeof(Schema));
	schema->name = _extractTokenString(identifier);
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

Entity * EntitySemanticAction(TokenLabel name, TokenLabel parent, Entity * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Entity * entity = calloc(1, sizeof(Entity));
	entity->name = _extractTokenString(name);
	if (parent != 0) {
		entity->parent = _extractTokenString(parent);
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

Attribute * AttributeSemanticAction(TokenLabel name, Type * type, ModifierList * modifiers) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Attribute * attribute = calloc(1, sizeof(Attribute));
	attribute->name = _extractTokenString(name);
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

Type * TypeSemanticAction(TokenLabel typeToken) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Type * type = calloc(1, sizeof(Type));
	
	// Map token to type kind
	switch (typeToken) {
		case TYPE_INTEGER: type->kind = TYPE_INTEGER; break;
		case TYPE_DECIMAL: type->kind = TYPE_DECIMAL; break;
		case TYPE_STRING: type->kind = TYPE_STRING; break;
		case TYPE_BOOL: type->kind = TYPE_BOOL; break;
		case TYPE_DATE: type->kind = TYPE_DATE; break;
		case TYPE_DATETIME: type->kind = TYPE_DATETIME; break;
		case TYPE_UUID: type->kind = TYPE_UUID; break;
		default: type->kind = TYPE_STRING; break;
	}
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

Modifier * ModifierSemanticAction(TokenLabel modifierType) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Modifier * modifier = calloc(1, sizeof(Modifier));
	
	switch (modifierType) {
		case PRIMARY: modifier->type = MOD_PRIMARY; break;
		case UNIQUE: modifier->type = MOD_UNIQUE; break;
		case NOT: modifier->type = MOD_NOT_NULL; break; // NOT NULL_TOK becomes MOD_NOT_NULL
		case DERIVED: modifier->type = MOD_DERIVED; break;
		default: modifier->type = MOD_PRIMARY; break;
	}
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

IdentifierList * IdentifierListSemanticAction(TokenLabel identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	IdentifierList * list = calloc(1, sizeof(IdentifierList));
	list->identifier = _extractTokenString(identifier);
	return list;
}

IdentifierList * AppendIdentifierListSemanticAction(IdentifierList * list, TokenLabel identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	if (list == NULL) {
		return IdentifierListSemanticAction(identifier);
	}
	
	IdentifierList * current = list;
	while (current->next != NULL) {
		current = current->next;
	}
	current->next = calloc(1, sizeof(IdentifierList));
	current->next->identifier = _extractTokenString(identifier);
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

Relationship * RelationshipSemanticAction(TokenLabel name, ParticipantList * participants, Relationship * body) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Relationship * relationship = calloc(1, sizeof(Relationship));
	relationship->name = _extractTokenString(name);
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

Participant * ParticipantSemanticAction(TokenLabel fromEntity, TokenLabel toEntity, Participation * participation) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Participant * participant = calloc(1, sizeof(Participant));
	participant->fromEntity = _extractTokenString(fromEntity);
	participant->toEntity = _extractTokenString(toEntity);
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

Expression * IdentifierExpressionSemanticAction(TokenLabel identifier) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Expression * expression = calloc(1, sizeof(Expression));
	expression->type = IDENTIFIER_EXPR;
	expression->identifier = _extractTokenString(identifier);
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

Literal * IntegerLiteralSemanticAction(TokenLabel token) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Literal * literal = calloc(1, sizeof(Literal));
	literal->type = INTEGER_LITERAL;
	// This would need proper token value extraction
	literal->integer = 0; // Placeholder
	return literal;
}

Literal * DecimalLiteralSemanticAction(TokenLabel token) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Literal * literal = calloc(1, sizeof(Literal));
	literal->type = DECIMAL_LITERAL;
	literal->decimal = 0.0; // Placeholder
	return literal;
}

Literal * StringLiteralSemanticAction(TokenLabel token) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Literal * literal = calloc(1, sizeof(Literal));
	literal->type = STRING_LITERAL;
	literal->string = _extractTokenString(token);
	return literal;
}

Literal * BooleanLiteralSemanticAction(TokenLabel token) {
	_logSyntacticAnalyzerAction(__FUNCTION__);
	Literal * literal = calloc(1, sizeof(Literal));
	literal->type = BOOLEAN_LITERAL;
	literal->boolean = false; // Placeholder
	return literal;
}
