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
        if( machine_id == 1 ) {
            update_token_buf(-1, -1, m_id, rtr, 0, -1);
        }
    }
    Processor(Processor const &) = delete;
    Processor(Processor&&) = delete;
    bool start_mcast();
    bool send_to_everyone();
    bool send_token_to_next();
    bool form_ring();
    void start_chat();
    bool socket_init();
    //sending multicast for previous neighbor in the ring
    void ring_request_multicast();

private:
    int machine_id;
    int aru = 0;                        //local aru, acumulatic acknoleged sequemce number for his processor
    int loss_rate = 0;
    int nums_packets;
    int number_of_machines;
    int port = PORT;
    std::queue<Message> msg_2b_sent;    //the messages that are waiting to be sent
    std::queue<Message> msg_received;   //the messages that are to be written into the file
    struct sockaddr_in temp_addr;
    int len = sizeof(temp_addr);
    struct sockaddr_in next_addr;
    int next_id;                   //next neighbor in ring <m_id+1, address>
    //generate a new message and token for sending
    void update_msg_buf(MSG_TYPE type, int seq=-1);
    void update_token_buf(int seq, int aru, int last_aru_setter, std::set<int> &new_rtr, int round, int fcc);
    void set_my_info();
    bool data_tranfer();

    //message sending and receiving
    int bytes;
    int num;
    //buffer for receiving
    Message* recv_buf = new Message();
    //buffer for sending
    Message* msg_buf = new Message();
    Token* token_buf = new Token();
    std::set<int> rtr;

    //states
    bool mcast_received = false;        //start_mcast is received
    bool ring_formed = false;           //indicate whether the ring is formed
    bool has_token = false;
    bool had_token = false;
    bool has_next = false;              //has next address
    bool is_all_data_received = false;  // if ture, then all everyone has everything

    // socket
    int ssm,srm;                        //sending & receiving socket fd for multicast
    int ssu;                        //socket fd for sending unicast
    struct sockaddr_in serv_addr;       // storing own addr, use for binding
    struct sockaddr_in name;
    struct sockaddr_in send_addr;

    // token timeout
    struct timeval timestamp;
    struct timeval last_token_sent_time;
    bool token_flag;
    void cancel_token_timer();
    void check_timeout();
    int last_token_round = -1;

    // ring multicast request
    int RING_MCAST_FREQ = 100000;
    int count = 0;

public:
    void reset_token_timer();

private:
    fd_set mask;
    fd_set read_mask, write_mask, excep_mask;
    struct ip_mreq mreq;
    unsigned char ttl_val;
    int mcast_addr;

    char my_hostname[256];
    char *my_ip;
    size_t ip_len;
};