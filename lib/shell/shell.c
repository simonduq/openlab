#include <stdio.h>
#include <string.h>
#include "platform.h"
#include "event.h"
#include "shell.h"


// Includes '\0' character
#define IS_WHITESPACE(character) (*((unsigned char *)(character)) <= ' ')

static void shell_print_help(struct shell_command *cmds);
static int shell_call_handler(struct shell_command *cmds, int argc, char **argv);
static struct shell_command *shell_find_cmd(struct shell_command *cmds, char *cmd);
static void shell_print_cmd_help(const char *name, const char *desc);

static void char_handler_irq(handler_arg_t arg, uint8_t c);
static void parse_command(void *command_buffer);

struct {
    struct shell_command *commands;
    int with_prompt;
} state = {NULL, 1};


void shell_init(struct shell_command *cmds, int with_prompt)
{
    state.commands = cmds;
    state.with_prompt = with_prompt;

    uart_set_rx_handler(uart_print, char_handler_irq, NULL);
    if (state.with_prompt) {
        printf("> ");
    }
}


int shell_handle_line(struct shell_command *cmds, char *line)
{
    int argc = 0;
    char *argv[MAX_ARGS_NUM + 1];
    argc = shell_parse_line(line, argv, MAX_ARGS_NUM);
    return shell_call_handler(cmds, argc, argv);
}


int shell_parse_line(char *line, char **argv, int max_argv)
{
    int argc = 0;
    memset(argv, (int)NULL, max_argv + 1);

    char *cur = line;
    while (*cur) {
        // Skip spaces
        if (IS_WHITESPACE(cur)) {
            *(cur++) = '\0';
            continue;
        }

        // We will overflow on this one
        if (argc >= max_argv) {
            printf("Too many arguments: Max %u\n", max_argv);
            argc = 0;
            break;
        }

        // Store word, '\0' will be put on next loop
        argv[argc++] = cur;
        while (!IS_WHITESPACE(cur))
            cur++;
    }

    /* On error argc == 0 */
    argv[argc] = NULL;

    return argc;
}

static int shell_call_handler(struct shell_command *cmds, int argc, char **argv)
{
    struct shell_command *cmd = NULL;
    int error = 1;

    if (argc > 0) {
        if ((cmd = shell_find_cmd(cmds, argv[0]))) {
            int ret = cmd->handler(argc, argv);
            if (ret) {
                printf("%s: Argument error\n", cmd->name);
                shell_print_cmd_help(cmd->name, cmd->desc);
            }
            return ret;
        }
        printf("%s: Unknown command\n", argv[0]);
    }

    shell_print_help(cmds);

    // No error if command was "help" or no command provided
    error &= (argc == 0);
    error &= strcmp("help", argv[0]);

    return (error ? -1 : 0);
}


static struct shell_command *shell_find_cmd(struct shell_command *cmds, char *command)
{
    struct shell_command *cmd = NULL;
    for (cmd = cmds; cmd->name; cmd++)
        if (!strcmp(cmd->name, command))
            return cmd;
    return NULL;
}

static void shell_print_cmd_help(const char *name, const char *desc)
{
    // " %-20s %s", name, desc

    /* there is no 'print aligned left' in printf so do it manually */
    int len = printf("%s", name);
    for (; len < 20; len++)
        printf(" ");

    printf(" %s\n", desc);
}


static void shell_print_help(struct shell_command *cmds)
{
    struct shell_command *cmd = NULL;

    shell_print_cmd_help("Command", "Description");
    printf("--------------------------------\n");
    shell_print_cmd_help("help", "Print this help");
    for (cmd = cmds; cmd->name; cmd++)
        shell_print_cmd_help(cmd->name, cmd->desc);
    printf("--------------------------------\n");
}

static void char_handler_irq(handler_arg_t arg, uint8_t c)
{
    static char command_buffer[COMMAND_LEN];
    static size_t index = 0;

    if (('\n' != c) && ('\r' != c))
        command_buffer[index++] = c;

    // line full or new line
    if (('\n' == c) || (COMMAND_LEN == index)) {
        command_buffer[index] = '\0';
        index = 0;
        event_post_from_isr(EVENT_QUEUE_APPLI, parse_command, command_buffer);
    }
}

static void parse_command(void *command_buffer)
{
    int ret = shell_handle_line(state.commands, command_buffer);
    if (state.with_prompt) {
        if (ret)  // print error number
            printf("%d ", ret);
        printf("> ");
    }
}
