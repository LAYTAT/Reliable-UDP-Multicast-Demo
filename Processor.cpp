#include "Processor.h"
bool Processor::start_mcast(){
    // TODO: ADD a big big while loop - Begin


    for(;;)
    {
        read_mask = mask;
        num = select( FD_SETSIZE, &read_mask, &write_mask, &excep_mask, NULL);
        if (num > 0) {
            if ( FD_ISSET(srm, &read_mask) ) {
                bytes = recvfrom(srm, recv_buf, sizeof(Message), 0, (sockaddr *)&temp_addr, (socklen_t  *)&len);
                if (bytes == -1) {
                    std::cerr << "Received Message Err" << std::endl;
                } else if (bytes < sizeof(Message)) {
                    std::cerr << "Received Message Corrupted. Bytes Received:" << bytes << std::endl;
                }

                //Start_Mcast Message Received, start ring formation
                if(recv_buf->type == -1) {
                    std::cout << "mcast_start msg received" << std::endl;
                    mcast_received = true;
                }

                //Form a ring!
                if(mcast_received && !ring_formed) {
                    ring_formed = form_ring(temp_addr);
                }

                if(ring_formed) {

                }


            }
        }
    }
    return false;
}

bool Processor::send_to_everyone(){
    long unsigned int bytes_sent = sendto(ssm, msg_buf, sizeof(Message), 0,(struct sockaddr *)&send_addr, sizeof(send_addr) );
    if(bytes_sent == -1) {
        std::cerr << "Multicast Message Error." << std::endl;
        exit(1);
    }else if(bytes_sent < sizeof (Message)) {
        std::cerr << "Multicast Message Error. Bytes Sent:" << bytes << std::endl;
        return false;
    }
    return true;
}

void Processor::gen_msg(int type, int seq = -1){
    memset(msg_buf, 0, sizeof(Message));
    msg_buf->type = type;
    if (type == -2) { //its a token!
        memcpy(&msg_buf->token, token_buf, sizeof(Token));
    }
    msg_buf->machine_id = machine_id;
    if(seq != -1) {
        memset(msg_buf->payload, 0, sizeof(Message)); //TODO: add payload
        msg_buf->random_num = std::rand() % 1000000 + 1;
    }
}

void Processor::gen_token(int seq, int aru, int last_aru_setter, std::set<int>& new_rtr, int round, int fcc){
    memset(token_buf, 0 , sizeof(Message));
    token_buf->seq = seq;
    token_buf->fcc = fcc;
    token_buf->last_aru_setter = last_aru_setter;
    token_buf->aru = aru;
    token_buf->round = round;
    int count = 0;
    for(auto itr = new_rtr.begin(); itr != new_rtr.end() && count < MAX_RTR; ++itr){
        token_buf->rtr[count] = *itr;
        count++;
    }
    if(count >= MAX_RTR) std::cerr << "Request overflow!" << std::endl;
}


/* It forms a ring between the N machines.
 * parameter is a reference of a potential next address
 * Returns true if ring formed succesfully, false otherwise
 */
// send token if you recieved a message from a previous positioned ring & you have token
// two cases: 1) recieved a token from previous process 2) recieved a message from the next process
bool Processor::form_ring(sockaddr_in & addr) {

    if (recv_buf->type == -2) { //token recieved



    }




    //check if token recieved
    if(!had_token) {
        // multicast in order let previous neighbor know your address
        gen_msg(1);
        send_to_everyone(); //multicast
        return false;
    }




    if(machine_id == 1 && has_token ){
//        When the start_mcast packet is received, node 0 will initialize a token that contains:
//        machine-id
//        round number, which is 0
//        Keep waiting for node 1 to send its initial packet.
//                When receive node 1’s initial packet, it will store the address of node 1 in loca storage and
//        Send the token to node 1
//        Timeout for token(which has a round number of 0)
//        If timeout
//        Resend the token
//        If token with round number 0 circled back
//        Round number plus one
//        Enter the payload sending stage
    } else {
//        process start_mcast sends start_mcast packets to every node
//        Marked each node for machine id in the range of [1, 10]
//            For each node in the range [1, 10]
//                When the start_mcast packet is received, the node will start an initialization process, the process will not stop until the token is received.
//                while the node does not have received token from the previous node in the ring
//                    It keeps sending an Init packet, which contains:
//                        machine_id
//                        node address(implicitly)
//            For each node 0 (the initial token holder)
        }
    // check for token



    // TODO: looking for next machine id
    if(!next_addr) {

    }
    return true;
}


    // TODO: Special TODOs for node 0
    //  Whenever received a token with round number R, update the round number to (R+1);
    //  Reset the fcc to 0 for every circle.

    // TODO: TOKEN HANDLING
/*  General flow upon receiving the token:
    ---------(beginning of retransmission stage)---------------------------------------
            Flow Control
    determine how many messages this processor can broadcast determined.
            The maximum number of messages sending for this process is defined as followed:
    GLOBAL_MAXIMUM, flow control for one ring cycle
    LOCAL_MAXMUM, flow control for one process
    Calculate global_balance
    global_balance = GLOBAL_MAXIMUM - token.fcc
    local_balance = min(LOCAL_MAXMUM, global_balance)
    The local_balance will be the maximum number of packets that a process can send.

            update retransmission requests (rtr in the token)
    broadcast the requested transmission
    subtract the number of retransmissions took place from the allowed to broadcast
    ----------(end of retransmission stage)----------------------------------------------
            ----------(beginning of broadcasting new messages)---------------------------
    for as many messages the process broadcast:
    get a message from the msg_2b_sent queue
    increment token sequence (token.seq)
    set message fields and broadcast
    ------------(end of broadcasting new messages)---------------------------------
            -------------(beginning of local variables & token update)----------------------
            update local variable my_aru
    if my_aru < token_aru or local.id == token.last_aru_setter or token.last_aru_setter == null then,
            token_aru = local.aru
    if token.aru == token.seq then
    token.last_aru_setter = null
    else token.last_aru_setter = local.id

    Retransmission Stage (1)
    This stage only take place if (token.aru < local.aru)
        This is where the broadcaster tries to resend the requested packets, and after this preprocessing, the broadcaster will resend all the packets other than the unsent ones.
        Even though the current broadcaster might not be able to send to all the requested packets(for that it might not have some of the requested packets itself), it will still
        send all the requested messages in the range of [0, local.aru]. The retransmission packets will not be sent immediately, it will be added to the queue.
*/

    // TODO: WHEN RECEIVING A REGULAR MESSAGE
//    If token retransmission timeout is set, check if this broadcast message’s machine-id is the set one, if so, cancel the token retransmission timeout.
//            Put this newly received message to msg_received queue, if msg.seq is bigger than the local aru.
//            Write the consecutive payload in msg_received into the file.
//            Update the current process’s retransmission request list

    // TODO: TIMEOUT

    // TODO: TERMINATION
    //    Termination
    //    Add a local variable for each process: last_token_aru which keep the last round of token aru
    //    When last_token_aru == maximum_seq_num and last_token_aru == current token.aru, this will mean
    //    that everybody can exit the ring for that no one is requesting for message retransmission anymore.


    // TODO: ADD a big big while loop - End


void Processor::start_chat(){
    int                bytes;
    int                num;
    char               mess_buf[MAX_MESS_LEN];
    char               input_buf[80];

    for(;;)
    {
        read_mask = mask;
        num = select( FD_SETSIZE, &read_mask, &write_mask, &excep_mask, NULL);
        if (num > 0) {
            if ( FD_ISSET(srm, &read_mask) ) {
                bytes = recv(srm, mess_buf, sizeof(mess_buf), 0 );
                mess_buf[bytes] = 0;
                printf( "received : %s\n", mess_buf );
            }else if( FD_ISSET(0, &read_mask) ) {
                bytes = read( 0, input_buf, sizeof(input_buf) );
                input_buf[bytes] = 0;
                printf( "there is an input: %s\n", input_buf );
                sendto(ssm, input_buf, strlen(input_buf), 0,
                       (struct sockaddr *)&send_addr, sizeof(send_addr) );
            }
        }
    }
}

bool Processor::socket_init(){
    mcast_addr = 225 << 24 | 0 << 16 | 1 << 8 | 1; /* (225.0.1.1) mcast IP group*/

    /*socket for receiving unicast*/
    ssu = socket(AF_INET, SOCK_DGRAM, 0);
    if (ssu < 0) {
        perror("Ucast: socket");
        exit(1);
    }

    /*specify server address for binding*/
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // TODO: for reuse of address. delete this setsockopt after debugging is done
    int set_resue = 1;
    if (setsockopt(ssu, SOL_SOCKET, SO_REUSEADDR, &set_resue, sizeof(int)) < 0){ //SO_REUSEPORT
        perror("Mcast: set reuse failed");
        exit(1);
    }

    /*unicast server socket*/
    if ( bind( ssu, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) < 0 ) {
        perror("rcv: bind err");
        exit(1);
    }

    /* socket for receiving multicast */
    srm = socket(AF_INET, SOCK_DGRAM, 0);
    if (srm < 0) {
        perror("Mcast: socket");
        exit(1);
    }

    // TODO: for reuse of address. delete this setsockopt after debugging is done
    set_resue = 1;
    if (setsockopt(srm, SOL_SOCKET, SO_REUSEADDR, &set_resue, sizeof(int)) < 0){ //SO_REUSEPORT
        perror("Mcast: set reuse failed");
        exit(1);
    }

    /* socket for receiving multicast */
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(PORT);
    if (bind(srm, (struct sockaddr *)&name, sizeof(name) ) < 0 ) {
        perror("Mcast: bind");
        exit(1);
    }

    mreq.imr_multiaddr.s_addr = htonl( mcast_addr );

    /* the interface could be changed to a specific interface if needed */
    mreq.imr_interface.s_addr = htonl( INADDR_ANY );

    if (setsockopt(srm, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&mreq,
                   sizeof(mreq)) < 0)
    {
        perror("Mcast: problem in setsockopt to join multicast address" );
    }

    ssm = socket(AF_INET, SOCK_DGRAM, 0); /* Socket for sending */
    if (ssm < 0) {
        perror("Mcast: socket");
        exit(1);
    }

    ttl_val = 1;
    if (setsockopt(ssm, IPPROTO_IP, IP_MULTICAST_TTL, (void *)&ttl_val,
                   sizeof(ttl_val)) < 0)
    {
        printf("Mcast: problem in setsockopt of multicast ttl %d - ignore in WinNT or Win95\n", ttl_val );
    }

    send_addr.sin_family = AF_INET;
    send_addr.sin_addr.s_addr = htonl(mcast_addr);  /* mcast address */
    send_addr.sin_port = htons(PORT);

    FD_ZERO( &mask );
    FD_ZERO( &write_mask );
    FD_ZERO( &excep_mask );
    FD_SET(srm, &mask );
    FD_SET( (long)0, &mask );    /* stdin */
    return true;
}