%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../../support/type/TokenDefinitions.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

#include "FlexActions.h"
#include "FlexExport.h"

// Declaraciones de funciones de acción léxica
extern CompilationStatus KeywordLexemeAction(TokenLabel label);
extern CompilationStatus IdentifierLexemeAction();
extern CompilationStatus TypeLexemeAction(TokenLabel label);
extern CompilationStatus OperatorLexemeAction(TokenLabel label);
extern CompilationStatus SymbolLexemeAction(TokenLabel label);
extern CompilationStatus LiteralLexemeAction(TokenLabel label);
extern CompilationStatus EnterCommentLexemeAction(FlexContext context);
extern CompilationStatus LeaveCommentLexemeAction();
extern CompilationStatus IgnoredLexemeAction();
extern CompilationStatus EOFLexemeAction();
extern CompilationStatus UnknownLexemeAction();

%}

/**
 * Contextos (start conditions)
 */
%x COMMENT_BLOCK

/**
 * Patrones reutilizables
 */
whitespace      [ \t\r\n]+
digit           [0-9]
integer         {digit}+
decimal         {digit}+"."{digit}+
identifier      [A-Za-z_][A-Za-z0-9_]*
string_literal  \"([^\"\\]|\\.)*\"
bool_literal    (true|false)

%%

/* === Palabras clave del lenguaje === */
"schema"                { return KeywordLexemeAction(SCHEMA); }
"entity"                { return KeywordLexemeAction(ENTITY); }
"relationship"          { return KeywordLexemeAction(RELATIONSHIP); }
"primary"               { return KeywordLexemeAction(PRIMARY); }
"unique"                { return KeywordLexemeAction(UNIQUE); }
"derived"               { return KeywordLexemeAction(DERIVED); }
"not"                   { return KeywordLexemeAction(NOT); }
"null"                  { return KeywordLexemeAction(NULL_TOK); }
"default"               { return KeywordLexemeAction(DEFAULT); }
"assert"                { return KeywordLexemeAction(ASSERT); }
"total"                 { return KeywordLexemeAction(TOTAL); }
"partial"               { return KeywordLexemeAction(PARTIAL); }
"if"                    { return KeywordLexemeAction(IF); }
"then"                  { return KeywordLexemeAction(THEN); }
"otherwise"             { return KeywordLexemeAction(OTHERWISE); }
"enum"                  { return KeywordLexemeAction(ENUM); }

/* === Tipos básicos === */
"integer"               { return TypeLexemeAction(TYPE_INTEGER); }
"decimal"               { return TypeLexemeAction(TYPE_DECIMAL); }
"string"                { return TypeLexemeAction(TYPE_STRING); }
"bool"                  { return TypeLexemeAction(TYPE_BOOL); }
"date"                  { return TypeLexemeAction(TYPE_DATE); }
"datetime"              { return TypeLexemeAction(TYPE_DATETIME); }
"uuid"                  { return TypeLexemeAction(TYPE_UUID); }

/* === Operadores === */
"="                     { return OperatorLexemeAction(OP_EQ); }
"!="                    { return OperatorLexemeAction(OP_NEQ); }
">"                     { return OperatorLexemeAction(OP_GT); }
"<"                     { return OperatorLexemeAction(OP_LT); }
">="                    { return OperatorLexemeAction(OP_GTE); }
"<="                    { return OperatorLexemeAction(OP_LTE); }
"+"                     { return OperatorLexemeAction(OP_ADD); }
"-"                     { return OperatorLexemeAction(OP_SUB); }
"*"                     { return OperatorLexemeAction(OP_MUL); }
"/"                     { return OperatorLexemeAction(OP_DIV); }
"and"                   { return OperatorLexemeAction(OP_AND); }
"or"                    { return OperatorLexemeAction(OP_OR); }

/* === Símbolos estructurales === */
"{"                     { return SymbolLexemeAction(OPEN_BRACE); }
"}"                     { return SymbolLexemeAction(CLOSE_BRACE); }
"("                     { return SymbolLexemeAction(OPEN_PAREN); }
")"                     { return SymbolLexemeAction(CLOSE_PAREN); }
":"                     { return SymbolLexemeAction(COLON); }
","                     { return SymbolLexemeAction(COMMA); }
";"                     { return SymbolLexemeAction(SEMICOLON); }
"."                     { return SymbolLexemeAction(DOT); }

/* === Literales === */
{integer}               { return LiteralLexemeAction(LIT_INTEGER); }
{decimal}               { return LiteralLexemeAction(LIT_DECIMAL); }
{string_literal}        { return LiteralLexemeAction(LIT_STRING); }
{bool_literal}          { return LiteralLexemeAction(LIT_BOOL); }

/* === Identificadores === */
{identifier}            { return IdentifierLexemeAction(); }

/* === Comentarios === */
// De bloque
"/*"                    { return EnterCommentLexemeAction(COMMENT_BLOCK); }
<COMMENT_BLOCK>"*/"     { return LeaveCommentLexemeAction(); }
<COMMENT_BLOCK>[^*]+    { return IgnoredLexemeAction(); }
<COMMENT_BLOCK>.        { return IgnoredLexemeAction(); }

// De línea
"//".*                  { return IgnoredLexemeAction(); }

/* === Espacios en blanco === */
{whitespace}            { return IgnoredLexemeAction(); }

/* === EOF y desconocidos === */
<<EOF>>                 { return EOFLexemeAction(); }
.                       { return UnknownLexemeAction(); }

%%

#include "FlexActions.h"
#include "FlexExport.h"
