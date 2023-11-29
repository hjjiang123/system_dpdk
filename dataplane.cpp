#include "dataplane.h"

/************************************Global Variables***************************************/



lcoreCommandQueue _command_queues[MAX_CORE_NUMS];         // Command queues for each core,to add or delete plugin

int _nextId = 1; // Next available plugin ID

std::mutex _mt[2][MAX_CORE_NUMS]; //_mt[0] is used for _command_queues update, _mt[1] is used for _core_queues update



/************************************Utility Functions************************************/


int registerPlugin(PluginInfo plugin)
{
    printf("registerPlugin\n");
    int id = _PM.loadPlugin(plugin);
    return id;
}


void unregisterPlugin(PluginInfo plugin)
{
    _PM.unloadPlugin(plugin.filename);
}


void unregisterPlugin(int pluginid)
{
    printf("unregister Plugin\n");
    PluginInfo *pi = _PM.getPluginInfo_fromid(pluginid);
    _PM.unloadPlugin(pi->filename);
}


void addPlugin(int pluginid, int coreid)
{
    printf("addPlugin\n");
    addPluginRuntime(pluginid, coreid);
    return;
}

void deletePlugin(int pluginid, int coreid)
{
    printf("deletePlugin\n");
    deletePluginRuntime(pluginid, coreid);
}

// Add a flow rule to the specified queue
int addFlowToQueue(uint16_t port_id, uint16_t rx_q, uint32_t src_ip, uint32_t src_mask, uint32_t dest_ip, uint32_t dest_mask)
{
    printf("addFlowToQueue\n");
    struct rte_flow_error error;
    flow_id *flow = generate_ipv4_flow(port_id, rx_q,
                                       src_ip, src_mask,
                                       dest_ip, dest_mask, &error);
    /* >8 End of create flow and the flow rule. */
    if (!flow)
    {
        printf("Flow can't be created %d message: %s\n",
               error.type,
               error.message ? error.message : "(no stated reason)");
        rte_exit(EXIT_FAILURE, "error in creating flow");
    }
    printf("flow id: %d\n", flow->id);
    return flow->id;
}

// Delete a flow rule from the specified queue
void deleteFlowFromQueue(int id)
{
    printf("deleteFlowFromQueue\n");
    destroy_ipv4_flow_with_id(id);
}
// Add a source queue for core to process traffic
void addQueueToCore(int queueid, int coreid)
{
    printf("addQueueToCore\n");
    addQueue(queueid, coreid);
}

// Remove a source queue from core to process traffic
void deleteQueueFromCore(int queueid, int coreid)
{
    printf("deleteQueueFromCore\n");
    removeQueue(queueid, coreid);
}
// Dump the result of a running plugin
void dumpResult(int pluginid, int coreid, char *filename){
    printf("dumpResult\n");
    char p[128];
    sprintf("./res/%s",filename);
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        
    }
}


void push_Command(Command c){
    
    int lcore_id;
    if(c.type==ADD_PLUGIN){
        lcore_id = c.args.add_plugin_arg.coreid;
    }else if(c.type==DELETE_PLUGIN){
        lcore_id = c.args.del_plugin_arg.coreid;
    }else if (c.type==ADD_QUEUE_TO_CORE){
        lcore_id = c.args.add_queue_arg.coreid;
    }else if (c.type==DELETE_FLOW_FROM_QUEUE){
        lcore_id = c.args.del_queue_arg.coreid;
    }else{
        printf("error CommandType\n");
        return;
    }
    printf("push_Command %d on core %d\n",c.type,lcore_id);
    _command_queues[lcore_id].mt.lock();
    if((_command_queues[lcore_id].rear+1)%SOCKET_QUEUE_SIZE==_command_queues[lcore_id].front){
        printf("command queue is full\n");
    }else {
        _command_queues[lcore_id].queue[_command_queues[lcore_id].rear] = c;
        _command_queues[lcore_id].rear = (_command_queues[lcore_id].rear+1)%SOCKET_QUEUE_SIZE;
    }
    _command_queues[lcore_id].mt.unlock();
    return;
}

/************************************System Functions***************************************/


// Daemon thread for each core
int handle_packet_per_core(void *arg)
{
    unsigned int lcore_id = rte_lcore_id();
    // std::cout << "startup lcore " << lcore_id << std::endl;
    struct rte_mbuf *mbufs[DPDKCAP_CAPTURE_BURST_SIZE];
    int record = 0;
    // int pkt_num = 0;
    // Receive and process packets
    // float um = 100; //等待的微秒数
    while (1)
    {
        struct Queueid* temp = core_queues[lcore_id];
        while (temp != NULL) {
            const uint16_t nb_rx = rte_eth_rx_burst(_port_id, temp->data, mbufs, DPDKCAP_CAPTURE_BURST_SIZE);
            for (uint16_t i = 0; i < nb_rx; ++i)
            {
                PluginRuntimeNode *current = _handlers_[lcore_id].head;
                while (current != NULL)
                {
                    current->data.func(mbufs[i], current->data.hash_table, current->data.res);
                    current = current->next;
                    // printf("current is not null\n");
                }
                rte_pktmbuf_free(mbufs[i]);
            }

            // pkt_num = pkt_num<nb_rx ? nb_rx:pkt_num;
            // printf("pkt_recv: %d",nb_rx);
            temp = temp->next;
        }
        
        // printf("record=%d\n",record);
        if (record++ == 10000000)
        {
            _command_queues[lcore_id].mt.lock();
            int k=_command_queues[lcore_id].front;
            while(k!=_command_queues[lcore_id].rear){
                switch (_command_queues[lcore_id].queue[k].type)
                {
                    case ADD_PLUGIN:
                        addPlugin(_command_queues[lcore_id].queue[k].args.add_plugin_arg.pluginid,
                                 _command_queues[lcore_id].queue[k].args.add_plugin_arg.coreid);
                        break;
                    case DELETE_PLUGIN:
                        deletePlugin(_command_queues[lcore_id].queue[k].args.del_plugin_arg.pluginid,
                         _command_queues[lcore_id].queue[k].args.del_plugin_arg.coreid);
                        break;
                    case ADD_QUEUE_TO_CORE:
                        addQueueToCore(_command_queues[lcore_id].queue[k].args.add_queue_arg.queueid,
                        _command_queues[lcore_id].queue[k].args.add_queue_arg.coreid);
                        break;
                    case DELETE_QUEUE_FROM_CORE:
                        deleteQueueFromCore(_command_queues[lcore_id].queue[k].args.del_queue_arg.queueid,
                         _command_queues[lcore_id].queue[k].args.del_queue_arg.coreid);
                        break;
                    default:
                        break;
                }
                k = (k+1) % SOCKET_QUEUE_SIZE;
            }
            _command_queues[lcore_id].front = k;
            _command_queues[lcore_id].mt.unlock();
            record = 0;
        }
        if (record++ == 100000000){
            
        }
        // um = um*5/4 - um*pkt_num/32;
        // if(um==0){
        //     printf("%d record=%d,pkt_recv: %d,um:%f\n",lcore_id,record,pkt_num,um);
        //     exit(1);
        // }
        // if(lcore_id==10){
        //     printf("%d record=%d,pkt_recv: %d,um:%f\n",lcore_id,record,pkt_num,um);
        // }
        // um = um>10000?10000:um;
        // pkt_num = 0;
        // rte_delay_us((unsigned int)um);
        rte_pause();
    }
}

// Main thread to deploy child threads to each core
void run()
{
    rte_flow_error error;
    for (int i = 0; i < _num_cores; i++)
    {
        int ret = rte_eal_remote_launch(handle_packet_per_core, NULL, i);
        if (ret != 0)
        {
            rte_panic("Cannot launch auxiliary thread\n");
        }
    }
    for (int i = 0; i < _num_cores; i++)
    {
        // Wait for logical cores to finish execution
        int ret = rte_eal_wait_lcore(i);
        if (ret < 0)
        {
            rte_panic("Cannot wait for lcore %u\n", i);
        }
    }
    // rte_eal_mp_wait_lcore();
    rte_flow_flush(_port_id, &error);
    rte_eth_dev_stop(_port_id);
    rte_eth_dev_close(_port_id);
    rte_eal_cleanup();
}