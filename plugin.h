#ifndef PLUGIN_H
#define PLUGIN_H
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <vector>
#include <string>
#include <memory>
#include "capture.h"

typedef char Byte;

/**
 * @brief Structure representing counter information.
 * 
 * This structure holds information about the number of rows, buckets, bucket size, and counter size.
 */
struct CounterInfo {
    int rownum;        /**< Number of rows */
    int bucketnum;      /**< Number of buckets */
    int bucketsize;     /**< Size of each bucket */
    int countersize;    /**< Size of each counter */
};


/**
 * @brief Structure representing hash information.
 * 
 * This structure contains the number of hashes, the original range, and the hash range.
 */
struct HashInfo {
    int hashnum;
    int key_len;
    int entries;
};


/**
 * @brief Structure representing information about a plugin.
 */
struct PluginInfo{
    int id;  /**< Plugin identifier */
    CounterInfo cnt_info;  /**< Counter information */
    HashInfo hash_info;  /**< Hash information */
    char funcname[30];  /**< Plugin function */
    char filename[100];  /**< Plugin filename */
};


/**
 * @brief Function pointer type definition.
 *
 * This typedef defines a function pointer type named PF. The function takes a pointer to a rte_vlan_hdr structure, a pointer to a rte_mbuf structure, a pointer to a rte_hash structure pointer, and a pointer to a Byte**** variable. It returns an integer value.
 *
 * @param vlan_hdr A pointer to a rte_vlan_hdr structure.
 * @param mbuf A pointer to a rte_mbuf structure.
 * @param hash_table A pointer to a rte_hash structure pointer.
 * @param res A pointer to a Byte**** variable.
 * @return An integer value.
 */
typedef int (*PF)(struct rte_vlan_hdr *, struct rte_mbuf *, struct rte_hash **hash_table, Byte ****res);


/**
 * @brief Structure representing the runtime information of a plugin.
 * 
 * This structure contains the following fields:
 * - id: The identifier of the plugin.
 * - res: A pointer to the resources used by the plugin.
 * - hash_table: A pointer to the hash tables used by the plugin.
 * - func: A function pointer to the plugin's main function.
 */
struct PluginRuntime{
    int id; 
    Byte ****res; 
    rte_hash **hash_table; 
    PF func; 
};


#endif