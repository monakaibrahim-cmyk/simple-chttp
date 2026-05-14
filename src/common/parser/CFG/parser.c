#include "parser.h"

#ifdef ENABLE_LOGGING
#include "common/Logs/logs.h"
#endif

#include <stdlib.h>
#include <string.h>

static bool config_default(void);
static char* config_trim(char* str);

static char* config_trim(char* str)
{
    char* end;

    while (isspace((unsigned char)*str))
    {
        str++;
    }

    if (*str == '\0')
    {
        return str;
    }

    end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end))
    {
        end--;
    }

    end[1] = '\0';

    return str;
}

void config_initialize(Config *cfg)
{
    cfg->count = 0;
}

void config_add(Config *cfg, const char *key, const char *value)
{
    if (cfg->count >= MAX_ENTRIES)
    {
        return;
    }

    strncpy(cfg->entries[cfg->count].key, key, MAX_KEY - 1);
    cfg->entries[cfg->count].key[MAX_KEY - 1] = '\0';

    strncpy(cfg->entries[cfg->count].value, value, MAX_VALUE - 1);
    cfg->entries[cfg->count].value[MAX_VALUE - 1] = '\0';

    cfg->count++;
}

bool config_load(Config* cfg)
{
    FILE* file = fopen(CONFIG_FILE, "r");
    
    if (!file)
    {
        if (!config_default())
        {
#ifdef ENABLE_LOGGING
            LOG_ERROR("Failed to create default Config.");
#endif
            exit(EXIT_FAILURE);
        }

        file = fopen(CONFIG_FILE, "r");

        if (!file)
        {
#ifdef ENABLE_LOGGING
            LOG_ERROR("Failed to reopen Config file.");
#endif
            exit(EXIT_FAILURE);
        }
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), file))
    {
        char* trimmed = config_trim(line);

        if (*trimmed == '\0' || *trimmed == '#')
        {
            continue;
        }

        char* eq = strchr(trimmed, '=');

        if (!eq)
        {
            continue;
        }

        *eq = '\0';
        
        char* key = config_trim(trimmed);
        char* value = config_trim(eq + 1);

        config_add(cfg, key, value);
    }

    fclose(file);

    return true;
}

static bool config_default(void)
{
    FILE* file = fopen(CONFIG_FILE, "w");

    if (file == NULL)
    {
#ifdef ENABLE_LOGGING
        LOG_ERROR("Error Creating Config File!");
#endif
        return false;
    }

    fprintf(file, "# HTTP Configuration\n");
    fprintf(file, "host = 0.0.0.0\n");
    fprintf(file, "port = 8000\n");
    fclose(file);

    return true;
}

const char* config_get(Config* cfg, const char* key)
{
    for (int i = 0; i < cfg->count; i++)
    {
        if (strcmp(cfg->entries[i].key, key) == 0)
        {
            return cfg->entries[i].value;
        }
    }

    return NULL;
}
