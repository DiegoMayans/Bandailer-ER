%top{

#include "../../support/type/CompilationStatus.h"
#include "../../support/type/FlexContext.h"
#include "../../support/type/TokenLabel.h"
#include "../syntactic-analysis/AbstractSyntaxTree.h"
#include "../syntactic-analysis/BisonParser.h"

}

%{

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
 * Flex contexts (a.k.a. start conditions).
 */
%x COMMENT_BLOCK

/**
 * Reusable patterns.
 */
whitespace      [ \t\r\n]+
digit           [0-9]
integer         {digit}+
decimal         {digit}+"."{digit}+
identifier      [A-Za-z_][A-Za-z0-9_]*
string_literal  \"([^\"\\]|\\.)*\"
bool_literal    (true|false)

%%

"schema"                { return KeywordLexemeAction(SCHEMA); }
"entity"                { return KeywordLexemeAction(ENTITY); }
"relationship"          { return KeywordLexemeAction(RELATIONSHIP); }
"primary"               { return KeywordLexemeAction(PRIMARY_MOD); }
"PRIMARY"               { return KeywordLexemeAction(PRIMARY_COMPOSITE); }
"unique"                { return KeywordLexemeAction(UNIQUE); }
"derived"               { return KeywordLexemeAction(DERIVED); }
"not"                   { return KeywordLexemeAction(NOT); }
"null"                  { return KeywordLexemeAction(NULL_TOK); }
"default"               { return KeywordLexemeAction(DEFAULT); }
"assert"                { return KeywordLexemeAction(ASSERT); }
"total"                 { return KeywordLexemeAction(TOTAL); }
"partial"               { return KeywordLexemeAction(PARTIAL); }
"IF"                    { return KeywordLexemeAction(IF); }
"THEN"                  { return KeywordLexemeAction(THEN); }
"OTHERWISE"             { return KeywordLexemeAction(OTHERWISE); }
"enum"                  { return KeywordLexemeAction(ENUM); }

"integer"               { return TypeLexemeAction(TOK_INTEGER_TYPE); }
"decimal"               { return TypeLexemeAction(TOK_DECIMAL_TYPE); }
"string"                { return TypeLexemeAction(TOK_STRING_TYPE); }
"bool"                  { return TypeLexemeAction(TOK_BOOL_TYPE); }
"date"                  { return TypeLexemeAction(TOK_DATE_TYPE); }
"datetime"              { return TypeLexemeAction(TOK_DATETIME_TYPE); }
"uuid"                  { return TypeLexemeAction(TOK_UUID_TYPE); }

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
"AND"                   { return OperatorLexemeAction(OP_AND); }
"OR"                    { return OperatorLexemeAction(OP_OR); }
"NOT"                   { return OperatorLexemeAction(OP_NOT); }
"->"                    { return OperatorLexemeAction(OP_ARROW); }

"{"                     { return SymbolLexemeAction(OPEN_BRACE); }
"}"                     { return SymbolLexemeAction(CLOSE_BRACE); }
"("                     { return SymbolLexemeAction(OPEN_PAREN); }
")"                     { return SymbolLexemeAction(CLOSE_PAREN); }
":"                     { return SymbolLexemeAction(COLON); }
","                     { return SymbolLexemeAction(COMMA); }
";"                     { return SymbolLexemeAction(SEMICOLON); }
"."                     { return SymbolLexemeAction(DOT); }

{integer}               { return LiteralLexemeAction(LIT_INTEGER); }
{decimal}               { return LiteralLexemeAction(LIT_DECIMAL); }
{string_literal}        { return LiteralLexemeAction(LIT_STRING); }
{bool_literal}          { return LiteralLexemeAction(LIT_BOOL); }

{identifier}            { return IdentifierLexemeAction(); }

"/*"                    { return EnterCommentLexemeAction(COMMENT_BLOCK); }
<COMMENT_BLOCK>"*/"     { return LeaveCommentLexemeAction(); }
<COMMENT_BLOCK>[^*]+    { return IgnoredLexemeAction(); }
<COMMENT_BLOCK>.        { return IgnoredLexemeAction(); }

"//".*                  { return IgnoredLexemeAction(); }

{whitespace}            { return IgnoredLexemeAction(); }
.                       { return UnknownLexemeAction(); }

<<EOF>>                 { return EOFLexemeAction(); }

%%

#include "FlexActions.h"
#include "FlexExport.h"
