/**
 * @author Matt Ricci
 */

// ALLOW FORMATTING
#ifndef PARSER_H
#define PARSER_H

#include "stdbool.h"

#define MAX_ARGS      5
#define MAX_ARG_LEN   20
#define MAX_VALUE_LEN 50
#define MAX_GROUPS    5

typedef enum {
  ARG_TYPE_BOOL,
  ARG_TYPE_STRING
} ArgType;

typedef struct {
  char name[MAX_ARG_LEN]; // Full name (e.g., "--flag")
  char shorthand;         // Shorthand name (e.g., '-f')
  ArgType type;
  bool required;
  bool provided;
  char value[MAX_VALUE_LEN];
} Argument;

typedef struct {
  int count;
  int indices[MAX_ARGS];
} ExclusionGroup;

typedef enum {
  PARSER_STATUS_OK,
  PARSER_STATUS_ERROR
} ErrorStatus;

typedef struct {
  ErrorStatus status;
  char *msg; // TODO: Make error field fixed string to allow sprintf
} Error;

// TODO: Implement mutual requirement for arguments that must be present together
typedef struct ArgParser {
  bool initialised;
  Error error;
  Argument args[MAX_ARGS];
  int numArgs;
  ExclusionGroup groups[MAX_GROUPS];
  int numGroups;
  int (*addArg)(struct ArgParser *parser, const char *name, char shorthand, ArgType type, bool required);
  int (*addMutexGroup)(struct ArgParser *parser, int indices[], int count);
  bool (*parseArgs)(struct ArgParser *parser, int argc, char *argv[]);
} ArgParser;

ArgParser ArgParser_init();
int ArgParser_addArg(ArgParser *parser, const char *name, char shorthand, ArgType type, bool required);
int ArgParser_addMutexGroup(ArgParser *parser, int indices[], int count);
bool ArgParser_parseArgs(ArgParser *parser, int argc, char *argv[]);

#endif
