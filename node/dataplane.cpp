#include "dataplane.h"

/************************************Global Variables***************************************/

lcoreCommandQueue _command_queues[MAX_CORE_NUMS]; // Command queues for each core,to add or delete plugin

int _nextId = 1; // Next available plugin ID

std::mutex _mt[2][MAX_CORE_NUMS]; //_mt[0] is used for _command_queues update, _mt[1] is used for _core_queues update

struct rte_ring *rings[MAX_CORE_NUMS]; // 全局数组，用于存储每个线程的环形缓冲区

uint64_t cpu_tsc;  //每秒的时钟周期数
/************************************Utility Functions************************************/

void registerSubTask(MSSubTask *task)
{
    printf("registerPlugin\n");
    _TM.loadTask(task);
}

void unregisterSubTask(MSSubTask *task)
{
    _TM.unloadTask(task->filename);
}

void unregisterSubTask(unsigned int subtask_id)
{
    printf("unregister Plugin\n");
    MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(subtask_id);
    _TM.unloadTask(pi->task->filename);
}

void deleteMSSubTask(unsigned int subtask_id)
{
    printf("deletePlugin\n");
    deleteMSSubTaskRuntime(subtask_id);
}

// Add a flow rule to the specified queue
int addFlowFilter(uint16_t port_id, uint32_t markid,uint32_t priority, uint32_t src_ip, uint32_t src_mask, uint32_t dest_ip, uint32_t dest_mask)
{
    printf("addFlowToQueue\n");
    struct rte_flow_error error;
    flow_id *flow = generate_ipv4_flow_only(port_id, markid,priority,
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
void deleteFlowFilter(int id)
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

bool push_Command(Command c)
{

    int lcore_id;
    if (c.type == ADD_SUBTASK)
    {
        lcore_id = c.args.add_task_arg.coreid;
    }
    else if (c.type == DELETE_SUBTASK)
    {
        lcore_id = c.args.del_task_arg.coreid;
    }
    else if(c.type == ADD_SUBTASK_SELF){
        lcore_id = c.args.add_task_self_arg.trtnode->trt.subtask.core_id;
    }
    else
    {
        printf("error CommandType\n");
        return false;
    }
    printf("push_Command %d on core %d\n", c.type, lcore_id);
    _command_queues[lcore_id].mt.lock();
    if ((_command_queues[lcore_id].rear + 1) % SOCKET_QUEUE_SIZE == _command_queues[lcore_id].front)
    {
        printf("command queue is full\n");
        _command_queues[lcore_id].mt.unlock();
        return false;
    }
    else
    {
        _command_queues[lcore_id].queue[_command_queues[lcore_id].rear] = c;
        _command_queues[lcore_id].rear = (_command_queues[lcore_id].rear + 1) % SOCKET_QUEUE_SIZE;
    }
    _command_queues[lcore_id].mt.unlock();
    return true;
}


/************************************monitor Functions***************************************/
SubTaskPerformance get_subtask_performance(unsigned int subtask_id){
    SubTaskPerformance stpf;
    MSSubTaskHandleInfo *pi = _TM.getMSSubTaskHandleInfo_fromid(subtask_id);
    MSSubTask *ptt = pi->task;
    pthread_mutex_lock(&ptt->monitor_mt);
    stpf.task_id = ptt->id.id1.task_id;
    stpf.inner_subtask_id = ptt->id.id1.subtask_id;
    stpf.pos = ptt->pos;
    memcpy(stpf.recore_tscs, ptt->recore_tscs, sizeof(unsigned long long)*60);
    memcpy(stpf.recv_nums, ptt->recv_nums, sizeof(unsigned int)*60);
    memcpy(stpf.tsc_nums, ptt->tsc_nums, sizeof(unsigned long long)*60);
    pthread_mutex_unlock(&ptt->monitor_mt);
    return stpf;
}


/************************************System Functions***************************************/

// Daemon thread for each processing core
int handle_packet_per_core(void *arg)
{
    unsigned int lcore_id = rte_lcore_id();
    // std::cout << "startup lcore " << lcore_id << std::endl;
    struct rte_mbuf *mbufs[MAX_DEQUEUE_PACKETS];
    int record = 0;
    MSSubTaskRuntimeNode * current;
    unsigned int available;
    unsigned int dequeue_count;

    uint64_t start, end, cycles;
    uint64_t now_tsv=rte_rdtsc()+cpu_tsc;

    MSSubTask *ptt;
    while (1)
    {
        // 从环形缓冲区中取出报文
        dequeue_count = rte_ring_dequeue_bulk(rings[lcore_id], (void**)mbufs, MAX_DEQUEUE_PACKETS, &available);
        // 处理出队的报文
        for (unsigned int i = 0; i < dequeue_count; i++) {
            struct rte_mbuf *mbuf = mbufs[i];
            current = _handlers_[lcore_id].head;
            while (current != NULL)
            {
                if(mbuf->hash.fdir.hi & current->trt.pis[0].id.id2 ==
                    current->trt.pis[0].id.id2){
                    start = rte_rdtsc();
                    current->trt.recv_num ++;
                    int temp = current->trt.pis[0].func(mbuf, current->trt.pis[0].res);
                    while(temp != -1){
                        temp = current->trt.pis[temp].func(mbuf, current->trt.pis[temp].res);
                    }
                    end = rte_rdtsc();
                    current->trt.tsc_num += end - start;
                }
                current = current->next;
            }
            rte_pktmbuf_free(mbuf);
        }
        // if (record++ == 10000000)
        if (end > now_tsv)
        {
            _command_queues[lcore_id].mt.lock();
            int k = _command_queues[lcore_id].front;
            while (k != _command_queues[lcore_id].rear)
            {
                switch (_command_queues[lcore_id].queue[k].type)
                {
                    case ADD_SUBTASK_SELF:
                        addMSSubTaskRuntime(_command_queues[lcore_id].queue[k].args.add_task_self_arg.trtnode);
                        break;
                    case DELETE_SUBTASK:
                        deleteMSSubTaskRuntime(_command_queues[lcore_id].queue[k].args.del_task_arg.subtask_id);
                        break;
                    default:
                        break;
                }
                k = (k + 1) % SOCKET_QUEUE_SIZE;
            }
            _command_queues[lcore_id].front = k;
            _command_queues[lcore_id].mt.unlock();

            // 输出性能指标
            current = _handlers_[lcore_id].head;
            while (current != NULL){
                ptt = current->trt.subtask;
                pthread_mutex_lock(&ptt->monitor_mt);
                ptt->recv_nums[ptt->pos] = current->trt.recv_num;
                ptt->tsc_nums[ptt->pos] = current->trt.tsc_num;
                ptt->recore_tscs[ptt->pos] = now_tsv;
                ptt->pos = (ptt->pos + 1) % 60;
                pthread_mutex_unlock(&ptt->monitor_mt);
                current = current->next;
            }
            // record = 0;
            now_tsv += cpu_tsc;
        }
        rte_pause();
    }
}

uint32_t get_packet_mark(struct rte_mbuf *mbuf) {
    if (mbuf->ol_flags & RTE_MBUF_F_RX_FDIR_ID) {
        return mbuf->hash.fdir.hi;
    } else {
        return 0;
    }
}
// Daemon thread for each capture core
int capture_core_thread(void *arg){
    unsigned int lcore_id = rte_lcore_id();
    struct rte_mbuf *mbufs[DPDKCAP_CAPTURE_BURST_SIZE];
    uint32_t markid;
    int coreid;
    while (1)
    {
        // 从环形缓冲区中取出一个报文
        struct Queueid* temp = core_queues[lcore_id];
        while (temp != NULL) {
            const uint16_t nb_rx = rte_eth_rx_burst(_port_id, temp->data, mbufs, DPDKCAP_CAPTURE_BURST_SIZE);
            for (uint16_t i = 0; i < nb_rx; ++i)
            {
                // 匹配markid 发送至不同的环形缓冲区
                if (mbufs[i]->ol_flags & RTE_MBUF_F_RX_FDIR_ID){
                    markid = mbufs[i]->hash.fdir.hi;
                    coreid = (markid>>7)&0X3ff;
                    //发送到不同环形缓冲区
                    rte_ring_enqueue(rings[coreid], mbufs[i]);
                }
            }
            temp = temp->next;
        }
        rte_pause();
    }
}

// Daemon thread for each write core
int write_core_thread(void *arg){
    while(1){
        dequeueMSSubTaskRuntimeNodeDump();
    }
}

// Main thread to deploy child threads to each core
void run()
{
    //获取每秒的时钟周期数
    cpu_tsc  = rte_get_tsc_hz();

    rte_flow_error error;
    for (int i = 2; i < _num_cores; i++)
    {
        char ring_name[RTE_RING_NAMESIZE];

        // 为每个环形缓冲区创建一个唯一的名称
        snprintf(ring_name, sizeof(ring_name), "ring_%u", i);

        // 创建环形缓冲区
        rings[i] = rte_ring_create(ring_name, RING_SIZE, rte_socket_id(), 0);
        if (!rings[i])
        {
            rte_exit(EXIT_FAILURE, "Cannot create ring for lcore %u\n", i);
        }
        int ret = rte_eal_remote_launch(handle_packet_per_core, NULL, i);
        if (ret != 0)
        {
            rte_panic("Cannot launch auxiliary thread\n");
        }
    }
    int ret = rte_eal_remote_launch(capture_core_thread, NULL, 1);
    if (ret != 0)
    {
        rte_panic("Cannot launch auxiliary thread\n");
    }
    ret = rte_eal_remote_launch(write_core_thread, NULL, 1);
    if (ret != 0)
    {
        rte_panic("Cannot launch auxiliary thread\n");
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





