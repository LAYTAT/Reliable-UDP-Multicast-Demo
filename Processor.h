//
// Created by lay on 10/6/2021.
//
#include "mcast_include.h"
#include <queue>
#include <set>
#include <utility>

#define NUM_OF_TOTAL_PACKETS (16000)
#define NUM_OF_MACHINES (10)

class Processor{
public:
    Processor(int m_id, int l, int n = NUM_OF_TOTAL_PACKETS, int nm = NUM_OF_MACHINES): machine_id(m_id), loss_rate(l), nums_packets(n), number_of_machines(nm){
    }
    Processor(Processor const &) = delete;
    Processor(Processor&&) = delete;
    bool start_mcast();
    bool form_ring();
    void start_chat();
    bool socket_init();

private:
    int machine_id;
    int aru = 0;                        //local aru, acumulatic acknoleged sequemce number for his processor
    int loss_rate = 0;
    int nums_packets;
    int number_of_machines;
    int port = PORT;
    std::queue<Message> msg_2b_sent;    //the messages that are waiting to be sent
    std::queue<Message> msg_received;   //the messages that are to be written into the file
    std::pair<int, struct sockaddr> next; //next neighbor in ring

    int                bytes;
    int                num;
    Message* recv_buf = new Message();
    bool mcast_received = false;
    bool ring_formed = false;


    // socket
    int ssm,srm;                          //sending & receiving socket fd for multicast
    int ssu;                              //socket fd for unicast
    struct sockaddr_in serv_addr;         // storing own addr, use for binding
    struct sockaddr_in name;
    struct sockaddr_in send_addr;
    fd_set mask;
    fd_set read_mask, write_mask, excep_mask;
    struct ip_mreq mreq;
    unsigned char ttl_val;
    int mcast_addr;
    // add socket
};