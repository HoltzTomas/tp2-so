#include "process.h"
#include "scheduler.h"
#include "memoryManager.h"
#include "lib.h"
#include "interrupts.h"

#define PROCESS_STACK_SIZE 4096

typedef struct WaitNode {
    pid_t waitingPid;
    struct WaitNode *next;
} WaitNode;

typedef struct Process {
    pid_t pid;
    pid_t parentPid;
    char name[PROCESS_NAME_LEN];
    uint8_t priority;
    ProcessState state;
    uint8_t foreground;
    uint64_t rsp;
    uint64_t rbp;
    void *stackBase;
    fd_t fds[2];
    int retValue;
    WaitNode *waiters;
} Process;

static Process processes[MAX_PROCESSES];
static pid_t currentPid = -1;
static pid_t nextPid = 1;

typedef struct {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rsi, rdi, rbp, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
} StackFrame;

static void process_wrapper(ProcessFunc func, uint64_t argc, char *argv[]) {
    uint64_t ret = func(argc, argv);
    process_exit((int)ret);
}

void process_init(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].state = ZOMBIE;
        processes[i].pid = -1;
    }
}

static int find_slot(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == -1)
            return i;
    }
    return -1;
}

static int find_process(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid == pid && processes[i].state != ZOMBIE)
            return i;
    }
    return -1;
}

pid_t process_create(ProcessFunc func, uint64_t argc, char *argv[], const char *name, uint8_t foreground, fd_t fds[2]) {
    int slot = find_slot();
    if (slot == -1)
        return -1;

    Process *p = &processes[slot];

    void *stack = mm_malloc(PROCESS_STACK_SIZE);
    if (stack == NULL)
        return -1;

    p->pid = nextPid++;
    p->parentPid = currentPid;
    strncpy(p->name, name, PROCESS_NAME_LEN - 1);
    p->name[PROCESS_NAME_LEN - 1] = '\0';
    p->priority = DEFAULT_PRIORITY;
    p->state = READY;
    p->foreground = foreground;
    p->stackBase = stack;
    p->retValue = 0;
    p->waiters = NULL;

    if (fds != NULL) {
        p->fds[0] = fds[0];
        p->fds[1] = fds[1];
    } else {
        p->fds[0] = STDIN;
        p->fds[1] = STDOUT;
    }

    uint64_t stackTop = (uint64_t)stack + PROCESS_STACK_SIZE;

    char **argvCopy = NULL;
    if (argc > 0 && argv != NULL) {
        argvCopy = (char **)mm_malloc((argc + 1) * sizeof(char *));
        if (argvCopy != NULL) {
            for (uint64_t i = 0; i < argc; i++) {
                if (argv[i] != NULL) {
                    uint64_t len = strlen(argv[i]) + 1;
                    argvCopy[i] = (char *)mm_malloc(len);
                    if (argvCopy[i] != NULL)
                        memcpy(argvCopy[i], argv[i], len);
                } else {
                    argvCopy[i] = NULL;
                }
            }
            argvCopy[argc] = NULL;
        }
    }

    StackFrame *frame = (StackFrame *)(stackTop - sizeof(StackFrame));
    memset(frame, 0, sizeof(StackFrame));
    frame->rip = (uint64_t)process_wrapper;
    frame->cs = 0x08;
    frame->rflags = 0x202;
    frame->rsp = stackTop;
    frame->ss = 0x00;
    frame->rdi = (uint64_t)func;
    frame->rsi = argc;
    frame->rdx = (uint64_t)argvCopy;

    p->rsp = (uint64_t)frame;
    p->rbp = stackTop;

    scheduler_add(p->pid, p->priority);
    return p->pid;
}

int process_kill(pid_t pid) {
    if (pid <= 0)
        return -1;

    int idx = find_process(pid);
    if (idx == -1)
        return -1;

    Process *p = &processes[idx];
    p->state = ZOMBIE;
    scheduler_remove(pid);

    WaitNode *waiter = p->waiters;
    while (waiter != NULL) {
        process_unblock(waiter->waitingPid);
        WaitNode *next = waiter->next;
        mm_free(waiter);
        waiter = next;
    }
    p->waiters = NULL;

    if (pid == currentPid) {
        currentPid = -1;
        force_timer();
    }

    return 0;
}

int process_block(pid_t pid) {
    int idx = find_process(pid);
    if (idx == -1)
        return -1;

    processes[idx].state = BLOCKED;
    scheduler_block(pid);

    if (pid == currentPid)
        force_timer();

    return 0;
}

int process_unblock(pid_t pid) {
    int idx = find_process(pid);
    if (idx == -1)
        return -1;

    if (processes[idx].state == BLOCKED) {
        processes[idx].state = READY;
        scheduler_unblock(pid);
    }
    return 0;
}

int process_set_priority(pid_t pid, uint8_t priority) {
    if (priority > MAX_PRIORITY)
        priority = MAX_PRIORITY;

    int idx = find_process(pid);
    if (idx == -1)
        return -1;

    processes[idx].priority = priority;
    scheduler_set_priority(pid, priority);
    return 0;
}

pid_t process_getpid(void) {
    return currentPid;
}

int process_list(ProcessInfo *info_array, int max_count) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES && count < max_count; i++) {
        if (processes[i].pid != -1 && processes[i].state != ZOMBIE) {
            info_array[count].pid = processes[i].pid;
            strncpy(info_array[count].name, processes[i].name, PROCESS_NAME_LEN);
            info_array[count].priority = processes[i].priority;
            info_array[count].rsp = processes[i].rsp;
            info_array[count].rbp = processes[i].rbp;
            info_array[count].foreground = processes[i].foreground;
            info_array[count].state = processes[i].state;
            info_array[count].parentPid = processes[i].parentPid;
            count++;
        }
    }
    return count;
}

int process_wait(pid_t pid) {
    int idx = find_process(pid);
    if (idx == -1)
        return -1;

    WaitNode *node = (WaitNode *)mm_malloc(sizeof(WaitNode));
    if (node == NULL)
        return -1;

    node->waitingPid = currentPid;
    node->next = processes[idx].waiters;
    processes[idx].waiters = node;

    process_block(currentPid);
    return processes[idx].retValue;
}

void process_yield(void) {
    force_timer();
}

void process_exit(int retValue) {
    if (currentPid <= 0)
        return;

    int idx = find_process(currentPid);
    if (idx == -1)
        return;

    processes[idx].retValue = retValue;
    process_kill(currentPid);

    while (1)
        _hlt();
}

ProcessState process_get_state(pid_t pid) {
    int idx = find_process(pid);
    if (idx == -1)
        return ZOMBIE;
    return processes[idx].state;
}

fd_t process_get_fd(pid_t pid, int index) {
    int idx = find_process(pid);
    if (idx == -1 || index < 0 || index > 1)
        return -1;
    return processes[idx].fds[index];
}

void process_set_fd(pid_t pid, int index, fd_t fd) {
    int idx = find_process(pid);
    if (idx == -1 || index < 0 || index > 1)
        return;
    processes[idx].fds[index] = fd;
}

pid_t process_get_foreground(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (processes[i].pid != -1 && processes[i].state != ZOMBIE && processes[i].foreground)
            return processes[i].pid;
    }
    return -1;
}

uint64_t *schedule(uint64_t *rsp) {
    if (currentPid > 0) {
        int idx = find_process(currentPid);
        if (idx != -1)
            processes[idx].rsp = (uint64_t)rsp;
    }

    pid_t next = scheduler_next();
    if (next == -1) {
        return rsp;
    }

    currentPid = next;
    int idx = find_process(currentPid);
    if (idx == -1)
        return rsp;

    processes[idx].state = RUNNING;
    return (uint64_t *)processes[idx].rsp;
}
