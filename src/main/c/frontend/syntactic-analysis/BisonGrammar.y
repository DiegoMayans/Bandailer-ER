%{

#include "../../support/type/TokenLabel.h"
#include "AbstractSyntaxTree.h"
#include "BisonActions.h"

/**
 * The error reporting function for Bison parser.
 *
 * @todo Add location to the grammar and "pushToken" API function.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Error-Reporting-Function.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Tracking-Locations.html
 */
void yyerror(const YYLTYPE * location, const char * message) {}

%}

// You touch this, and you die.
%define api.pure full
%define api.push-pull push
%define api.value.union.name SemanticValue
%define parse.error detailed
%locations

%union {
    /* Terminals */
    signed int integer;
    double decimal;
    char *string;
    bool boolean;
    TokenLabel token;

    /* Non-terminals */
    Program *program;
    Schema *schema;
    Entity *entity;
    Relationship *relationship;
    Attribute *attribute;
    AttributeList *attributeList;
    RelationshipList *relationshipList;
    Participant *participant;
    ParticipantList *participantList;
    Expression *expression;
    Literal *literal;
    Type *type;
    Modifier *modifier;
    ModifierList *modifierList;
    PrimaryKey *primaryKey;
    Assertion *assertion;
		Participation *participation;
		IdentifierList *identifierList;
}

/**
 * Destructors. This functions are executed after the parsing ends, so if the
 * AST must be used in the following phases of the compiler you shouldn't used
 * this approach for the AST root node ("program" non-terminal, in this
 * grammar), or it will drop the entire tree even if the parsing succeeds.
 *
 * @see https://www.gnu.org/software/bison/manual/html_node/Destructor-Decl.html
 */
%destructor { destroyExpression($$); } <expression>
%destructor { free($$); } <string>

/** Terminals. */
%token <token> SCHEMA ENTITY RELATIONSHIP PRIMARY UNIQUE DERIVED NOT NULL_TOK DEFAULT ASSERT TOTAL PARTIAL IF THEN OTHERWISE ENUM

%token <token> TYPE_INTEGER TYPE_DECIMAL TYPE_STRING TYPE_BOOL TYPE_DATE TYPE_DATETIME TYPE_UUID

%token <token> OP_EQ OP_NEQ OP_GT OP_LT OP_GTE OP_LTE
%token <token> OP_ADD OP_SUB OP_MUL OP_DIV
%token <token> OP_AND OP_OR OP_NOT OP_ARROW

%token <token> OPEN_BRACE CLOSE_BRACE OPEN_PAREN CLOSE_PAREN
%token <token> COLON COMMA SEMICOLON DOT

%token <token> LIT_INTEGER LIT_DECIMAL LIT_STRING LIT_BOOL
%token <token> IDENTIFIER

%token <token> IGNORED UNKNOWN EOF

/** Non-terminals. */
%type <program> program
%type <schema> schema_decl schema_body
%type <entity> entity_decl entity_body
%type <relationship> relationship_decl relationship_body
%type <attribute> attribute_decl
%type <attributeList> attribute_list
%type <type> type_spec
%type <modifier> modifier
%type <modifierList> modifier_list
%type <primaryKey> primary_decl
%type <assertion> assert_decl
%type <participant> relationship_participant
%type <participantList> relationship_participant_list
%type <expression> expression
%type <literal> literal
%type <participation> participation_opt
%type <identifierList> identifier_list


/**
 * Precedence and associativity.
 *
 * @see https://en.cppreference.com/w/cpp/language/operator_precedence.html
 * @see https://www.gnu.org/software/bison/manual/html_node/Precedence.html
 */
%left OP_ADD OP_SUB
%left OP_MUL OP_DIV

%%

// IMPORTANT: To use λ in the following grammar, use the %empty symbol.

program:
      schema_decl                             { $$ = SchemaProgramSemanticAction($1); }
    | program schema_decl                     { $$ = AppendSchemaProgramSemanticAction($1, $2); }
    ;

schema_decl:
      SCHEMA IDENTIFIER schema_body
        { $$ = SchemaSemanticAction($2, $3); }
    ;

schema_body:
      %empty 			                            { $$ = EmptySchemaBodySemanticAction(); }
    | schema_body entity_decl                { $$ = AppendEntitySchemaBodySemanticAction($1, $2); }
    | schema_body relationship_decl           { $$ = AppendRelationshipSchemaBodySemanticAction($1, $2); }
    ;

entity_decl:
      ENTITY IDENTIFIER OPEN_BRACE entity_body CLOSE_BRACE
        { $$ = EntitySemanticAction($2, NULL, $4); }
    | ENTITY IDENTIFIER COLON IDENTIFIER OPEN_BRACE entity_body CLOSE_BRACE
        { $$ = EntitySemanticAction($2, $4, $6); } /* inheritance */
    ;

entity_body:
      %empty							                    { $$ = EmptyEntityBodySemanticAction(); }
    | entity_body attribute_decl              { $$ = AppendAttributeEntityBodySemanticAction($1, $2); }
    | entity_body primary_decl                { $$ = SetPrimaryKeyEntityBodySemanticAction($1, $2); }
    | entity_body assert_decl                 { $$ = AppendAssertionEntityBodySemanticAction($1, $2); }
    ;

attribute_decl:
      IDENTIFIER COLON type_spec modifier_list
        { $$ = AttributeSemanticAction($1, $3, $4); }
    ;

modifier_list:
      %empty                     							{ $$ = EmptyModifierListSemanticAction(); }
    | modifier_list modifier                  { $$ = AppendModifierSemanticAction($1, $2); }
    ;

modifier:
      PRIMARY                                 { $$ = ModifierSemanticAction(PRIMARY); }
    | UNIQUE                                  { $$ = ModifierSemanticAction(UNIQUE); }
    | NOT NULL_TOK                            { $$ = ModifierSemanticAction(NOT_NULL); }
    | DEFAULT literal                         { $$ = ModifierDefaultSemanticAction($2); }
    | DERIVED                                 { $$ = ModifierSemanticAction(DERIVED); }
    ;

primary_decl:
      PRIMARY OPEN_PAREN identifier_list CLOSE_PAREN
        { $$ = PrimaryKeySemanticAction($3); }
    ;

identifier_list:
      IDENTIFIER                              { $$ = IdentifierListSemanticAction($1); }
    | identifier_list COMMA IDENTIFIER        { $$ = AppendIdentifierListSemanticAction($1, $3); }
    ;

assert_decl:
      ASSERT expression                       { $$ = AssertionSemanticAction($2); }
    ;

relationship_decl:
      RELATIONSHIP IDENTIFIER OPEN_PAREN relationship_participant_list CLOSE_PAREN relationship_body
        { $$ = RelationshipSemanticAction($2, $4, $6); }
    ;

relationship_participant_list:
      relationship_participant                { $$ = ParticipantListSemanticAction($1); }
    | relationship_participant_list COMMA relationship_participant
                                              { $$ = AppendParticipantListSemanticAction($1, $3); }
    ;

relationship_participant:
      IDENTIFIER OP_MUL OP_ARROW IDENTIFIER participation_opt
        { $$ = ParticipantSemanticAction($1, $4, $5); }
    ;

participation_opt:
      %empty                                  { $$ = NULL; }
    | TOTAL                                   { $$ = ParticipationSemanticAction(TOTAL); }
    | PARTIAL                                 { $$ = ParticipationSemanticAction(PARTIAL); }
    ;

relationship_body:
      %empty		                              { $$ = EmptyRelationshipBodySemanticAction(); }
    | OPEN_BRACE attribute_list CLOSE_BRACE   { $$ = RelationshipBodySemanticAction($2); }
    ;

attribute_list:
      attribute_decl                          { $$ = AttributeListSemanticAction($1); }
    | attribute_list attribute_decl           { $$ = AppendAttributeListSemanticAction($1, $2); }
    ;

type_spec:
      TYPE_INTEGER                            { $$ = TypeSemanticAction(TYPE_INTEGER); }
    | TYPE_DECIMAL                            { $$ = TypeSemanticAction(TYPE_DECIMAL); }
    | TYPE_STRING                             { $$ = TypeSemanticAction(TYPE_STRING); }
    | TYPE_BOOL                               { $$ = TypeSemanticAction(TYPE_BOOL); }
    | TYPE_DATE                               { $$ = TypeSemanticAction(TYPE_DATE); }
    | TYPE_DATETIME                           { $$ = TypeSemanticAction(TYPE_DATETIME); }
    | TYPE_UUID                               { $$ = TypeSemanticAction(TYPE_UUID); }
    | ENUM OPEN_BRACE identifier_list CLOSE_BRACE
                                              { $$ = EnumTypeSemanticAction($3); }
    ;

expression:
      literal                                 { $$ = LiteralExpressionSemanticAction($1); }
    | IDENTIFIER                              { $$ = IdentifierExpressionSemanticAction($1); }
    | expression OP_ADD expression            { $$ = ArithmeticExpressionSemanticAction($1, $3, ADDITION); }
    | expression OP_SUB expression            { $$ = ArithmeticExpressionSemanticAction($1, $3, SUBTRACTION); }
    | expression OP_MUL expression            { $$ = ArithmeticExpressionSemanticAction($1, $3, MULTIPLICATION); }
    | expression OP_DIV expression            { $$ = ArithmeticExpressionSemanticAction($1, $3, DIVISION); }
    | expression OP_EQ expression             { $$ = RelationalExpressionSemanticAction($1, $3, EQUAL); }
    | expression OP_NEQ expression            { $$ = RelationalExpressionSemanticAction($1, $3, NOT_EQUAL); }
    | expression OP_GT expression             { $$ = RelationalExpressionSemanticAction($1, $3, GREATER); }
    | expression OP_LT expression             { $$ = RelationalExpressionSemanticAction($1, $3, LESS); }
    | expression OP_GTE expression            { $$ = RelationalExpressionSemanticAction($1, $3, GREATER_EQUAL); }
    | expression OP_LTE expression            { $$ = RelationalExpressionSemanticAction($1, $3, LESS_EQUAL); }
    | expression OP_AND expression            { $$ = LogicalExpressionSemanticAction($1, $3, AND); }
    | expression OP_OR expression             { $$ = LogicalExpressionSemanticAction($1, $3, OR); }
    | OP_NOT expression                       { $$ = LogicalNotExpressionSemanticAction($2); }
    | IF expression THEN expression OTHERWISE expression
                                              { $$ = ConditionalExpressionSemanticAction($2, $4, $6); }
    | OPEN_PAREN expression CLOSE_PAREN       { $$ = ParenthesizedExpressionSemanticAction($2); }
    ;

literal:
      LIT_INTEGER                             { $$ = IntegerLiteralSemanticAction($1); }
    | LIT_DECIMAL                             { $$ = DecimalLiteralSemanticAction($1); }
    | LIT_STRING                              { $$ = StringLiteralSemanticAction($1); }
    | LIT_BOOL                                { $$ = BooleanLiteralSemanticAction($1); }
    ;

%%
