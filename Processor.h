//
// Created by lay on 10/6/2021.
//
#include "mcast_include.h"

#define NUM_OF_TOTAL_PACKETS (16000)
#define NUM_OF_MACHINES (10)

class Processor{
public:
    Processor(int m_id, int l, int n = NUM_OF_TOTAL_PACKETS, int nm = NUM_OF_MACHINES): machine_id(m_id), loss_rate(l), nums_packets(n), number_of_machines(nm){
        next_id = m_id%(number_of_machines) + 1;
        std::srand(std::time(nullptr));
        if(machine_id == 1) {
            gen_token(-1, -1, -1, rtr, 0, -1);
            has_token = true;
        }
        memset(recv_buf, 0, sizeof(Message));
        memset(msg_buf, 0, sizeof(Message));
        memset(token_buf, 0, sizeof(Token));
    }
    Processor(Processor const &) = delete;
    Processor(Processor&&) = delete;
    bool start_mcast();
    bool send_to_everyone();

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
    struct sockaddr * next_addr = NULL;
    int next_id = -1;                   //next neighbor in ring <m_id+1, address>
    //generate a new message and token for sending
    void gen_msg(int type, int seq);
    void gen_token(int seq, int aru, int last_aru_setter, std::set<int> &rtr, int round, int fcc);

    //message sending and receiving
    long unsigned int  bytes;
    int                num;
    Message* recv_buf = new Message();
    Message* msg_buf = new Message();
    Token* token_buf = new Token();
    std::set<int> rtr;

    //states
    bool mcast_received = false;        //start_mcast is received
    bool ring_formed = false;           //indicate whether the ring is formed
    bool has_token = false;

    // socket
    int ssm,srm;                        //sending & receiving socket fd for multicast
    int ssu;                            //socket fd for unicast
    struct sockaddr_in serv_addr;       // storing own addr, use for binding
    struct sockaddr_in name;
    struct sockaddr_in send_addr;
    fd_set mask;
    fd_set read_mask, write_mask, excep_mask;
    struct ip_mreq mreq;
    unsigned char ttl_val;
    int mcast_addr;
    // add socket
};