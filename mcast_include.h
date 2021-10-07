//
// Created by lay on 10/6/2021.
//

#ifndef MCAST_INCLUDE
#define MCAST_INCLUDE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <queue>
#include <iostream>
#include <vector>

#define PORT (10040)
//#define PORT (10280) 
#define DATA_SIZE (1400)
#define MAX_FLOW_FOR_ONE_PROCESS
#define MAX_FLOW_FOR_ONE_RING_CYCLE

struct Message{         // or called packet
    int type;           // indicate different types of message, join message for example
    int seq;            // the global sequence number for the current packet
    int machine_id;
    int random_num;
    unsigned char data[DATA_SIZE]; // random data or zero
};

struct Token{
    int seq;            // The largest sequence number for any message, an upper limit of sequemce number
    int aru;            // Used to demtermine if all processors on the ring have received all messages up until this number
    int last_aru_setter;// the machine id of the last processor that have set aru
    std::vector<int> rtr;    // also called nacks, a request list that contains one or more retransmission requiest
                        // check vector size for request size.
};



#endif
