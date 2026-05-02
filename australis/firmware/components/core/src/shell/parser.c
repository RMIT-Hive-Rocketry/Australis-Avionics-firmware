/***********************************************************************************
 * @file        shell.c                                                            *
 * @author      Matt Ricci                                                         *
 * @addtogroup  Shell                                                              *
 ***********************************************************************************/

#include "parser.h"

#include <string.h>
#include <stdbool.h>

/* =============================================================================== */
/**
 * @brief
 *
 * @param parser Pointer to the Praser structure.
 * @return void
 **
 * =============================================================================== */
ArgParser ArgParser_init() {
  ArgParser parser;
  parser.initialised   = true;
  parser.error.status  = PARSER_STATUS_OK;
  parser.error.msg     = "";
  parser.numArgs       = 0;
  parser.numGroups     = 0;
  parser.addArg        = ArgParser_addArg;
  parser.addMutexGroup = ArgParser_addMutexGroup;
  parser.parseArgs     = ArgParser_parseArgs;
  return parser;
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * =============================================================================== */
int ArgParser_addArg(ArgParser *parser, const char *name, char shorthand, ArgType type, bool required) {
  if (parser->numArgs >= MAX_ARGS) {
    parser->error.status = PARSER_STATUS_ERROR;
    parser->error.msg    = "Error: Too many arguments";
    return -1;
  }
  Argument *arg = &parser->args[parser->numArgs++];
  strncpy(arg->name, name, MAX_ARG_LEN - 1);
  arg->name[MAX_ARG_LEN - 1] = '\0';
  arg->shorthand             = shorthand;
  arg->type                  = type;
  arg->required              = required;
  arg->provided              = false;
  arg->value[0]              = '\0';
  return parser->numArgs - 1;
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * =============================================================================== */
int ArgParser_addMutexGroup(ArgParser *parser, int indices[], int count) {
  if (parser->numGroups >= MAX_GROUPS) {
    parser->error.status = PARSER_STATUS_ERROR;
    parser->error.msg    = "Error: Too many exclusion groups";
    return -1;
  }
  ExclusionGroup *group = &parser->groups[parser->numGroups++];
  group->count          = count;
  for (int i = 0; i < count; i++) {
    group->indices[i] = indices[i];
  }
  return parser->numGroups - 1;
}

/* =============================================================================== */
/**
 * @brief
 *
 * @param
 *
 * @return
 **
 * =============================================================================== */
bool ArgParser_parseArgs(ArgParser *parser, int argc, char *argv[]) {
  for (int i = 0; i < argc; i++) {
    bool matched = false;
    for (int j = 0; j < parser->numArgs; j++) {
      Argument *arg = &parser->args[j];
      // Match full name or shorthand
      if (strcmp(argv[i], arg->name) == 0 || (strlen(argv[i]) == 2 && argv[i][0] == '-' && argv[i][1] == arg->shorthand)) {
        if (arg->type == ARG_TYPE_BOOL) {
          arg->provided = true;
        } else if (arg->type == ARG_TYPE_STRING) {
          if (i + 1 < argc) {
            strncpy(arg->value, argv[++i], MAX_VALUE_LEN - 1);
            arg->value[MAX_VALUE_LEN - 1] = '\0';
            arg->provided                 = true;
          } else {
            // TODO: Make sprintf compatible for arg name print
            parser->error.status = PARSER_STATUS_ERROR;
            parser->error.msg    = "Error: Missing value for argument";
            return false;
          }
        }
        matched = true;
        break;
      }
    }
    if (!matched) {
      // TODO: Make sprintf compatible for arg name print
      parser->error.status = PARSER_STATUS_ERROR;
      parser->error.msg    = "Error: Unknown argument";
      return false;
    }
  }

  // Check for required arguments
  for (int i = 0; i < parser->numArgs; i++) {
    Argument *arg = &parser->args[i];
    if (arg->required && !arg->provided) {
      parser->error.status = PARSER_STATUS_ERROR;
      parser->error.msg    = "Error: Missing required argument";
      return false;
    }
  }

  // Check for mutually exclusive arguments
  for (int i = 0; i < parser->numGroups; i++) {
    ExclusionGroup *group = &parser->groups[i];
    int count             = 0;
    for (int j = 0; j < group->count; j++) {
      if (parser->args[group->indices[j]].provided) {
        count++;
      }
    }
    if (count > 1) {
      parser->error.status = PARSER_STATUS_ERROR;
      parser->error.msg    = "Error: Mutually exclusive arguments provided in group";
      return false;
    }
  }

  return true;
}
