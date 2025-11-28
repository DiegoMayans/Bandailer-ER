#include "GraphvizGenerator.h"
#include "../domain-specific//SymbolTable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* MODULE INTERNAL STATE */

static Logger *_logger = NULL;

/** Shutdown module's internal state. */
void _shutdownGraphvizGeneratorModule() {
  if (_logger != NULL) {
    logDebugging(_logger, "Destroying module: GraphvizGenerator...");
    destroyLogger(_logger);
    _logger = NULL;
  }
}

ModuleDestructor initializeGraphvizGeneratorModule() {
  _logger = createLogger("GraphvizGenerator");
  return _shutdownGraphvizGeneratorModule;
}

/* PRIVATE FUNCTIONS */

static void writeHeader(FILE *f, const char *schemaName);
static void writeFooter(FILE *f);
static void writeEntity(FILE *f, Entity *e);
static void writeAttributeNode(FILE *f, Entity *e, Attribute *attr);
static void writeRelationship(FILE *f, Relationship *r, SymbolTable *symbolTable);
static void writeRelationshipAttributeNode(FILE *f, const char *relName,
                                           Attribute *attr);
static const char *typeToString(TypeKind kind);
static const char *cardinalityToString(CardinalityType cardinality);
static char *expressionToString(Expression *expr);
static bool hasModifier(ModifierList *modifiers, ModifierType type);
static bool isAttributeInPrimaryKey(Entity *entity, const char *attrName);


// Writes the DOT file header with graph configuration.
static void writeHeader(FILE *f, const char *schemaName) {
  fprintf(f, "digraph %s {\n", schemaName ? schemaName : "ERModel");

  fprintf(f, "    layout=neato;\n");
  fprintf(f, "    overlap=false;\n");
  fprintf(f, "    splines=curved;\n");

  fprintf(f, "    node [style=filled, fillcolor=white];\n");
  fprintf(f, "    edge [fontsize=10, len=1.5];\n\n");
}

// Writes the DOT file footer (closing brace).
static void writeFooter(FILE *f) { fprintf(f, "}\n"); }

// Converts a TypeKind enum to its string representation.
static const char *typeToString(TypeKind kind) {
  switch (kind) {
  case TYPE_INTEGER:
    return "integer";
  case TYPE_DECIMAL:
    return "decimal";
  case TYPE_STRING:
    return "string";
  case TYPE_BOOL:
    return "bool";
  case TYPE_DATE:
    return "date";
  case TYPE_DATETIME:
    return "datetime";
  case TYPE_UUID:
    return "uuid";
  case TYPE_ENUM:
    return "enum";
  default:
    return "unknown";
  }
}

// Converts a CardinalityType enum to its string representation.
static const char *cardinalityToString(CardinalityType cardinality) {
  switch (cardinality) {
  case ONE:
    return "1";
  case MANY:
    return "N";
  default:
    return "?";
  }
}

// Checks if a modifier list contains a specific modifier type.
static bool hasModifier(ModifierList *modifiers, ModifierType type) {
  for (ModifierList *ml = modifiers; ml != NULL; ml = ml->next) {
    if (ml->modifier && ml->modifier->type == type) {
      return true;
    }
  }
  return false;
}

// Converts an expression AST to a string representation.
typedef struct {
  char *data;
  size_t length;
  size_t capacity;
} StringBuilder;

// Helper to check if an attribute is in the PRIMARY(...) declaration
static bool isAttributeInPrimaryKey(Entity *entity, const char *attrName) {
  if (!entity || !entity->primaryKey || !attrName)
    return false;

  for (QualifiedIdentifierList *qidList = entity->primaryKey->attributes;
       qidList != NULL; qidList = qidList->next) {
    QualifiedIdentifier *qid = qidList->qid;
    if (!qid)
      continue;

    // Check if this qualified identifier matches the attribute
    // It can be either "attrName" or "EntityName:attrName"
    if (qid->attribute && strcmp(qid->attribute, attrName) == 0) {
      // If entity is NULL or matches current entity, it's a match
      if (!qid->entity || strcmp(qid->entity, entity->name) == 0) {
        return true;
      }
    }
  }
  return false;
}

// Helper to write an attribute node with proper styling.
static void writeAttributeNode(FILE *f, Entity *e, Attribute *attr) {
  if (!attr || !attr->name || !e)
    return;

  // Check both MOD_PRIMARY modifier and PRIMARY(...) declaration
  bool isPk = hasModifier(attr->modifiers, MOD_PRIMARY) ||
              isAttributeInPrimaryKey(e, attr->name);

  // Check for derived modifier
  bool isDerived = hasModifier(attr->modifiers, MOD_DERIVED);

  // Get type string
  const char *typeStr = "unknown";
  if (attr->type) {
    typeStr = typeToString(attr->type->kind);
  }

  // Determine peripheries (double border for derived attributes)
  int peripheries = isDerived ? 2 : 1;

  fprintf(f,
          "    attr_%s_%s [shape=ellipse, style=solid, peripheries=%d, "
          "fontname=\"Arial\", fontsize=10, label=<",
          e->name, attr->name, peripheries);

  if (isPk)
    fprintf(f, "<u>");
  fprintf(f, "%s : %s", attr->name, typeStr);
  if (isPk)
    fprintf(f, "</u>");

  fprintf(f, ">];\n");

  fprintf(f, "    %s -> attr_%s_%s [dir=none, color=\"gray40\"];\n", e->name,
          e->name, attr->name);
}

// Writes an entity node in Chen notation (oval shape).
static void writeEntity(FILE *f, Entity *e) {
  if (!e)
    return;

  const char *style = e->weak ? "dashed" : "solid";

  fprintf(f,
          "    %s [shape=ellipse, style=\"%s\", fontname=\"Arial Bold\", "
          "label=\"%s\"];\n",
          e->name, style, e->name);

  // Generate attribute nodes and connect them to the entity
  if (e->attributes) {
    for (AttributeList *al = e->attributes; al != NULL; al = al->next) {
      if (al->attribute) {
        writeAttributeNode(f, e, al->attribute);
      }
    }
  }

  // Generate assertion nodes if present
  if (e->assertions) {
    int assertCount = 0;
    for (Assertion *as = e->assertions; as != NULL; as = as->next) {
      char *exprStr = expressionToString(as->condition);
      fprintf(
          f,
          "    assert_%s_%d [shape=note, style=filled, fillcolor=lightyellow, "
          "fontsize=9, label=\"assert: %s\"];\n",
          e->name, assertCount, exprStr);
      fprintf(f,
              "    %s -> assert_%s_%d [dir=none, style=dotted, "
              "color=\"gray60\"];\n",
              e->name, e->name, assertCount);
      free(exprStr);
      assertCount++;
    }
  }
}

// Helper to write a relationship attribute node.
static void writeRelationshipAttributeNode(FILE *f, const char *relName,
                                           Attribute *attr) {
  if (!attr || !attr->name)
    return;

  // Check for derived modifier
  bool isDerived = hasModifier(attr->modifiers, MOD_DERIVED);

  // Get type string
  const char *typeStr = "unknown";
  if (attr->type) {
    typeStr = typeToString(attr->type->kind);
  }

  // Determine peripheries (double border for derived attributes)
  int peripheries = isDerived ? 2 : 1;

  fprintf(f,
          "    attr_rel_%s_%s [shape=ellipse, peripheries=%d, fontsize=9, "
          "label=\"%s : %s\"];\n",
          relName, attr->name, peripheries, attr->name, typeStr);
  fprintf(f,
          "    %s_rel -> attr_rel_%s_%s [dir=none, style=dashed, "
          "color=\"gray40\"];\n",
          relName, relName, attr->name);
}



// Writes a relationship node and its connections in Chen notation.
static void writeRelationship(FILE *f, Relationship *r, SymbolTable *symbolTable) {
  if (!r)
    return;

  bool isIdentifyingRel = false;

  for (ParticipantList *pl = r->participants; pl != NULL; pl = pl->next) {
    Participant *p = pl->participant;
    if (!p || !p->entityName)
      continue;

    // Check if entity is weak and has total participation
    Entity *entity = lookupEntity(symbolTable, p->entityName);
    if (entity && entity->weak && p->participation &&
        p->participation->type == PARTICIPATION_TOTAL) {
      isIdentifyingRel = true;
      break;
    }
  }

  int peripheries = isIdentifyingRel ? 2 : 1;
  fprintf(f,
          "    %s_rel [shape=diamond, style=filled, fillcolor=lightgray, "
          "peripheries=%d, label=\"%s\"];\n",
          r->name, peripheries, r->name);

  if (r->attributes) {
    for (AttributeList *al = r->attributes; al != NULL; al = al->next) {
      if (al->attribute) {
        writeRelationshipAttributeNode(f, r->name, al->attribute);
      }
    }
  }

  // Connect entities to the relationship diamond
  for (ParticipantList *pl = r->participants; pl != NULL; pl = pl->next) {
    Participant *p = pl->participant;
    if (!p || !p->entityName)
      continue;

    const char *card = cardinalityToString(p->cardinality);

    // Determine edge style based on participation
    const char *color = "black";
    if (p->participation && p->participation->type == PARTICIPATION_TOTAL) {
      color = "black:black";
    }

    fprintf(f,
            "    %s -> %s_rel [taillabel=\"%s\", labeldistance=1.5, dir=none, "
            "color=\"%s\"];\n",
            p->entityName, r->name, card, color);
  }
}

/* PUBLIC FUNCTIONS */

bool generateGraphviz(Program *program, SymbolTable *symbolTable, const char *outputFilename) {
  if (!program || !symbolTable || !outputFilename) {
    logError(_logger, "Invalid arguments to generateGraphviz");
    return false;
  }

  logDebugging(_logger, "Generating Graphviz DOT file: %s", outputFilename);

  FILE *f = fopen(outputFilename, "w");
  if (!f) {
    logError(_logger, "Failed to open output file: %s", outputFilename);
    return false;
  }

  Schema *schema = program->schema;

  if (!schema) {
    logWarning(_logger, "No schema found in program");
    writeHeader(f, "Empty");
    writeFooter(f);
    fclose(f);
    return true;
  }

  logDebugging(_logger, "Processing schema: %s", schema->name);

  writeHeader(f, schema->name);

  // 1. Generate entity nodes
  for (Entity *e = schema->entities; e != NULL; e = e->next) {
    writeEntity(f, e);
  }

  fprintf(f, "\n");

  // 2. Generate relationship nodes and edges
  for (Relationship *r = schema->relationships; r != NULL; r = r->next) {
    writeRelationship(f, r, symbolTable);
  }

  fprintf(f, "\n");

  // 3. Generate inheritance edges
  for (Entity *e = schema->entities; e != NULL; e = e->next) {
    if (e->parent) {
      fprintf(f, "    %s -> %s [arrowhead=onormal, label=\"is-a\"];\n", e->name,
              e->parent);
    }
  }

  writeFooter(f);
  fclose(f);

  logInformation(_logger, "Successfully generated Graphviz DOT file: %s",
                 outputFilename);
  return true;
}

bool generateImageFromDot(const char *dotFilename,
                          const char *outputImageFilename, const char *format) {
  if (!dotFilename || !outputImageFilename || !format) {
    logError(_logger, "Invalid arguments for image generation.");
    return false;
  }

  logDebugging(_logger, "Executing Graphviz command to generate %s...", format);

  // Uses dot command to generate image from DOT file.
  char command[1024];

  int wrote = snprintf(command, sizeof(command), "dot -T%s %s -o %s", format,
                       dotFilename, outputImageFilename);

  if (wrote < 0 || wrote >= sizeof(command)) {
    logError(_logger, "Command string too long or encoding error.");
    return false;
  }

  logDebugging(_logger, "System command: %s", command);

  int result = system(command);

  if (result != 0) {
    logError(_logger, "Graphviz command failed. 'dot' might not be installed "
                      "or not in PATH.");
    logError(_logger, "Command returned code: %d", result);
    return false;
  }

  logInformation(_logger, "Image generated successfully: %s",
                 outputImageFilename);
  return true;
}

/* STRING BUILDER FUNCTIONS */

static void sb_init(StringBuilder *sb) {
  sb->capacity = 256;
  sb->length = 0;
  sb->data = malloc(sb->capacity);
  if (sb->data)
    sb->data[0] = '\0';
}

static void sb_append(StringBuilder *sb, const char *str) {
  if (!sb->data || !str)
    return;
  size_t len = strlen(str);
  if (sb->length + len >= sb->capacity) {
    size_t new_capacity = sb->capacity * 2;
    if (new_capacity < sb->length + len + 1)
      new_capacity = sb->length + len + 1;
    char *new_data = realloc(sb->data, new_capacity);
    if (!new_data)
      return; // Allocation failed
    sb->data = new_data;
    sb->capacity = new_capacity;
  }
  strcpy(sb->data + sb->length, str);
  sb->length += len;
}

static void sb_printf(StringBuilder *sb, const char *format, ...) {
  if (!sb->data)
    return;
  va_list args;
  va_start(args, format);

  char temp[128];
  int len = vsnprintf(temp, sizeof(temp), format, args);
  va_end(args);

  if (len < 0)
    return;

  if ((size_t)len < sizeof(temp)) { // If it fits in temp, append it
    sb_append(sb, temp);
  } else { // If it's larger, we need to allocate enough space
    char *dyn_buf = malloc(len + 1);
    if (dyn_buf) {
      va_start(args, format);
      vsnprintf(dyn_buf, len + 1, format, args);
      va_end(args);
      sb_append(sb, dyn_buf);
      free(dyn_buf);
    }
  }
}

static char *sb_tostring(StringBuilder *sb) {
  if (!sb->data)
    return strdup("");
  return strdup(sb->data);
}

static void sb_destroy(StringBuilder *sb) {
  if (sb->data) {
    free(sb->data);
    sb->data = NULL;
  }
  sb->length = 0;
  sb->capacity = 0;
}

static void buildExpressionString(StringBuilder *sb, Expression *expr) {
  if (!expr)
    return;

  switch (expr->type) {
  case LITERAL_EXPR:
    if (expr->literal) {
      switch (expr->literal->type) {
      case INTEGER_LITERAL:
        sb_printf(sb, "%d", expr->literal->integer);
        break;
      case DECIMAL_LITERAL:
        sb_printf(sb, "%.2f", expr->literal->decimal);
        break;
      case STRING_LITERAL:
        sb_append(sb, "\\\"");
        sb_append(sb, expr->literal->string);
        sb_append(sb, "\\\"");
        break;
      case BOOLEAN_LITERAL:
        sb_append(sb, expr->literal->boolean ? "true" : "false");
        break;
      default:
        sb_append(sb, "?");
      }
    }
    break;

  case IDENTIFIER_EXPR:
    sb_append(sb, expr->identifier ? expr->identifier : "?");
    break;

  case ARITHMETIC_EXPR:
    sb_append(sb, "(");
    buildExpressionString(sb, expr->arithmetic.left);
    switch (expr->arithmetic.arithmeticOp) {
    case ADDITION:
      sb_append(sb, " + ");
      break;
    case SUBTRACTION:
      sb_append(sb, " - ");
      break;
    case MULTIPLICATION:
      sb_append(sb, " * ");
      break;
    case DIVISION:
      sb_append(sb, " / ");
      break;
    }
    buildExpressionString(sb, expr->arithmetic.right);
    sb_append(sb, ")");
    break;

  case RELATIONAL_EXPR:
    buildExpressionString(sb, expr->relational.left);
    switch (expr->relational.relationalOp) {
    case EQUAL:
      sb_append(sb, " == ");
      break;
    case NOT_EQUAL:
      sb_append(sb, " != ");
      break;
    case GREATER:
      sb_append(sb, " \\> ");
      break;
    case LESS:
      sb_append(sb, " \\< ");
      break;
    case GREATER_EQUAL:
      sb_append(sb, " \\>= ");
      break;
    case LESS_EQUAL:
      sb_append(sb, " \\<= ");
      break;
    }
    buildExpressionString(sb, expr->relational.right);
    break;

  case LOGICAL_EXPR:
    buildExpressionString(sb, expr->logical.left);
    switch (expr->logical.logicalOp) {
    case AND:
      sb_append(sb, " \\&\\& ");
      break;
    case OR:
      sb_append(sb, " || ");
      break;
    }
    buildExpressionString(sb, expr->logical.right);
    break;

  case LOGICAL_NOT_EXPR:
    sb_append(sb, "!");
    buildExpressionString(sb, expr->logicalNot.operand);
    break;

  case CONDITIONAL_EXPR:
    buildExpressionString(sb, expr->conditional.condition);
    sb_append(sb, " ? ");
    buildExpressionString(sb, expr->conditional.thenExpr);
    sb_append(sb, " : ");
    buildExpressionString(sb, expr->conditional.elseExpr);
    break;

  case PARENTHESIZED_EXPR:
    sb_append(sb, "(");
    buildExpressionString(sb, expr->parenthesized.expression);
    sb_append(sb, ")");
    break;

  default:
    sb_append(sb, "?");
  }
}

// Converts an expression AST to a string representation.
static char *expressionToString(Expression *expr) {
  if (!expr)
    return strdup("");

  StringBuilder sb;
  sb_init(&sb);
  buildExpressionString(&sb, expr);
  char *result = sb_tostring(&sb);
  sb_destroy(&sb);
  return result;
}
