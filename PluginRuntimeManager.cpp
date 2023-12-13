#include "PluginRuntimeManager.h"

PluginRuntimeList _handlers_[MAX_CORE_NUMS];

PluginRuntimeDumps *nodequeue[MAX_DUMP_NUMS]; // Plugin runtime nodes for dumping results
int nodequeue_front = 0;  // Front index of the queue
int nodequeue_rear = 0;   // Rear index of the queue
int nodequeue_count = 0;  // Number of elements in the queue

pthread_mutex_t nodequeue_mutex = PTHREAD_MUTEX_INITIALIZER;    // Mutex lock
pthread_cond_t nodequeue_cond_empty = PTHREAD_COND_INITIALIZER; // Condition variable for empty queue

// Enqueue a plugin runtime node
bool enqueuePluginRuntimeNode(PluginRuntimeDumps *nodedump) {
    pthread_mutex_lock(&nodequeue_mutex);
    if (nodequeue_count == MAX_DUMP_NUMS) {
        return false;
    }
    nodequeue[nodequeue_rear] = nodedump;
    nodequeue_rear = (nodequeue_rear + 1) % MAX_DUMP_NUMS;
    nodequeue_count++;

    pthread_cond_signal(&nodequeue_cond_empty);
    pthread_mutex_unlock(&nodequeue_mutex);
    return true;
}

// Dequeue a plugin runtime node
void dequeuePluginRuntimeNode() {
    pthread_mutex_lock(&nodequeue_mutex);
    while (nodequeue_count == 0) {
        pthread_cond_wait(&nodequeue_cond_empty, &nodequeue_mutex);
    }
    PluginRuntimeDumps * item = nodequeue[nodequeue_front];
    nodequeue_front = (nodequeue_front + 1) % MAX_DUMP_NUMS;
    nodequeue_count--;
    pthread_mutex_unlock(&nodequeue_mutex);
    dumpPluginRuntimeNode(item->node->data.id, item->filename, item->node);
}


void initPluginRuntimeList()
{
    for (int i = 0; i < MAX_CORE_NUMS; i++)
    {
        _handlers_[i].head = NULL;
        _handlers_[i].tail = NULL;
    }
}

void addPluginRuntime(int pluginid, int coreid)
{
    // printf("addPluginRuntime\n");
    // Get plugin information
    PluginInfo *pi = _PM.getPluginInfo_fromid(pluginid);
    // Allocate resources for the plugin
    Byte ****res = (Byte ****)malloc(pi->cnt_info.rownum * sizeof(Byte ***));
    for (int i = 0; i < pi->cnt_info.rownum; i++)
    {
        res[i] = (Byte ***)malloc(pi->cnt_info.bucketnum * sizeof(Byte **));
        for (int j = 0; j < pi->cnt_info.bucketnum; j++)
        {
            res[i][j] = (Byte **)malloc(pi->cnt_info.bucketsize * sizeof(Byte *));
            for (int k = 0; k < pi->cnt_info.bucketsize; k++)
            {
                res[i][j][k] = (Byte *)malloc(pi->cnt_info.countersize * sizeof(Byte));
            }
        }
    }
    // printf("addPluginRuntime: %d\n", 0);
    // Create hash tables
    // struct rte_hash *hash_table[pi->hash_info.hashnum];
    // for (unsigned int i = 0; i < pi->hash_info.hashnum; i++)
    // {
    //     char name[20];
    //     sprintf(name, "%d_%d", pi->id, i);
    //     struct rte_hash_parameters hash_params = {
    //         .name = name,
    //         .entries = pi->hash_info.entries,
    //         .key_len = pi->hash_info.key_len,
    //         .hash_func = rte_hash_crc,
    //         .hash_func_init_val = i,
    //     };
    //     hash_table[i] = rte_hash_create(&hash_params);
    //     if (hash_table[i] == NULL)
    //     {
    //         printf("Failed to create hash table %d\n", i);
    //         return;
    //     }
    // }
    // printf("addPluginRuntime: %d\n", 1);
    // Get the function pointer for the plugin
    PF myFunctionPtr = (PF)_PM.getFunction<PF>(pi->filename, pi->funcname);
    // Create a plugin runtime object
    PluginRuntime *newPlugin = (PluginRuntime *)malloc(sizeof(PluginRuntime));
    newPlugin->id = pluginid;
    newPlugin->res = res;
    // newPlugin->hash_table = hash_table;
    newPlugin->func = myFunctionPtr;
    PluginRuntimeNode *newNode = (PluginRuntimeNode *)malloc(sizeof(PluginRuntimeNode));
    newNode->data = *newPlugin;
    newNode->next = NULL;
    // printf("addPluginRuntime: %d\n", 2);
    if (_handlers_[coreid].head == NULL)
    {
        _handlers_[coreid].head = newNode;
        _handlers_[coreid].tail = newNode;
    }
    else
    {
        _handlers_[coreid].tail->next = newNode;
        _handlers_[coreid].tail = newNode;
    }
    // printf("addPluginRuntime: %d\n", _handlers_[coreid].tail->data.id);
}

bool popPluginRuntime(int pluginid, int coreid, PluginRuntimeNode *node)
{
    node = _handlers_[coreid].head;
    PluginRuntimeNode *previous = NULL;
    PluginInfo *pi = _PM.getPluginInfo_fromid(pluginid);
    while (node != NULL)
    {
        if (node->data.id == pluginid)
        {
            if (previous == NULL)
            {
                _handlers_[coreid].head = node->next;
            }
            else
            {
                previous->next = node->next;
            }

            if (node == _handlers_[coreid].tail)
            {
                _handlers_[coreid].tail = previous;
            }
            return true;
        }
        previous = node;
        node = node->next;
    }
    return false;
}

void deletePluginRuntime(int pluginid, int coreid)
{
    PluginInfo *pi = _PM.getPluginInfo_fromid(pluginid);
    PluginRuntimeNode *node = NULL;
    if(popPluginRuntime(pluginid, coreid, node)){
        // Release memory
        for (int i = 0; i < pi->cnt_info.rownum; i++)
        {
            for (int j = 0; j < pi->cnt_info.bucketnum; j++)
            {
                for (int k = 0; k < pi->cnt_info.bucketsize; k++)
                {
                    free((void *)(node->data.res)[i][j][k]);
                    node->data.res[i][j][k] = NULL;
                }
                free((void *)(node->data.res)[i][j]);
                node->data.res[i][j] = NULL;
            }
            free((void *)(node->data.res[i]));
            node->data.res[i] = NULL;
        }
        free((void *)node->data.res);
        node->data.res = NULL;

        // Release hash tables
        // for (int i = 0; i < pi->hash_info.hashnum; i++)
        // {
        //     rte_hash_free(node->data.hash_table[i]);
        //     node->data.hash_table[i] = NULL;
        // }
        // node->data.hash_table = NULL;
        // Remove from the array of plugins to be deployed
        free(node);
        node = NULL;
    }
}

void dumpPluginRuntimeNode(int pluginid, char *filename,PluginRuntimeNode *node){
    PluginInfo *pi = _PM.getPluginInfo_fromid(pluginid);
    printf("dumpResult\n");
    char p[128];
    sprintf(p,"./res/%s",filename);
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        printf("popPluginRuntime success\n");
        for (int i = 0; i < pi->cnt_info.rownum; i++)
        {
            for (int j = 0; j < pi->cnt_info.bucketnum; j++)
            {
                for (int k = 0; k < pi->cnt_info.bucketsize; k++)
                {
                    fwrite(node->data.res[i][j][k], pi->cnt_info.countersize, 1, file);
                    free((void *)(node->data.res)[i][j][k]);
                    node->data.res[i][j][k] = NULL;
                }
                free((void *)(node->data.res)[i][j]);
                node->data.res[i][j] = NULL;
            }
            free((void *)(node->data.res[i]));
            node->data.res[i] = NULL;
        }
    }
    fclose(file);
}
