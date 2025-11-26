#include "GraphvizGenerator.h"
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
static void writeRelationship(FILE *f, Relationship *r, Schema *schema);
static void writeRelationshipAttributeNode(FILE *f, const char *relName,
                                           Attribute *attr);
static const char *typeToString(TypeKind kind);
static const char *cardinalityToString(CardinalityType cardinality);
static char *expressionToString(Expression *expr);
static bool hasModifier(ModifierList *modifiers, ModifierType type);
static bool isAttributeInPrimaryKey(Entity *entity, const char *attrName);
static Entity *lookupEntityInSchema(Schema *schema, const char *entityName);

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
static char *expressionToString(Expression *expr) {
  if (!expr)
    return strdup("");

  char buffer[512];
  char *left, *right, *operand;

  switch (expr->type) {
  case LITERAL_EXPR:
    if (expr->literal) {
      switch (expr->literal->type) {
      case INTEGER_LITERAL:
        snprintf(buffer, sizeof(buffer), "%d", expr->literal->integer);
        break;
      case DECIMAL_LITERAL:
        snprintf(buffer, sizeof(buffer), "%.2f", expr->literal->decimal);
        break;
      case STRING_LITERAL:
        snprintf(buffer, sizeof(buffer), "\\\"%s\\\"", expr->literal->string);
        break;
      case BOOLEAN_LITERAL:
        snprintf(buffer, sizeof(buffer), "%s",
                 expr->literal->boolean ? "true" : "false");
        break;
      default:
        snprintf(buffer, sizeof(buffer), "?");
      }
    }
    return strdup(buffer);

  case IDENTIFIER_EXPR:
    return strdup(expr->identifier ? expr->identifier : "?");

  case ARITHMETIC_EXPR:
    left = expressionToString(expr->arithmetic.left);
    right = expressionToString(expr->arithmetic.right);
    switch (expr->arithmetic.arithmeticOp) {
    case ADDITION:
      snprintf(buffer, sizeof(buffer), "(%s + %s)", left, right);
      break;
    case SUBTRACTION:
      snprintf(buffer, sizeof(buffer), "(%s - %s)", left, right);
      break;
    case MULTIPLICATION:
      snprintf(buffer, sizeof(buffer), "(%s * %s)", left, right);
      break;
    case DIVISION:
      snprintf(buffer, sizeof(buffer), "(%s / %s)", left, right);
      break;
    }
    free(left);
    free(right);
    return strdup(buffer);

  case RELATIONAL_EXPR:
    left = expressionToString(expr->relational.left);
    right = expressionToString(expr->relational.right);
    switch (expr->relational.relationalOp) {
    case EQUAL:
      snprintf(buffer, sizeof(buffer), "%s == %s", left, right);
      break;
    case NOT_EQUAL:
      snprintf(buffer, sizeof(buffer), "%s != %s", left, right);
      break;
    case GREATER:
      snprintf(buffer, sizeof(buffer), "%s \\> %s", left, right);
      break;
    case LESS:
      snprintf(buffer, sizeof(buffer), "%s \\< %s", left, right);
      break;
    case GREATER_EQUAL:
      snprintf(buffer, sizeof(buffer), "%s \\>= %s", left, right);
      break;
    case LESS_EQUAL:
      snprintf(buffer, sizeof(buffer), "%s \\<= %s", left, right);
      break;
    }
    free(left);
    free(right);
    return strdup(buffer);

  case LOGICAL_EXPR:
    left = expressionToString(expr->logical.left);
    right = expressionToString(expr->logical.right);
    switch (expr->logical.logicalOp) {
    case AND:
      snprintf(buffer, sizeof(buffer), "%s \\&\\& %s", left, right);
      break;
    case OR:
      snprintf(buffer, sizeof(buffer), "%s || %s", left, right);
      break;
    }
    free(left);
    free(right);
    return strdup(buffer);

  case LOGICAL_NOT_EXPR:
    operand = expressionToString(expr->logicalNot.operand);
    snprintf(buffer, sizeof(buffer), "!%s", operand);
    free(operand);
    return strdup(buffer);

  case CONDITIONAL_EXPR:
    left = expressionToString(expr->conditional.condition);
    right = expressionToString(expr->conditional.thenExpr);
    operand = expressionToString(expr->conditional.elseExpr);
    snprintf(buffer, sizeof(buffer), "%s ? %s : %s", left, right, operand);
    free(left);
    free(right);
    free(operand);
    return strdup(buffer);

  case PARENTHESIZED_EXPR:
    operand = expressionToString(expr->parenthesized.expression);
    snprintf(buffer, sizeof(buffer), "(%s)", operand);
    free(operand);
    return strdup(buffer);

  default:
    return strdup("?");
  }
}

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

// Helper to lookup an entity in the schema by name.
static Entity *lookupEntityInSchema(Schema *schema, const char *entityName) {
  if (!schema || !entityName)
    return NULL;

  for (Entity *e = schema->entities; e != NULL; e = e->next) {
    if (e->name && strcmp(e->name, entityName) == 0) {
      return e;
    }
  }
  return NULL;
}

// Writes a relationship node and its connections in Chen notation.
static void writeRelationship(FILE *f, Relationship *r, Schema *schema) {
  if (!r)
    return;

  bool isIdentifyingRel = false;

  for (ParticipantList *pl = r->participants; pl != NULL; pl = pl->next) {
    Participant *p = pl->participant;
    if (!p || !p->entityName)
      continue;

    // Check if entity is weak and has total participation
    Entity *entity = lookupEntityInSchema(schema, p->entityName);
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

bool generateGraphviz(Program *program, const char *outputFilename) {
  if (!program || !outputFilename) {
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

  writeHeader(f, schema->name);

  // 1. Generate entity nodes
  logDebugging(_logger, "Generating entity nodes...");
  for (Entity *e = schema->entities; e != NULL; e = e->next) {
    writeEntity(f, e);
  }

  fprintf(f, "\n");

  // 2. Generate relationship nodes and edges
  logDebugging(_logger, "Generating relationship nodes...");
  for (Relationship *r = schema->relationships; r != NULL; r = r->next) {
    writeRelationship(f, r, schema);
  }

  fprintf(f, "\n");

  // 3. Generate inheritance edges
  logDebugging(_logger, "Generating inheritance edges...");
  for (Entity *e = schema->entities; e != NULL; e = e->next) {
    if (e->parent) {
      // Arrow from child to parent with special styling
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
