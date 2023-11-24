#include "QueueManager.h"


struct Queueid* core_queues[MAX_CORE_NUMS];

void addQueue(int queue, int core) {
    struct Queueid* newQueueid = (struct Queueid*)malloc(sizeof(struct Queueid));
    newQueueid->data = queue;
    newQueueid->next = NULL;

    if (core_queues[core] == NULL) {
        core_queues[core] = newQueueid;
    } else {
        struct Queueid* temp = core_queues[core];
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newQueueid;
    }
}

void removeQueue(int queue, int core) {
    struct Queueid* temp = core_queues[core];
    struct Queueid* prev = NULL;

    if (temp != NULL && temp->data == queue) {
        core_queues[core] = temp->next;
        free(temp);
        return;
    }

    while (temp != NULL && temp->data != queue) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        return;
    }

    prev->next = temp->next;
    free(temp);
}
