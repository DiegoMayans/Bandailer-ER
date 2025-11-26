#include "RelationshipAttributeRules.h"
#include "../../../support/logging/Logger.h"
#include <string.h>

static Logger *_logger = NULL;

static void initializeLogger() {
  if (!_logger)
    _logger = createLogger("RelationshipAttributeRules");
}

bool validateRelationshipAttributeModifiers(ValidationContext *ctx,
                                            Relationship *relationship) {
  initializeLogger();

  if (!relationship->attributes) {
    return true; // No attributes to validate
  }

  bool ok = true;

  for (AttributeList *attrList = relationship->attributes; attrList != NULL;
       attrList = attrList->next) {
    Attribute *attr = attrList->attribute;
    if (!attr)
      continue;

    // Check all modifiers on this attribute
    for (ModifierList *modList = attr->modifiers; modList != NULL;
         modList = modList->next) {
      Modifier *mod = modList->modifier;
      if (!mod)
        continue;

      // Check if modifier is PRIMARY
      if (mod->type == MOD_PRIMARY) {
        logError(_logger,
                 "In relationship '%s': attribute '%s' cannot have 'primary' "
                 "modifier - relationship attributes cannot be primary keys",
                 relationship->name, attr->name);
        ctx->hasErrors = true;
        ok = false;
      }
    }
  }

  return ok;
}
