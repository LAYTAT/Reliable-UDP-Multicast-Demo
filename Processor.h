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
        gen_token(-1, -1, m_id, rtr, 0, -1);

    }
    Processor(Processor const &) = delete;
    Processor(Processor&&) = delete;
    bool start_mcast();
    bool send_to_everyone();

    bool form_ring(sockaddr_in & addr);
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
    struct sockaddr_in temp_addr;
    int len = sizeof(temp_addr);
    struct sockaddr_in next_addr;
    int next_id;                   //next neighbor in ring <m_id+1, address>
    //generate a new message and token for sending
    void gen_msg(int type, int seq);
    void gen_token(int seq, int aru, int last_aru_setter, std::set<int> &rtr, int round, int fcc);
    void set_my_info();

    //message sending and receiving
    long unsigned int  bytes;
    int                num;
    Message* recv_buf = new Message(); //receiving
    Message* msg_buf = new Message(); //sending
    Token* token_buf = new Token();
    std::set<int> rtr;

    //states
    bool mcast_received = false;        //start_mcast is received
    bool ring_formed = false;           //indicate whether the ring is formed
    bool has_token = false;
    bool had_token = false;

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


    char my_hostname[256];
    char *my_ip;
    // Functions for getting address
    // Returns hostname for the local computer
    void checkHostName(int hostname)
    {
        if (hostname == -1)
        {
            perror("gethostname");
            exit(1);
        }
    }
    // Returns host information corresponding to host name
    void checkHostEntry(struct hostent * hostentry)
    {
        if (hostentry == NULL)
        {
            perror("gethostbyname");
            exit(1);
        }
    }
    // Converts space-delimited IPv4 addresses
    // to dotted-decimal format
    void checkIPbuffer(char *IPbuffer)
    {
        if (NULL == IPbuffer)
        {
            perror("inet_ntoa");
            exit(1);
        }
    }
};