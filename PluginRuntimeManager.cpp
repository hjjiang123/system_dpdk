#include "PluginRuntimeManager.h"

PluginRuntimeList _handlers_[MAX_CORE_NUMS];

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
    struct rte_hash *hash_table[pi->hash_info.hashnum];
    for (unsigned int i = 0; i < pi->hash_info.hashnum; i++)
    {
        char name[20];
        sprintf(name, "%d_%d", pi->id, i);
        struct rte_hash_parameters hash_params = {
            .name = name,
            .entries = pi->hash_info.entries,
            .key_len = pi->hash_info.key_len,
            .hash_func = rte_hash_crc,
            .hash_func_init_val = i,
        };
        hash_table[i] = rte_hash_create(&hash_params);
        if (hash_table[i] == NULL)
        {
            printf("Failed to create hash table %d\n", i);
            return;
        }
    }
    // printf("addPluginRuntime: %d\n", 1);
    // Get the function pointer for the plugin
    PF myFunctionPtr = (PF)_PM.getFunction<PF>(pi->filename, pi->funcname);
    // Create a plugin runtime object
    PluginRuntime *newPlugin =(PluginRuntime *)malloc(sizeof(PluginRuntime));
    newPlugin->id = pluginid;
    newPlugin->res = res;
    newPlugin->hash_table = hash_table;
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

void deletePluginRuntime(int pluginid, int coreid)
{
    PluginRuntimeNode *current = _handlers_[coreid].head;
    PluginRuntimeNode *previous = NULL;
    PluginInfo *pi = _PM.getPluginInfo_fromid(pluginid);
    while (current != NULL)
    {
        if (current->data.id == pluginid)
        {
            if (previous == NULL)
            {
                _handlers_[coreid].head = current->next;
            }
            else
            {
                previous->next = current->next;
            }

            if (current == _handlers_[coreid].tail)
            {
                _handlers_[coreid].tail = previous;
            }
            // Release memory
            for (int i = 0; i < pi->cnt_info.rownum; i++)
            {
                for (int j = 0; j < pi->cnt_info.bucketnum; j++)
                {
                    for (int k = 0; k < pi->cnt_info.bucketsize; k++)
                    {
                        free((void *)(current->data.res)[i][j][k]);
                        current->data.res[i][j][k]= NULL;
                    }
                    free((void *)(current->data.res)[i][j]);
                    current->data.res[i][j]= NULL;
                }
                free((void *)(current->data.res[i]));
                current->data.res[i] = NULL;
            }
            free((void *)current->data.res);
            current->data.res = NULL;
    
            // Release hash tables
            for (int i = 0; i < pi->hash_info.hashnum; i++)
            {
                rte_hash_free(current->data.hash_table[i]);
                current->data.hash_table[i] = NULL;
            }
            current->data.hash_table = NULL;
            // Remove from the array of plugins to be deployed
            free(current);
            current = NULL;
            // printf("deletePluginRuntime\n");
            break;
        }
        previous = current;
        current = current->next;
    }
    
}
