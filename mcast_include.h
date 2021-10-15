//
// Created by lay on 10/6/2021.
//

#ifndef MCAST_INCLUDE
#define MCAST_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include <ctime>
#include <queue>
#include <set>
#include <utility>
#include <set>
#include <arpa/inet.h>
#include <sys/time.h>
#include "recv_dbg.h"
#include <assert.h>
#include <algorithm>
#include <fstream>
#include <map>
#include <unordered_map>

#define MAX_MESS_LEN 1400
#define PORT (10040)
//#define PORT (10280) 
#define DATA_SIZE (1400)
#define MAX_FLOW_FOR_ONE_PROCESS (500)
#define MAX_FLOW_FOR_ONE_RING_CYCLE (5000)
#define MAX_RTR (300)
#define TOKEN_TIMEOUT_GAP_IN_SECONDS (1)
#define TOEKN_TIMEOUT_GAP_IN_USEC  (100000) //1 / 10 sec
#define NUM_OF_MEMBER (10)

struct Token{
    int seq;                        // The largest sequence number for any message, an upper limit of sequemce number
    int aru;                        // Used to demtermine if all processors on the ring have received all messages up until this number
    int last_aru_setter;            // the machine id of the last processor that have set aru OR the token initiator for ring forming round.
    unsigned int rtr_size;          // the number of requested
    unsigned int rtr[MAX_RTR];      // also called nacks, a request list that contains one or more retransmission requiest check vector size for request size
    unsigned int round;             // the round number in the ring cycle
    int fcc;                        // for flow control
};

enum class MSG_TYPE{
    START_MCAST = -1,
    REQUEST_RING = 1,
    TOKEN = -2,
    DATA = 2,
    EXIT = 3
};

struct Message{                     // or called packet
    MSG_TYPE type;                       // indicate different types of message, or token for example
    int seq;                        // global packet_index
    int pkt_idx;                    // packet index for each machine
    int machine_id;
    int random_num;
    //struct Token token;
    unsigned char payload[DATA_SIZE];  //1400 random payload or zero
};

#endif
