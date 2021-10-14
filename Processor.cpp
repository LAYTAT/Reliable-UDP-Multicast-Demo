#include "Processor.h"
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

bool Processor::start_mcast(){
    // get my address for later sending
    set_my_info();
    std::cout << "My machine id: " << machine_id << std::endl;
    std::cout << "My Host Name: " << my_hostname << std::endl;
    std::cout << "My Host IP: " << my_ip << std::endl;

    struct timeval select_timeout;
    select_timeout.tv_sec = 0;
    select_timeout.tv_usec = 0;

    // initialize recv debug mode
    recv_dbg_init( loss_rate, machine_id );
    std::cout << "Set machine" << machine_id << " recv loss rate to " << loss_rate << std::endl;

    for(;;)
    {
        read_mask = mask;
        /* get start time */
        num = select( FD_SETSIZE, &read_mask, &write_mask, &excep_mask, &select_timeout);
        if (num > 0) {
            if ( FD_ISSET(srm, &read_mask) ) {
                if(!mcast_received){ // if not recv mcast start receive using recv
                    bytes = recv(srm, (char*)recv_buf, sizeof(Message), 0);
                    if (bytes == -1) {
                        std::cerr << "Received Message Err" << std::endl;
                    } else if (bytes > 0 && bytes < sizeof(Message)) {
                        std::cerr << "Received Message Corrupted. Bytes Received:" << bytes << std::endl;
                    }
                    //Start_Mcast Message Received, start ring formation
                    if(recv_buf->type == MSG_TYPE::START_MCAST ) {
                        std::cout << "mcast_start msg received" << std::endl;
                        mcast_received = true;
                    }
                }

                bytes = recv_dbg(srm, (char*)recv_buf, sizeof(Message), 0);
                if (bytes == -1) {
                    std::cerr << "Received Message Err" << std::endl;
                } else if (bytes > 0 && bytes < sizeof(Message)) {
                    std::cerr << "Received Message Corrupted. Bytes Received:" << bytes << std::endl;
                } else if (bytes == 0) {
                    continue;
                }

                //Form a ring!
                if(mcast_received && !ring_formed) {
                    ring_formed = form_ring();
                    continue;
                }

                //ring formed, tranfer messages untill finished
                if(ring_formed) {
                    std::cout << "Ring:     Ring is formed" << std::endl;
                    // TODO: when received token with sent token round number, reset timer
                    // TODO: multicast only with updated token(with plus 1 round #)
                    is_all_data_received = data_tranfer();
                }

                if(is_all_data_received){
                    std::cout << "congratulation: everything is received" << std::endl;
                    break;
                }
            }
        }
        if(mcast_received && !ring_formed ){
            ring_request_multicast();           //keep multicast until token received
        }
        check_timeout(); //timeout for token
    }
    return false;
}

bool Processor::data_tranfer(){
    // TODO: multicast and updating on the token
    return false;
}

void Processor::ring_request_multicast(){
    //check if token recieved
    count = ( count + 1 ) % RING_MCAST_FREQ;

    if(count != 0) return;

    if((!had_token && machine_id != 1) || (machine_id == 1)) {
        // multicast in order let previous neighbor know your address in order to form the ring
        update_msg_buf(MSG_TYPE::REQUEST_RING);
        std::cout << "my ip sent"<< my_ip_ << std::endl;
        if(!send_to_everyone()){
            std::cerr << "send to everyone err" << std::endl;
        }
    }
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

bool Processor::send_token_to_next() {
    assert(has_next);
    long unsigned int bytes_sent = sendto(ssu, msg_buf, sizeof(Message), 0,(struct sockaddr *)&next_addr, sizeof(next_addr) );
    std::cout << "Sending:      machine " << machine_id << " sent token with round number " << token_buf->round << " to " << inet_ntoa(next_addr.sin_addr) << std::endl;
    if(bytes_sent == -1) {
        std::cerr << "Unicast Message Error." << std::endl;
        exit(1);
    }else if(bytes_sent < sizeof (Message)) {
        std::cerr << "Unicast Message Error. Bytes Sent:" << bytes << std::endl;
        return false;
    }
    has_token = false;
    last_token_round = token_buf->round;
    reset_token_timer();
    return true;
}

void Processor::update_msg_buf(MSG_TYPE type, int seq){
    memset(msg_buf, 0, sizeof(Message));
    msg_buf->type = type;
    msg_buf->machine_id = machine_id;
    if(seq != -1) {
        memset(msg_buf->payload, 0, sizeof(Message)); //TODO: add payload
        msg_buf->random_num = std::rand() % 1000000 + 1;
    }
    if(type == MSG_TYPE::REQUEST_RING) {
        memcpy(msg_buf->payload, my_ip_, strlen(my_ip_)); //send my_ip
        msg_buf->payload[strlen(my_ip)] = 0; // null char
    }
    if(type == MSG_TYPE::TOKEN) {
        memcpy(msg_buf->payload, token_buf, sizeof(Token));
    }
}

void Processor::update_token_buf(int seq, int aru, int last_aru_setter, std::set<int>& new_rtr, int round, int fcc){
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

void Processor::reset_token_timer(){
    std::cout << "Timer:    set for token with round number " << last_token_round << std::endl;
    token_flag = true;
    gettimeofday(&last_token_sent_time, nullptr);
}

void Processor::cancel_token_timer(){
    token_flag = false;
}

void Processor::check_timeout(){
    if(token_flag){
        gettimeofday(&timestamp, NULL);
        if (timestamp.tv_sec - last_token_sent_time.tv_sec >= TOKEN_TIMEOUT_GAP_IN_SECONDS){
            /* resend token */
            send_token_to_next();
            std::cout << "Timer:      Timeout! Token resend to machine "<< next_id <<" at timestamp " << timestamp.tv_sec << std::endl;
            gettimeofday(&last_token_sent_time,NULL);
        }
    }
}

bool Processor::form_ring() {
    switch (recv_buf->type) {
        case MSG_TYPE::TOKEN:
            memcpy(recv_buf->payload, token_buf, sizeof(Token));
            std::cout << "Received:     machine " << machine_id << " received token with round number " << token_buf->round << "." << std::endl;
            if(token_buf->round == last_token_round) {
                if(machine_id == 1) {
                    std::cout << "Ring:     Ring is formed!" << std::endl;
                    return true;
                } else {
                    std::cout << "already sent token(with the same round number)" << std::endl;
                    break;
                }
            }

            if(has_next && !had_token) {
                update_msg_buf(MSG_TYPE::TOKEN);
                send_token_to_next();
                had_token = true;
                //send token to next
            } else {
                std::cout << "machine " << machine_id << " does not have next" << std::endl;
            }
            if(!has_next && !had_token){
                has_token = true;
                had_token = true;
            }
            break;
        case MSG_TYPE::REQUEST_RING:
            if (next_id != recv_buf->machine_id) break;
            if (!has_next ) {
                std::cout << "REQUEST_RING from machine_id : " << recv_buf->machine_id << std::endl;
                char next_ip[strlen((const char *) recv_buf->payload)];
                memcpy(next_ip, recv_buf->payload, strlen((char *) recv_buf->payload));
                next_addr.sin_family = AF_INET;
                next_addr.sin_addr.s_addr = inet_addr(next_ip);// htonl(addr_binary);  /* ucast address */
                next_addr.sin_port = htons(PORT);
                std::cout << "Set next addr to: " << next_ip << "; s_addr = " << inet_addr(next_ip) << std::endl;
                has_next = true;
            }
            if(machine_id == 1 && has_next && !had_token) {
                std::cout << "Sending:      machine 1 is sending token" << std::endl;
                update_msg_buf(MSG_TYPE::TOKEN);
                send_token_to_next();
                had_token = true;
            }else if (has_token && has_next) {
                std::cout << "Sending:      machine " << machine_id << " is sending token" << std::endl;
                update_msg_buf(MSG_TYPE::TOKEN);
                send_token_to_next();
            }
            break;
        case MSG_TYPE::DATA:
            std::cout << "Received:     machine " << machine_id << " received data message with from machine " << msg_buf->machine_id << "." << std::endl;
            if(has_next && has_token && had_token) {
                return true;
            }
            break;
        default:
            break;
    }
    return false;
}


    // TODO: Special TODOs for node 0
    //  Whenever received a token with round number R, update the round number to (R+1);
    //  Reset the fcc to 0 for every circle.

    // TODO: TOKEN HANDLING


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

    /*socket for sending unicast*/
    ssu = socket(AF_INET, SOCK_DGRAM, 0);

    /*specify server address for binding*/
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    /* socket for receiving multicast */
    srm = socket(AF_INET, SOCK_DGRAM, 0);
    if (srm < 0) {
        perror("Mcast: socket");
        exit(1);
    }

    // TODO: for reuse of address. delete this setsockopt after debugging is done
    int set_resue = 1;
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
//    FD_SET(sru, &mask);
    FD_SET( (long)0, &mask );    /* stdin */
    return true;
}

void Processor::set_my_info() {
    struct hostent *host_entry;
    int hostname;

    // To retrieve hostname
    hostname = gethostname(my_hostname, sizeof(my_hostname));
    checkHostName(hostname);

    // To retrieve host information
    host_entry = gethostbyname(my_hostname);
    checkHostEntry(host_entry);

    // To convert an Internet network
    // address into ASCII string
    my_ip = inet_ntoa(*((struct in_addr*)
            host_entry->h_addr_list[0]));
    memcpy(&my_ip_, my_ip, strlen(my_ip));
}