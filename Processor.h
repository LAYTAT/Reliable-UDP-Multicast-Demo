//
// Created by lay on 10/6/2021.
//
#include "mcast_include.h"
#include <queue>
#include <set>
#include <utility>

#define NUM_OF_TOTAL_PACKETS (16000)
class Processor{
public:
    Processor(int m_id, int l, int n = NUM_OF_TOTAL_PACKETS): machine_id(m_id), loss_rate(l), nums_packets(n){
    }
    Processor(Processor const &) = delete;
    Processor(Processor&&) = delete;
    bool start_ring();
    void start_chat();

    bool socket_init();

private:
    int machine_id;
    int aru = 0;                        //local aru, acumulatic acknoleged sequemce number for his processor
    int loss_rate = 0;
    int nums_packets;
    int port = PORT;
    std::queue<Message> msg_2b_sent;    //the messages that are waiting to be sent
    std::queue<Message> msg_received;   //the messages that are to be written into the file
    std::pair<int, struct sockaddr> next; //next neighbor in ring


    // socket
    int ss,sr;                          //sending & receiving socket fd
    struct sockaddr_in name;
    struct sockaddr_in send_addr;
    fd_set mask;
    fd_set read_mask, write_mask, excep_mask;
    struct ip_mreq mreq;
    unsigned char ttl_val;
    int mcast_addr;
    // add socket
};