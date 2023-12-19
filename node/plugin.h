#ifndef PLUGIN_H
#define PLUGIN_H

typedef char Byte;

typedef union
{
    struct
    {
        unsigned int core_id : 10;      // 核心号
        unsigned int subtask_index : 10; // 核心内子任务号,由任务调度分配
        unsigned int flip : 10;          // 是否翻转，0->否，1->是
        unsigned int reserve : 2;       // 保留位
    } id1;
    unsigned int id2;
} MARKID;

/**
 * @brief Structure representing counter information.
 *
 * This structure holds information about the number of rows, buckets, bucket size, and counter size.
 */
struct CounterInfo
{
    int rownum;      /**< Number of rows */
    int bucketnum;   /**< Number of buckets */
    int bucketsize;  /**< Size of each bucket */
    int countersize; /**< Size of each counter */
};

// /**
//  * @brief Structure representing hash information.
//  *
//  * This structure contains the number of hashes, the original range, and the hash range.
//  */
// struct HashInfo {
//     int hashnum;
//     unsigned int key_len;
//     unsigned int entries;
// };

/**
 * @brief Structure representing information about a plugin.
 */
struct PluginInfo
{
    // unsigned int task_id; // 任务号，由任务注册分配
    // MARKID id;                 /**< Plugin identifier */
    CounterInfo cnt_info; /**< Counter information */
    // HashInfo hash_info;  /**< Hash information */
    char funcname[30];  /**< Plugin function */
    char filename[100]; /**< Plugin filename */
    bool flipable;      /**< Whether the plugin is flipable */
};

/**
 * @brief Function pointer type definition.
 *
 * This typedef defines a function pointer type named PF. The function takes a pointer to a rte_vlan_hdr structure, a pointer to a rte_mbuf structure, a pointer to a rte_hash structure pointer, and a pointer to a Byte**** variable. It returns an integer value.
 *
 * @param mbuf A pointer to a rte_mbuf structure.
 * @param res A pointer to a Byte**** variable.
 * @return An integer value.
 */
typedef int (*PF)(struct rte_mbuf *, Byte ****res);

/**
 * @brief Structure representing the runtime information of a plugin.
 *
 * This structure contains the following fields:
 * - id: The identifier of the plugin.
 * - res: A pointer to the resources used by the plugin.
 * - hash_table: A pointer to the hash tables used by the plugin.
 * - func: A function pointer to the plugin's main function.
 */
struct PluginRuntime
{
    MARKID id; // 32位插件运行时，由plugin生成
    Byte ****res;
    // rte_hash **hash_table;
    PF func;
};

#endif