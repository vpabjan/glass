#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct proc {
    pid_t pid;
    char *cmd;
    int restart;
    struct proc *next;
} proc_t;

typedef struct {
    proc_t *head;
} proc_list_t;

static pid_t spawn(const char *cmd) {
    pid_t pid = fork();
    if (pid == 0) {
        execl("/bin/sh", "sh", "-c", cmd, NULL);
        _exit(1);
    }
    return pid;
}

void proc_list_init(proc_list_t *l) {
    l->head = NULL;
}

pid_t proc_launch(proc_list_t *l, const char *cmd, int use_home, int restart) {
    char *final = NULL;

    if (use_home) {
        const char *home = getenv("HOME");
        if (!home) return -1;

        size_t len = strlen(home) + strlen(cmd) + 2;
        final = malloc(len);
        snprintf(final, len, "%s/%s", home, cmd);
    } else {
        final = strdup(cmd);
    }

    pid_t pid = spawn(final);
    if (pid < 0) {
        free(final);
        return -1;
    }

    proc_t *p = calloc(1, sizeof(proc_t));
    p->pid = pid;
    p->cmd = final;
    p->restart = restart;
    p->next = l->head;
    l->head = p;

    return pid;
}

void proc_check(proc_list_t *l) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (proc_t *p = l->head; p; p = p->next) {
            if (p->pid == pid && p->restart) {
                p->pid = spawn(p->cmd);
                break;
            }
        }
    }
}

void proc_list_free(proc_list_t *l) {
    proc_t *p = l->head;
    while (p) {
        proc_t *next = p->next;
        free(p->cmd);
        free(p);
        p = next;
    }
    l->head = NULL;
}
