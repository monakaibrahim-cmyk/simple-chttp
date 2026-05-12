#ifndef PARSER_H
#define PARSER_H

#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_LINE        1024
#define readonly        const   
#define ENV_FILE     ".env"

static char* ltrim(char* s);
static void rtrim(char* s);
static char* unquote(char* s);
void env_load();
static bool config_default(void); 

#endif // PARSER_H
