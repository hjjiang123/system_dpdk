#include <stdlib.h>
#include "capture.h"
struct Queueid {
    int data;
    struct Queueid* next;
};

extern struct Queueid* core_queues[MAX_CORE_NUMS];
void addQueue(int queue, int core);
void removeQueue(int queue, int core);