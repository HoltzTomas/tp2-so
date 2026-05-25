#include "process.h"
#include "scheduler.h"
#include "memoryManager.h"
#include "interrupts.h"
#include "lib.h"

extern void increment_ticks(void);

static PCB processes[MAX_PROCESSES];
static pid_t currentPid = -1;
static pid_t nextPid = 0;

static void idle_func(uint64_t argc, char *argv[]) {
    (void)argc;
    (void)argv;
    while (1) {
        _hlt();
    }
}



void process_init(void) {
    memset(processes, 0, sizeof(processes));
    for (int i = 0; i < MAX_PROCESSES; i++) {
        processes[i].pid = -1;
        processes[i].state = ZOMBIE;
    }

    /* Create idle process (PID 0) */
    processes[0].pid = 0;
    strncpy(processes[0].name, "idle", PROCESS_NAME_LEN);
    processes[0].state = READY;
    processes[0].priority = MAX_PRIORITY;
    processes[0].foreground = 0;
    processes[0].parentPid = -1;
    processes[0].fds[0] = 0;
    processes[0].fds[1] = 1;

    void *stack = mm_malloc(PROCESS_STACK_SIZE);
    processes[0].stackBase = stack;
    void *stackTop = (void *)((uint64_t)stack + PROCESS_STACK_SIZE);
    processes[0].rsp = _initialize_stack_frame((void (*)(uint64_t, char **))idle_func, NULL, stackTop);

    nextPid = 1;
    currentPid = 0;
    processes[0].state = RUNNING;

    scheduler_init();
    scheduler_add(0);
}

pid_t process_create(ProcessFunc func, uint64_t argc, char *argv[],
                     const char *name, uint8_t foreground, int16_t fds[2]) {
    if (nextPid >= MAX_PROCESSES) {
        /* Look for a free slot */
        int found = -1;
        for (int i = 1; i < MAX_PROCESSES; i++) {
            if (processes[i].pid == -1 || processes[i].state == ZOMBIE) {
                found = i;
                break;
            }
        }
        if (found == -1)
            return -1;
        nextPid = found;
    }

    pid_t pid = nextPid++;
    PCB *p = &processes[pid];

    p->pid = pid;
    strncpy(p->name, name, PROCESS_NAME_LEN);
    p->state = READY;
    p->priority = DEFAULT_PRIORITY;
    p->foreground = foreground;
    p->parentPid = currentPid;
    p->retValue = 0;
    p->waitingFor = -1;
    p->quantumsLeft = 0;

    if (fds != NULL) {
        p->fds[0] = fds[0];
        p->fds[1] = fds[1];
    } else {
        p->fds[0] = 0;
        p->fds[1] = 1;
    }

    void *stack = mm_malloc(PROCESS_STACK_SIZE);
    if (stack == NULL)
        return -1;

    p->stackBase = stack;
    void *stackTop = (void *)((uint64_t)stack + PROCESS_STACK_SIZE);

    p->rsp = _initialize_stack_frame((void (*)(uint64_t, char **))func, (void *)argc, stackTop);

    scheduler_add(pid);
    return pid;
}

int process_kill(pid_t pid) {
    if (pid <= 0 || pid >= MAX_PROCESSES)
        return -1;
    if (processes[pid].pid == -1)
        return -1;

    PCB *p = &processes[pid];
    p->state = ZOMBIE;
    p->retValue = -1;

    scheduler_remove(pid);

    /* Wake up parent if waiting */
    if (p->parentPid >= 0 && p->parentPid < MAX_PROCESSES) {
        PCB *parent = &processes[p->parentPid];
        if (parent->state == BLOCKED && parent->waitingFor == pid) {
            parent->state = READY;
            parent->waitingFor = -1;
            scheduler_add(p->parentPid);
        }
    }

    /* Free stack */
    if (p->stackBase != NULL) {
        mm_free(p->stackBase);
        p->stackBase = NULL;
    }

    p->pid = -1;

    if (pid == currentPid) {
        currentPid = -1;
        force_timer();
    }

    return 0;
}

int process_block(pid_t pid) {
    if (pid < 0 || pid >= MAX_PROCESSES)
        return -1;
    if (processes[pid].pid == -1)
        return -1;
    if (processes[pid].state == ZOMBIE)
        return -1;

    PCB *p = &processes[pid];

    if (p->state == BLOCKED) {
        /* Toggle: unblock */
        p->state = READY;
        scheduler_add(pid);
    } else {
        p->state = BLOCKED;
        scheduler_remove(pid);
        if (pid == currentPid)
            force_timer();
    }

    return 0;
}

int process_unblock(pid_t pid) {
    if (pid < 0 || pid >= MAX_PROCESSES)
        return -1;
    if (processes[pid].pid == -1)
        return -1;
    if (processes[pid].state != BLOCKED)
        return -1;

    processes[pid].state = READY;
    scheduler_add(pid);
    return 0;
}

void process_exit(int retValue) {
    if (currentPid < 0)
        return;

    PCB *p = &processes[currentPid];
    p->state = ZOMBIE;
    p->retValue = retValue;

    scheduler_remove(currentPid);

    /* Wake up parent if waiting */
    if (p->parentPid >= 0 && p->parentPid < MAX_PROCESSES) {
        PCB *parent = &processes[p->parentPid];
        if (parent->state == BLOCKED && parent->waitingFor == currentPid) {
            parent->state = READY;
            parent->waitingFor = -1;
            scheduler_add(p->parentPid);
        }
    }

    /* Free stack */
    if (p->stackBase != NULL) {
        mm_free(p->stackBase);
        p->stackBase = NULL;
    }

    p->pid = -1;
    currentPid = -1;
    force_timer();

    /* Should never reach here */
    while (1)
        _hlt();
}

int process_wait(pid_t pid) {
    if (pid < 0 || pid >= MAX_PROCESSES)
        return -1;
    if (processes[pid].pid == -1 || processes[pid].state == ZOMBIE)
        return processes[pid].retValue;

    PCB *current = &processes[currentPid];
    current->state = BLOCKED;
    current->waitingFor = pid;
    scheduler_remove(currentPid);
    force_timer();

    /* When we get here, the child has exited */
    return processes[pid].retValue;
}

pid_t process_getpid(void) {
    return currentPid;
}

int process_nice(pid_t pid, uint8_t priority) {
    if (pid < 0 || pid >= MAX_PROCESSES)
        return -1;
    if (processes[pid].pid == -1)
        return -1;
    if (priority > MAX_PRIORITY)
        priority = MAX_PRIORITY;

    processes[pid].priority = priority;
    scheduler_set_priority(pid, priority);
    return 0;
}

int process_list(ProcessInfo *buf, int max) {
    int count = 0;
    for (int i = 0; i < MAX_PROCESSES && count < max; i++) {
        if (processes[i].pid != -1) {
            buf[count].pid = processes[i].pid;
            strncpy(buf[count].name, processes[i].name, PROCESS_NAME_LEN);
            buf[count].priority = processes[i].priority;
            buf[count].state = processes[i].state;
            buf[count].foreground = processes[i].foreground;
            buf[count].parentPid = processes[i].parentPid;
            count++;
        }
    }
    return count;
}

PCB *process_get_current(void) {
    if (currentPid < 0)
        return NULL;
    return &processes[currentPid];
}

void process_yield(void) {
    force_timer();
}

int process_set_fds(pid_t pid, int16_t fds[2]) {
    if (pid < 0 || pid >= MAX_PROCESSES)
        return -1;
    if (processes[pid].pid == -1)
        return -1;
    processes[pid].fds[0] = fds[0];
    processes[pid].fds[1] = fds[1];
    return 0;
}

/* Called by context switch - save and restore RSP */
void *schedule(void *currentRsp) {
    increment_ticks();

    if (currentPid >= 0 && processes[currentPid].state == RUNNING) {
        processes[currentPid].rsp = currentRsp;
        processes[currentPid].state = READY;
    }

    pid_t next = scheduler_next();
    if (next < 0)
        next = 0; /* Fallback to idle */

    currentPid = next;
    processes[currentPid].state = RUNNING;
    return processes[currentPid].rsp;
}
