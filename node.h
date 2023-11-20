#ifndef Node_H
#define Node_H
#include "capture.h"



struct Node {
    const char name[20];
    void (*process)(struct rte_vlan_hdr *, struct rte_mbuf * );
};


#endif