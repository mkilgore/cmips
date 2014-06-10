/*
 * Copyright (C) 2013 Matt Kilgore
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */
#include "common.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "cmds.h"

static char **parse_line(char *line, int *argc)
{
    char **lines = NULL;
    int sflag = 0, i, len, last;

    len = strlen(line);
    *argc = 0;

    for (i = 0; i < len; i++) {
         if (!isspace(line[i]) && !sflag) {
             (*argc)++;
             lines = realloc(lines, *argc * sizeof(*lines));
             last = i;
             sflag = 1;
         } else if (isspace(line[i]) && sflag) {
             line[i] = '\0';
             lines[*argc - 1] = strdup(line + last);
             sflag = 0;
         }
    }

    if (sflag) {
         lines[*argc - 1] = strdup(line + last);
         sflag = 0;
    }

    return lines;
}

static char *match_func(const char *line, int state)
{
    static int list_index, len;
    const char *name;

    if (!state) {
        list_index = 0;
        len = strlen(line);
    }

    while ((name = cmips_cmds[list_index].cmd_id)) {
        list_index++;

        if (strncmp(name, line, len) == 0)
            return strdup(name);
    }

    return NULL;
}

static char **complete_line(const char *line, int start, int end)
{
    char **matches;

    matches = NULL;

    if (start == 0)
        matches = rl_completion_matches((char *)line, match_func);

    return matches;
}

void run_input_loop(void)
{
    struct cmips_cmd *cmd;
    char *line = NULL, **lines;
    int args, end_flag = 0, i;

    rl_attempted_completion_function = complete_line;

    do {
        if (line)
            free(line);
        line = readline("cmips> ");
        if (line && *line)
            add_history(line);
        else
            continue;

        lines = parse_line(line, &args);

        for (cmd = cmips_cmds; cmd->cmd_id != NULL; cmd++) {
            if (strcmp(lines[0], cmd->cmd_id) == 0) {
                if (cmd->cmd)
                    (cmd->cmd) (args - 1, lines + 1);
                else
                    end_flag = 1; /* Only 'quit' doesn't have a function */
                break;
            }
        }

        for (i = 0; i < args; i++)
            free(lines[i]);
        free(lines);
    } while (!end_flag);

    if (line)
        free(line);
    clear_history();
}
