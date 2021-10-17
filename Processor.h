//
// Created by lay on 10/6/2021.
//
#include "mcast_include.h"

class Processor{
public:
    Processor(int m_id, int l, int n = NUM_OF_TOTAL_PACKETS, int nm = NUM_OF_MACHINES): machine_id(m_id), loss_rate(l), nums_packets(n), number_of_machines(nm),  input_buf(){
        next_id = m_id%(number_of_machines) + 1;
        std::srand(std::time(nullptr));

    }
    Processor(Processor const &) = delete;
    Processor(Processor&&) = delete;
    Performance start_mcast();
    bool send_to_everyone();
    bool send_token_to_next();
    bool form_ring();
    void start_chat();
    bool socket_init();
    //sending multicast for previous neighbor in the ring
    void ring_request_multicast();
    void close_file();
    void close_sockets();

    void open_file();
    void deleteMap(std::map<int, Message *> map);
    std::map<int, Message *> msg_received_map; //keeps track of sequence numbers in msg_received, also cache
    FILE * fp; //file pointer for writing into the file

private:

    //data transfer
    int machine_id;
    int loss_rate = 0;
    int nums_packets;
    int number_of_machines;

    int port = PORT;

    int next_id;                   //next neighbor in ring <m_id+1, address>
    bool data_tranfer();
    int last_token_aru;
    std::vector<Message *> input_buf;
    std::set<int> input_set;
    int fwut = 0; //file written up to.

    int last_agreed_aru = 0;
    int agreed_aru_count = 0;


    //generate a new message and token for sending
    // when sending token: update_sending_token_buf, update_msg_buf, send token
    void update_msg_buf(MSG_TYPE type); //always update msg_buf before sending anything

    void update_sending_token_buf(int s, int a, int last_aru_setter, std::set<int> &new_rtr, int round, int fcc);
    void set_my_info();

    //message sending and receiving
    int bytes;
    int num;
    //buffer for receiving
    Message* recv_buf = new Message();
    //buffer for sending
    Message* msg_buf = new Message();
    Token* received_token_buf = new Token();
    Token* sending_token_buf = new Token();
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
    struct sockaddr_in temp_addr;
    int len = sizeof(temp_addr);
    struct sockaddr_in next_addr;

    // token timeout
    struct timeval timestamp;
    struct timeval last_token_sent_time;
    bool token_flag;
    void cancel_token_timer();      // TODO:
    void check_timeout();
    int last_token_round = -1;
//    int last_sent_token_seq = -1;  // TODO: try this
    void reset_token_timer();

    // ring multicast request
    int RING_MCAST_FREQ = 500000;  //test shows 500000 is a usable baseline
    int count = 0;

    fd_set mask;
    fd_set read_mask, write_mask, excep_mask;
    struct ip_mreq mreq;
    unsigned char ttl_val;
    int mcast_addr;

    // ip
    char my_hostname[256];
    char *my_ip;
    char my_ip_[13];
    size_t ip_len;

    // packets
    int aru = 0;                        //local aru, acumulatic acknoleged sequemce number for his processor
    int pkt_idx;

    int seq = 0;

    // token process
    void store_to_input();
    int find_max_messages();
    void update_rtr_aru_with_msg(int msg_seq);
    void flush_input_buf();
    int retransmission(int n);
    void update_rtr_with_token_seq();
    int broadcasting_new_messages(int m2);
    Message *make_Message(MSG_TYPE type, int s, int pkt, int id, int rand);

    // termination
    bool check_if_everybody_ready_to_exit();
    int ENDING_COUNT = 10;
    int BROADCASTING_TIMES = 50;
    int seq_equal_last_seq_and_aru_equal_seq_count = 0;

    void broadcast_exit_messages();

    void update_rtr_aru_with_new_broadcast(int brdcst_msg_seq);
};
