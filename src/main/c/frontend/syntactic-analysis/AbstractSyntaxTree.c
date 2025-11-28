#include "AbstractSyntaxTree.h"

/* MODULE INTERNAL STATE */

static Logger * _logger = NULL;

/** Shutdown module's internal state. */
void _shutdownAbstractSyntaxTreeModule() {
	if (_logger != NULL) {
		logDebugging(_logger, "Destroying module: AbstractSyntaxTree...");
		destroyLogger(_logger);
		_logger = NULL;
	}
}

ModuleDestructor initializeAbstractSyntaxTreeModule() {
	_logger = createLogger("AbstractSyntaxTree");
	return _shutdownAbstractSyntaxTreeModule;
}

/* PUBLIC FUNCTIONS */

void destroyProgram(Program* program) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (program != NULL) {
		destroySchema(program->schema);
		free(program);
	}
}

void destroySchema(Schema* schema) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (schema != NULL) {
		free(schema->name);
		destroyEntity(schema->entities);
		destroyRelationship(schema->relationships);
		free(schema);
	}
}

void destroyEntity(Entity* entity) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (entity != NULL) {
		free(entity->name);
		free(entity->parent);
		destroyAttributeList(entity->attributes);
		destroyPrimaryKey(entity->primaryKey);
		destroyAssertion(entity->assertions);
		destroyEntity(entity->next);
		free(entity);
	}
}

void destroyRelationship(Relationship* relationship) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (relationship != NULL) {
		free(relationship->name);
		destroyParticipantList(relationship->participants);
		destroyAttributeList(relationship->attributes);
		destroyRelationship(relationship->next);
		free(relationship);
	}
}

void destroyAttribute(Attribute* attribute) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (attribute != NULL) {
		free(attribute->name);
		destroyType(attribute->type);
		destroyModifierList(attribute->modifiers);
		destroyAttribute(attribute->next);
		free(attribute);
	}
}

void destroyAttributeList(AttributeList* attributeList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (attributeList != NULL) {
		destroyAttribute(attributeList->attribute);
		destroyAttributeList(attributeList->next);
		free(attributeList);
	}
}

void destroyRelationshipList(RelationshipList* relationshipList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (relationshipList != NULL) {
		destroyRelationship(relationshipList->relationship);
		destroyRelationshipList(relationshipList->next);
		free(relationshipList);
	}
}

void destroyParticipant(Participant* participant) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (participant != NULL) {
		free(participant->entityName);
		destroyParticipation(participant->participation);
		destroyParticipant(participant->next);
		free(participant);
	}
}

void destroyParticipantList(ParticipantList* participantList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (participantList != NULL) {
		destroyParticipant(participantList->participant);
		destroyParticipantList(participantList->next);
		free(participantList);
	}
}

void destroyExpression(Expression* expression) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (expression != NULL) {
		switch (expression->type) {
			case LITERAL_EXPR:
				destroyLiteral(expression->literal);
				break;
			case IDENTIFIER_EXPR:
				free(expression->identifier);
				break;
			case ARITHMETIC_EXPR:
				destroyExpression(expression->arithmetic.left);
				destroyExpression(expression->arithmetic.right);
				break;
			case RELATIONAL_EXPR:
				destroyExpression(expression->relational.left);
				destroyExpression(expression->relational.right);
				break;
			case LOGICAL_EXPR:
				destroyExpression(expression->logical.left);
				destroyExpression(expression->logical.right);
				break;
			case LOGICAL_NOT_EXPR:
				destroyExpression(expression->logicalNot.operand);
				break;
			case CONDITIONAL_EXPR:
				destroyExpression(expression->conditional.condition);
				destroyExpression(expression->conditional.thenExpr);
				destroyExpression(expression->conditional.elseExpr);
				break;
			case PARENTHESIZED_EXPR:
				destroyExpression(expression->parenthesized.expression);
				break;
		}
		free(expression);
	}
}

void destroyLiteral(Literal* literal) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (literal != NULL) {
		if (literal->type == STRING_LITERAL) {
			free(literal->string);
		}
		free(literal);
	}
}

void destroyType(Type* type) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (type != NULL) {
		free(type);
	}
}

void destroyModifier(Modifier* modifier) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (modifier != NULL) {
		if (modifier->type == MOD_DEFAULT) {
			destroyLiteral(modifier->defaultValue);
		}
		destroyModifier(modifier->next);
		free(modifier);
	}
}

void destroyModifierList(ModifierList* modifierList) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (modifierList != NULL) {
		destroyModifier(modifierList->modifier);
		destroyModifierList(modifierList->next);
		free(modifierList);
	}
}

void destroyPrimaryKey(PrimaryKey* primaryKey) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (primaryKey != NULL) {
		destroyQualifiedIdentifierList(primaryKey->attributes);
		free(primaryKey);
	}
}

void destroyAssertion(Assertion* assertion) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (assertion != NULL) {
		destroyExpression(assertion->condition);
		destroyAssertion(assertion->next);
		free(assertion);
	}
}

void destroyParticipation(Participation* participation) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (participation != NULL) {
		free(participation);
	}
}

void destroyQualifiedIdentifierList(QualifiedIdentifierList* qidList) {
		logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
		if (qidList != NULL) {
				destroyQualifiedIdentifier(qidList->qid);
				destroyQualifiedIdentifierList(qidList->next);
				free(qidList);
		}
}

void destroyQualifiedIdentifier(QualifiedIdentifier* qid) {
	logDebugging(_logger, "Executing destructor: %s", __FUNCTION__);
	if (qid != NULL) {
		free(qid->entity);
		free(qid->attribute);
		free(qid);
	}
}
