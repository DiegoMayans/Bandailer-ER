#include "AttributeModifierRules.h"
#include "../../../support/logging/Logger.h"
#include <string.h>

static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("AttributeModifierRules");
}

static const char* typeKindToString(TypeKind kind) {
  switch (kind) {
    case TYPE_INTEGER: return "integer";
    case TYPE_DECIMAL: return "decimal";
    case TYPE_STRING: return "string";
    case TYPE_BOOL: return "boolean";
    case TYPE_DATE: return "date";
    case TYPE_DATETIME: return "datetime";
    case TYPE_UUID: return "uuid";
    case TYPE_ENUM: return "enum";
    default: return "unknown";
  }
}

static bool isLiteralType(TypeKind kind) {
  return kind == TYPE_INTEGER || kind == TYPE_DECIMAL || 
         kind == TYPE_STRING || kind == TYPE_BOOL;
}

static bool isPrimaryKeyType(TypeKind kind) {
  return kind == TYPE_UUID || kind == TYPE_STRING || kind == TYPE_INTEGER;
}

bool validateDefaultModifierTypes(ValidationContext *ctx, Entity *entity) {
  initializeLogger();

  if (!entity->attributes) {
    return true; // No attributes to validate
  }

  bool ok = true;

  for (AttributeList *attrList = entity->attributes; attrList != NULL;
       attrList = attrList->next) {
    Attribute *attr = attrList->attribute;
    if (!attr || !attr->type)
      continue;

    // Check if this attribute has a default modifier
    bool hasDefaultModifier = false;
    for (ModifierList *modList = attr->modifiers; modList != NULL;
         modList = modList->next) {
      if (modList->modifier && modList->modifier->type == MOD_DEFAULT) {
        hasDefaultModifier = true;
        break;
      }
    }

    if (!hasDefaultModifier)
      continue;

    // Validate that the type is a literal type
    if (!isLiteralType(attr->type->kind)) {
      logError(_logger,
               "In entity '%s': attribute '%s' has 'default' modifier but type "
               "'%s' is not allowed - only integer, decimal, string, and "
               "boolean types are permitted for default values",
               entity->name, attr->name, typeKindToString(attr->type->kind));
      ctx->hasErrors = true;
      ok = false;
    }
  }

  return ok;
}

bool validatePrimaryModifierTypes(ValidationContext *ctx, Entity *entity) {
  initializeLogger();

  if (!entity->attributes) {
    return true; // No attributes to validate
  }

  bool ok = true;

  for (AttributeList *attrList = entity->attributes; attrList != NULL;
       attrList = attrList->next) {
    Attribute *attr = attrList->attribute;
    if (!attr || !attr->type)
      continue;

    // Check if this attribute has a primary modifier
    bool hasPrimaryModifier = false;
    for (ModifierList *modList = attr->modifiers; modList != NULL;
         modList = modList->next) {
      if (modList->modifier && modList->modifier->type == MOD_PRIMARY) {
        hasPrimaryModifier = true;
        break;
      }
    }

    if (!hasPrimaryModifier)
      continue;

    // Validate that the type is allowed for primary keys
    if (!isPrimaryKeyType(attr->type->kind)) {
      logError(_logger,
               "In entity '%s': attribute '%s' has 'primary' modifier but type "
               "'%s' is not allowed - only uuid, string, and integer types are "
               "permitted for primary keys",
               entity->name, attr->name, typeKindToString(attr->type->kind));
      ctx->hasErrors = true;
      ok = false;
    }
  }

  return ok;
}
