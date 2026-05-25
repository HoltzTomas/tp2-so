#include "scheduler.h"
#include "lib.h"

typedef struct {
    pid_t pid;
    uint8_t priority;
    uint16_t quantumsLeft;
    uint8_t active;
} SchedEntry;

static SchedEntry queue[MAX_PROCESSES];
static int queueSize = 0;
static int currentIndex = -1;

void scheduler_init(void) {
    memset(queue, 0, sizeof(queue));
    queueSize = 0;
    currentIndex = -1;
}

void scheduler_add(pid_t pid) {
    /* Check if already in queue */
    for (int i = 0; i < queueSize; i++) {
        if (queue[i].pid == pid && queue[i].active) {
            return;
        }
    }

    /* Find empty slot or add at end */
    int slot = -1;
    for (int i = 0; i < queueSize; i++) {
        if (!queue[i].active) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        if (queueSize >= MAX_PROCESSES)
            return;
        slot = queueSize++;
    }

    queue[slot].pid = pid;
    queue[slot].priority = 1;
    queue[slot].quantumsLeft = 0;
    queue[slot].active = 1;
}

void scheduler_remove(pid_t pid) {
    for (int i = 0; i < queueSize; i++) {
        if (queue[i].pid == pid && queue[i].active) {
            queue[i].active = 0;
            if (currentIndex == i)
                currentIndex = -1;
            return;
        }
    }
}

pid_t scheduler_next(void) {
    if (queueSize == 0)
        return 0;

    /* Check if current process still has quantums left */
    if (currentIndex >= 0 && currentIndex < queueSize &&
        queue[currentIndex].active && queue[currentIndex].quantumsLeft > 0) {
        queue[currentIndex].quantumsLeft--;
        return queue[currentIndex].pid;
    }

    /* Find next active process (round-robin) */
    int start = (currentIndex + 1) % queueSize;
    int i = start;
    int idleIdx = -1;

    do {
        if (queue[i].active) {
            if (queue[i].pid == 0) {
                idleIdx = i;
            } else {
                currentIndex = i;
                queue[i].quantumsLeft = queue[i].priority; /* priority+1 total quantums */
                return queue[i].pid;
            }
        }
        i = (i + 1) % queueSize;
    } while (i != start);

    /* Only idle is available */
    if (idleIdx >= 0) {
        currentIndex = idleIdx;
        return 0;
    }

    return 0; /* fallback to idle */
}

void scheduler_tick(void) {
    /* Called each timer tick - currently handled by quantumsLeft in scheduler_next */
}

void scheduler_set_priority(pid_t pid, uint8_t priority) {
    for (int i = 0; i < queueSize; i++) {
        if (queue[i].pid == pid && queue[i].active) {
            queue[i].priority = priority;
            return;
        }
    }
}

uint8_t scheduler_get_priority(pid_t pid) {
    for (int i = 0; i < queueSize; i++) {
        if (queue[i].pid == pid && queue[i].active) {
            return queue[i].priority;
        }
    }
    return DEFAULT_PRIORITY;
}
