#include "parser.h"
#include "common/Logs/logs.h"

static char* ltrim(char* s)
{
    while (isspace((unsigned char)*s))
    {
        s++;
    }

    return s;
}

static void rtrim(char* s)
{
    if (!s || *s == '\0')
    {
        return;
    }

    char* end = s + strlen(s) - 1;

    while (end >= s && isspace((unsigned char)*end))
    {
        *end = '\0';
        end--;
    }
}

static char* unquote(char* s)
{
    size_t length = strlen(s);

    if (length >= 2)
    {
        if ((s[0] == '"' && s[length - 1] == '"') || (s[0] == '\'' &&  s[length - 1] == '\''))
        {
            s[length - 1] = '\0';
            return s + 1;
        }
    }

    return s;
}

void env_load()
{
    FILE* file = fopen(ENV_FILE, "r");

    if (!file)
    {
        if (!config_default())
        {
            LOG_ERROR("Failed to create default .env file.");
            exit(EXIT_FAILURE);
        }

        file = fopen(ENV_FILE, "r");

        if (!file)
        {
            LOG_ERROR("Failed to reopen .env file.");
            exit(EXIT_FAILURE);
        }
    }

    char line[MAX_LINE];

    while (fgets(line, sizeof(line), file))
    {
        line[strcspn(line, "\r\n")] = 0;

        char *ptr = ltrim(line);

        if (*ptr == '\0' || *ptr == '#')
        {
            continue;
        }

        char *eq = strchr(ptr, '=');

        if (!eq)
        {
            continue;
        }

        *eq = '\0';

        char *key = ltrim(ptr);

        rtrim(key);

        char *value = ltrim(eq + 1);

        rtrim(value);

        value = unquote(value);

        if (strlen(key) > 0)
        {
#ifdef _WIN32
            _putenv_s(key, value);
#else
            setenv(key, value, 1);
#endif
        }
    }

    fclose(file);
}

static bool config_default(void)
{
    FILE* file = fopen(ENV_FILE, "w");

    if (file == NULL)
    {
        LOG_ERROR("Error Creating .env file!");
        return false;
    }

    fprintf(file, "# HTTP Configuration\n");
    fprintf(file, "HOST=0.0.0.0\n");
    fprintf(file, "PORT=8000\n");
    fclose(file);

    return true;
}

