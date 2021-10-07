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
    Processor(Processor&&) = default;
    bool start_process();
    bool send_in_queue();
    
private:
    int machine_id;
    int aru = 0;                        //local aru, acumulatic acknoleged sequemce number for his processor
    int loss_rate = 0;
    int nums_packets;
    int port = PORT;
    std::queue<Message> msg_2b_sent;    //the messages that are waiting to be sent
    std::queue<Message> msg_received;   //the messages that are to be written into the file

    std::pair<int, struct sockaddr> prev; //previous neighbor in ring
    std::pair<int, struct sockaddr> next; //next neighbor in ring

    bool socket_init();
    // add socket
};