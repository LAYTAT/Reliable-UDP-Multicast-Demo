#include "Processor.h"
bool Processor::start_process(){

    // socket and bind
    if(!socket_init()) return false;
    
    // wait for start_mcast message
    // use recv
    // use multicast
    // add implmentation here

    // Ring formation Process
    // keep sending Message for join with others
    // until current process has got the token from machines[machine_id - 1]

    // Ring is formed, start reacting to token send from previous neighbor
    // Process the token 

    // Retransmission Stage
    // This stage only take place if (token.aru < Processor.aru)
    // add the requested packets to the msg_2b_sent queue, if do not have all requested packet, send until the newest one
    // when adding to the queue, if reach glabal cycle maximun or max for one process, enter sending stage
    if(!send_in_queue()) return false;

    // if flow allows
    // Even though the current broadcaster might not be able to send to all the requested packets(for that it might not have some of the requested packets itself), it will still send all the requested messages in the range of [0, local.aru]. The retransmission packets will not be sent immediately, it will be added to the queue.

    // Token Update
    // Seq
    // Seq number indicates the newest message sent by the broadcaster, which can be smaller than the previous token.seq if the broadcaster is resending packets. So when the broadcaster sends new packets, it will be updated to keep up.
    // Aru
    // If (token.aru == token.seq) 
    // then the broadcaster will send new data packets, and update the token.aru and the token. seq both to the newest sent packets to be sent. Packets will be put into a queue, and the actual sending will happen later.
    // Question: when does the broadcast take place? before or after the token unicast? We at the moment do not know how will this affect the overall protocol, we are currently doing a broadcast before sending the token to the next side of the ring.
    // If (token.aru < token.seq && token.aru < local.aru)
    // If(token.aru.who_lower_it_last_time
    // ==
    // current.machine_id)
    // So it is the one that lowered the aru, and the token.aru is still the same, should set token.aru to its local aru.
    // else 
    // Enter the Retransmission Stage and the token.aru stays unchanged.

    // If(token.aru > local.aru)


    // Lower the token.aru to the local aru. And update the token.to with the lost packets’ seq_nums. 
    // fcc: this should be used to control the bound of data flow. fcc itself is a count of the number of packets broadcast by all processors during the previous rotation of the token. It stays unchanged if no packets are to be sent, and adds with the new packets if there are packets to be sent.

    return false;
}

bool Processor::socket_init(){
    return false;
}

bool Processor::send_in_queue(){
    return false;
}