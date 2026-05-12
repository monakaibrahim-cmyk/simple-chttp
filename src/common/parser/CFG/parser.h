#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE        256
#define MAX_KEY         64
#define MAX_VALUE       128
#define MAX_ENTRIES     100
#define readonly        const
#define CONFIG_FILE     "./Config.cfg"

typedef struct
{
    char key[MAX_KEY];
    char value[MAX_VALUE];
} Entry;

typedef struct
{
    Entry entries[MAX_ENTRIES];
    int count;
} Config;

static char* config_trim(char* str);
void config_initialize(Config* cfg);
void config_add(Config* cfg, readonly char* key, readonly char* value);
bool config_load(Config* cfg);
static bool config_default(void); 
readonly char* config_get(Config* cfg, readonly char* key);

#endif // PARSER_H

