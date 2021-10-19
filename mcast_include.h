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
#include <unordered_map>

#define NUM_OF_TOTAL_PACKETS (16000)
#define GLOBAL_MAX (1600) //20:1600
#define LOCAL_MAX (160) //20:160
#define NUM_OF_MACHINES (10)
#define MAX_MESS_LEN 1400
//#define PORT (10040)  // Chanha
#define PORT (10280)  // Junjie
#define DATA_SIZE (1400) // cannot be changed
#define MAX_RTR (340)
#define TOKEN_TIMEOUT_GAP_IN_MSECONDS (1)
#define TOKEN_TIMEOUT_GAP_IN_USECONDS (100) //20:250/100

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
    MSG_TYPE type;                  // indicate different types of message, or token for example
    int seq;                        // global packet_index
    int pkt_idx;                    // packet index for each machine
    int machine_id;
    int random_num;
    //struct Token token;
    unsigned char payload[DATA_SIZE];  //1400 random payload or zero
};

struct Performance{
    long long msec;
    long long total_packet;
    long long pakcet_size_in_bytes;
};

#endif
