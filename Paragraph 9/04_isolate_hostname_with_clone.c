#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int child_main(void *ptr)
{
    const char *name = (const char *)ptr;

    if (sethostname(name, strlen(name)) == -1)
    {
        perror("sethostname");
        return 1;
    }

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        perror("gethostname");
        return 1;
    }

    printf("Child process: %s\n", hostname);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    const size_t stack_size = 1024 * 1024;
    char *child_stack = malloc(stack_size);
    if (child_stack == NULL)
    {
        perror("malloc");
        return 1;
    }

    pid_t pid = clone(child_main,
                      child_stack + stack_size,
                      CLONE_NEWUTS | CLONE_NEWUSER | SIGCHLD,
                      argv[1]);

    if (pid == -1)
    {
        perror("clone");
        free(child_stack);
        return 1;
    }

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        perror("gethostname");
        free(child_stack);
        return 1;
    }

    printf("Parent process: %s\n", hostname);

    int status;
    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        free(child_stack);
        return 1;
    }

    free(child_stack);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return 1;
}
