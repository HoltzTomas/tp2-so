#include "scheduler.h"
#include "lib.h"

typedef struct SchedNode {
    pid_t pid;
    uint8_t priority;
    uint8_t quantum;
    uint8_t quantumLeft;
    uint8_t blocked;
    struct SchedNode *next;
} SchedNode;

static SchedNode nodes[MAX_PROCESSES];
static SchedNode *readyQueue = NULL;
static SchedNode *currentNode = NULL;
static int nodeCount = 0;

void scheduler_init(void) {
    readyQueue = NULL;
    currentNode = NULL;
    nodeCount = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        nodes[i].pid = -1;
    }
}

static int find_node_slot(void) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (nodes[i].pid == -1)
            return i;
    }
    return -1;
}

static SchedNode *find_node(pid_t pid) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (nodes[i].pid == pid)
            return &nodes[i];
    }
    return NULL;
}

void scheduler_add(pid_t pid, uint8_t priority) {
    int slot = find_node_slot();
    if (slot == -1)
        return;

    nodes[slot].pid = pid;
    nodes[slot].priority = priority;
    nodes[slot].quantum = priority + 1;
    nodes[slot].quantumLeft = priority + 1;
    nodes[slot].blocked = 0;
    nodes[slot].next = NULL;

    if (readyQueue == NULL) {
        readyQueue = &nodes[slot];
        readyQueue->next = readyQueue;
    } else {
        SchedNode *last = readyQueue;
        while (last->next != readyQueue)
            last = last->next;
        last->next = &nodes[slot];
        nodes[slot].next = readyQueue;
    }
    nodeCount++;
}

void scheduler_remove(pid_t pid) {
    if (readyQueue == NULL)
        return;

    SchedNode *node = find_node(pid);
    if (node == NULL)
        return;

    if (node->next == node) {
        readyQueue = NULL;
        if (currentNode == node)
            currentNode = NULL;
    } else {
        SchedNode *prev = readyQueue;
        while (prev->next != node)
            prev = prev->next;
        prev->next = node->next;

        if (readyQueue == node)
            readyQueue = node->next;
        if (currentNode == node)
            currentNode = prev;
    }

    node->pid = -1;
    nodeCount--;
}

void scheduler_block(pid_t pid) {
    SchedNode *node = find_node(pid);
    if (node != NULL)
        node->blocked = 1;
}

void scheduler_unblock(pid_t pid) {
    SchedNode *node = find_node(pid);
    if (node != NULL) {
        node->blocked = 0;
        node->quantumLeft = node->quantum;
    }
}

void scheduler_set_priority(pid_t pid, uint8_t priority) {
    SchedNode *node = find_node(pid);
    if (node != NULL) {
        node->priority = priority;
        node->quantum = priority + 1;
        node->quantumLeft = node->quantum;
    }
}

pid_t scheduler_next(void) {
    if (readyQueue == NULL)
        return -1;

    if (currentNode != NULL && !currentNode->blocked && currentNode->pid != -1) {
        currentNode->quantumLeft--;
        if (currentNode->quantumLeft > 0)
            return currentNode->pid;
    }

    SchedNode *start;
    if (currentNode == NULL || currentNode->next == NULL)
        start = readyQueue;
    else
        start = currentNode->next;

    SchedNode *iter = start;
    int checked = 0;
    do {
        if (!iter->blocked && iter->pid != -1) {
            currentNode = iter;
            iter->quantumLeft = iter->quantum;
            return iter->pid;
        }
        iter = iter->next;
        checked++;
    } while (iter != start && checked < nodeCount + 1);

    return -1;
}

void scheduler_yield(void) {
    if (currentNode != NULL)
        currentNode->quantumLeft = 0;
}

pid_t scheduler_current(void) {
    if (currentNode == NULL)
        return -1;
    return currentNode->pid;
}

uint8_t scheduler_get_priority(pid_t pid) {
    SchedNode *node = find_node(pid);
    if (node == NULL)
        return 0;
    return node->priority;
}
