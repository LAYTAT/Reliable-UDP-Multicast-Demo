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
    void start_chat();
    bool socket_init();
    void close_file();
    void close_sockets();
    void open_file();
    void deleteMap(std::unordered_map<int, Message *> map);
    std::unordered_map<int, Message *> msg_received_map; //keeps track of sequence numbers in msg_received, also cache
    FILE * fp; //file pointer for writing into the file

private:
    //data transfer
    int machine_id;
    int loss_rate = 0;
    int nums_packets;
    int number_of_machines;
    int next_id;                   //next neighbor in ring <m_id+1, address>
    int last_token_aru;
    std::vector<Message *> input_buf;
    std::set<int> input_set;
    int fwut = 0; //file written up to.
    int last_local_aru;

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

    // socket and IP address
    fd_set mask;
    fd_set read_mask, write_mask, excep_mask;
    struct ip_mreq mreq;
    unsigned char ttl_val;
    int mcast_addr;
    int ssm,srm;                        //sending & receiving socket fd for multicast
    int ssu;                        //socket fd for sending unicast
    struct sockaddr_in serv_addr;       // storing own addr, use for binding
    struct sockaddr_in name;
    struct sockaddr_in send_addr;
    struct sockaddr_in temp_addr;
    int len = sizeof(temp_addr);
    struct sockaddr_in next_addr;
    char my_hostname[256];
    char *my_ip;
    char my_ip_[13];

    // token timeout
    struct timeval timestamp;
    struct timeval last_token_sent_time;
    struct timeval last_round_time;    // ring multicast request
    bool token_flag;

    int last_token_round = -1;

    int RING_MCAST_FREQ_FACTOR = 5000;  //test shows 500000 is a usable baseline
    int count = 0;

    // packets
    int aru = 0;                        //local aru, acumulatic acknoleged sequemce number for his processor
    int pkt_idx;
    int seq = 0;

    // timer
    void reset_token_timer();
    void cancel_token_timer();
    void check_timeout();

    // ring
    void set_my_info();
    bool send_to_everyone();
    bool send_token_to_next();
    bool form_ring();
    void ring_request_multicast();  //sending multicast for previous neighbor in the ring

    //generate a new message and token for sending
    // when sending token: update_sending_token_buf, update_msg_buf, send token
    void update_msg_buf(MSG_TYPE type); //always update msg_buf before sending anything
    void update_sending_token_buf(int s, int a, int last_aru_setter, int rtr_size, std::set<int> &new_rtr, int round, int fcc);

    // token and msg process
    bool data_tranfer();
    void store_to_input();
    int find_max_messages();
    void update_rtr_aru_with_msg(int msg_seq);
    void update_rtr_with_token_seq();
    void update_rtr_aru_with_new_broadcast(int brdcst_msg_seq);
    void flush_input_buf();
    int retransmission(int m);
    int broadcasting_new_messages(int m2);
    void write_to_file();
    Message *make_Message(MSG_TYPE type, int s, int pkt, int id, int rand);

    // termination
    int ENDING_COUNT = 2;
    int BROADCASTING_TIMES = 50;
    int seq_equal_last_seq_and_aru_equal_seq_count = 0;
    bool check_if_everybody_ready_to_exit();
    void broadcast_exit_messages();

    // checking
    int total_rtr_count = 0;
};
