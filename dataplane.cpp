#include "dataplane.h"
DataPlane::DataPlane()
{
    _record = 0;
    std::unique_ptr<PluginInterface> p = new PluginTest0(-1);
    _handlers.push_back(p);
    _handlers_updated.push_back(p);
    _nextId = 1
}

void DataPlane::addPlugin(std::unique_ptr<PluginInterface> newPlugin)
{
    _mt.lock();
    _handlers_updated.push_back(newPlugin);
    _mt.unlock();
}
void DataPlane::configurePort(unsigned int port_id){
    _port_id = port_id;
}
void DataPlane::configureNumCores(unsigned int numcores){
    _num_cores = numcores;
    _record.resize(numcores);
    _mt.resize(numcores);
}

void DataPlane::init(int argc, char **argv){
    int ret = rte_eal_init(1, NULL);
    if (ret < 0) rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");
    
    _num_cores = rte_lcore_count();
    if (_num_cores < 2) rte_exit(EXIT_FAILURE, "Insufficient cores\n");
}
void DataPlane::handle_packet_per_core(){
    unsigned int lcore_id = rte_lcore_id();
    std::cout<<"startup lcore "<<lcore_id<<std::endl;
    struct rte_mbuf* mbufs[DPDKCAP_CAPTURE_BURST_SIZE];
    struct rte_ether_hdr* eth_hdr;

    // 接收报文并处理
    while (1) {
        const uint16_t nb_rx = rte_eth_rx_burst(_port_id, 0, mbufs, DPDKCAP_CAPTURE_BURST_SIZE);
        
        for (uint16_t i = 0; i < nb_rx; ++i) {
            eth_hdr = rte_pktmbuf_mtod(mbufs[i], struct rte_ether_hdr*);
            if (ntohs(eth_hdr->ether_type) == RTE_ETHER_TYPE_VLAN)
            {
                struct rte_vlan_hdr *vlan_hdr = rte_pktmbuf_mtod_offset(mbufs[i], struct rte_vlan_hdr *, sizeof(struct rte_ether_hdr));
                for (int j = 0; j < _handlers[lcore_id].size(); j++)
                {
                    _handlers[lcore_id][j]->process(vlan_hdr, mbufs[i]);  
                }
            }

            rte_pktmbuf_free(mbufs[i]);
        }
        _record[lcore_id]++;
        if (_record[lcore_id] == 10000)
        {
            _mt[lcore_id].lock();
            _handlers[lcore_id] = _handlers_updated[lcore_id];
            _mt[lcore_id].unlock();
            _record[lcore_id] = 0;
        }
    }
    
}
void DataPlane::run(int argc, char **argv)
{
    for(int i=0;i<_num_cores;i++){
        rte_eal_remote_launch(handle_packet_per_core,)
    }
    rte_eal_mp_wait_lcore();
    rte_flow_flush(port_id, &error);
    rte_eth_dev_stop(port_id);
    rte_eth_dev_close(port_id);
    rte_eal_cleanup();
}

void DataPlane::updateTest()
{
    int id = 0;
    while (true)
    {
        std::cout << "add " << id << std::endl;
        PluginInterface *p = new PluginTest1(id++);
        sleep(100);
        addPlugin(p);
        sleep(1000);
    }
}
void DataPlane::registerPlugin(std::unique_ptr<PluginInterface> plugin)
{
    // 为插件分配编号并注册
    plugin->setID(_nextId++);
    _plugins[_nextId] = std::move(plugin);
}

void DataPlane::unregisterPlugin(std::unique_ptr<PluginInterface> plugin)
{
    // 在 plugins 列表中查找并移除指定的插件
    auto it = std::find_if(_plugins.begin(), _plugins.end(), [&](const auto &p)
                           { return p.second()->getName() == plugin->getName(); });
    if (it != _plugins.end())
    {
        _plugins.erase(it);
    }
}