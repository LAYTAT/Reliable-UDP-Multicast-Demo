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
                    continue;
                }

                bytes = recv_dbg(srm, (char*)recv_buf, sizeof(Message), 0);
                if (bytes == -1) {
                    std::cerr << "Received Message Err" << std::endl;
                } else if (bytes > 0 && bytes < sizeof(Message)) {
                    std::cerr << "Received Message Corrupted. Bytes Received:" << bytes << std::endl;
                } else if(bytes == 0) {
                    if (recv_buf->type == MSG_TYPE::TOKEN){
                        std::cerr << "Lost Token machine id: " << recv_buf->machine_id << std::endl;
                    } else {
                        std::cerr << "Lost Msg from machine id: " << recv_buf->machine_id << std::endl;
                    }
                    continue;
                }

                //Form a ring!
                if(mcast_received && !ring_formed) {
                    ring_formed = form_ring();
                }

                //ring formed, tranfer messages untill finished
                if(ring_formed) {
                    is_all_data_received = data_tranfer();
                }

                if(is_all_data_received){
                    std::cout << "congratulation: everything is received" << std::endl;
                    break;
                }
            } else if (FD_ISSET( srm, &excep_mask) ){
                std::cout << "exception for srm " << std::endl;
            }
            else {
                std::cout << "msg for non-srm " << std::endl;
            }
        }
        if(mcast_received && !ring_formed ){
            ring_request_multicast();           //keep multicast until token received
        }
        check_timeout(); //timeout for token
    }
    return false;
}

void Processor::store_to_input() {
    Message * message = make_Message(MSG_TYPE::DATA, recv_buf->seq, recv_buf->pkt_idx, recv_buf->machine_id, recv_buf->random_num);
    input_buf.push_back(message);
    //input_set.insert(recv_buf->seq);
    std::cout << "I just stored to input_buf, and its content: seq: " << message->seq << std::endl;
}

void Processor::update_rtr() {
    //update rtr by checking token_buf->seq
    for(int i = aru + 1; i <= token_buf->seq; ++i) {
        if(input_set.count(i)==0)
            rtr.insert(i);
    }
}

void Processor::update_rtr_aru(int msg_seq) {
    //update aru if this one connected
    input_set.insert(msg_seq);
    rtr.erase(msg_seq);
    for(auto itr = input_set.find(msg_seq); itr != input_set.end(); itr++){
        if(*itr == aru + 1) {
            aru++;
        } else break;
    }
    // update rtr
    for(int i = aru + 1; i < msg_seq; ++i) {
        if(input_set.count(i)==0)
            rtr.insert(i);
    }
}

bool Processor::data_tranfer(){


    switch (recv_buf->type) {
        case MSG_TYPE::DATA: {

            std::cout << "Data Message Recieved, SEQ: " << recv_buf->seq << "pkt idx: " << recv_buf->pkt_idx <<
                      "from machine: " << recv_buf->machine_id << "rand: " << recv_buf->random_num << std::endl;

            std::cout << "My ARU is: " << aru << std::endl;


            reset_token_timer();

            //we recieved a multicast data
            //temp seq is the message seq
            int temp_seq = 0;
            temp_seq = recv_buf->seq ;

            //ignore data you already have
            if (temp_seq <= aru) {
                std::cout << "I already received this seq number" << std::endl;
                break;
            }

            //push new message into the input_buffer
            store_to_input();

            //update_rtr_aru wanted action
            // recieved 1, 2, 4, //3 will go to the rtr, aru = 2, 1,2,4 is in the input buffer
            // recieved 5, 6, 8 // aru = 2, rtr = 3,7, and 1,2,4,5,6,8 in input buffer
            // recieved 3, 9 //aru = 6, rtr =,,,,//update_rtr_aru wanted action
            //            // recieved 1, 2, 4, //3 will go to the rtr, aru = 2, 1,2,4 is in the input buffer
            //            // recieved 5, 6, 8 // aru = 2, rtr = 3,4,7, and 1,2,4,5,6,8 in input buffer
            //            // recieved 3, 9 //aru = 6, rtr =,,,,
            //            // sort buffer, aru = last continous integer in the buffer, rtr = from aru (4) to input_buf last element (10)...
            // sort buffer, aru = last continous integer in the buffer, rtr = from aru (4) to input_buf last element (10)...
            update_rtr_aru(temp_seq);
            std::cout << "After Processing this Message, My ARU is " << aru << std::endl;
            break;
        }
        case MSG_TYPE::TOKEN: {

            memcpy(token_buf, recv_buf->payload, sizeof(Token));

            //if round number is 50 break TODO: fix this ending condition
            if (token_buf->round >= 50) {
                return true;
            }

            std::cout << "Received token info: seq: " << token_buf->seq << "aru: " << token_buf->aru <<
                      "las: " << token_buf->last_aru_setter << "round: " << token_buf->round << "fcc: " << token_buf->fcc << std::endl;

            int token_aru_received = token_buf->aru;

            std::cout << "When I Received a Token, My ARU is  " << aru << "My seq idx: " << seq << std::endl;


            if(token_buf -> round == last_token_round && machine_id != 1) break;

            cancel_token_timer();
            //we recieved a token
            //copy token data into our local token_buf

            std::cout << "Token Recieved with Round Number: " << token_buf->round << std::endl;

            //flush our input buffer by writing them or retain them.
            /*
             * Updating data structures
             */
            flush_input_buf(); // writes to a file, updates msg_received, input empty
            std::cout << "input flush success!" <<std::endl;

            //find max number of messages that can be sent by this processor
            int m = find_max_messages();

            //update retransmission request by looking at token seq
            update_rtr();

            //find number of max retransmissions
            int num_retrans = std::min(m, (int)token_buf->rtr_size);

            //r is the number of retranmission happened
            int r = retransmission(num_retrans);
            std::cout << "Retransmission success! number of retransmission was: " << r << std::endl;
            //subtract number of retransmissions from m, call it m2
            int m2 = m - r;
            int b = 0;

            if (token_buf->seq == token_buf->aru) {
                //updates token_aru, local_aru, and token_seq as broadcasting messages
                b = broadcasting_new_messages(m2);
                token_buf->last_aru_setter = 0;
            }
            if (token_buf->seq - b == aru) {
                aru += b;
            }

            //update token parameters
            if (aru < token_buf->aru || machine_id == token_buf->last_aru_setter || token_buf->last_aru_setter == 0) {
                token_buf->aru = aru;
                if (token_buf->aru == token_buf->seq) {
                    token_buf->last_aru_setter = 0;
                } else {
                    token_buf->last_aru_setter = machine_id;
                }
            }
            //token_aru updated, token_last setter updated, token seq updated, update token rtr now before sending a token
            //iterate through set rtr and put it into token_buf rtr
            //update_token_buf(int seq, int aru, int last_aru_setter, std::set<int>& new_rtr, int round, int fcc){
            int token_seq = token_buf->seq; int token_aru = token_buf->aru; int last_aru_setter = token_buf->last_aru_setter;
            int round = token_buf->round;
            int fcc = token_buf->fcc;
            if (machine_id == 1) { //handles the machine id = 1, round update and fcc update
                std::cout << "I incremente token to " << round + 1 << std::endl;
                round = token_buf->round + 1;
                fcc = 0;
            }
            fcc = fcc + r + b;

            std::cout << "Token Updated to seq: " << token_seq << "aru: " << token_aru <<
            "las: " << last_aru_setter << "round: " << round << "fcc: " << fcc << std::endl;

            last_token_aru = token_aru_received;

            //update token_buf
            update_token_buf(token_seq, token_aru, last_aru_setter, rtr, round, fcc);
            update_msg_buf(MSG_TYPE::TOKEN);
            send_token_to_next();
            break;
        }
        default:
            break;
    }

    return false;
}

//broadcasting new messages
//think about pkt_index, seq_num, recieved_msg,
int Processor::broadcasting_new_messages(int m2) {

    int b =0;
    for (int i = 0; i < m2; i++) {
        if (pkt_idx == nums_packets) {
            break;
        }
        token_buf->seq++; pkt_idx++;
        seq = token_buf->seq;
        update_msg_buf(MSG_TYPE::DATA);
        msg_received_map.insert(std::make_pair(msg_buf->seq,
                                               make_Message(msg_buf->type, msg_buf->seq, msg_buf->pkt_idx, msg_buf->machine_id, msg_buf->random_num)));
        std::cout << "Data Message Sent, SEQ: " << msg_buf->seq << "pkt idx: " << msg_buf->pkt_idx <<
                  "from machine: " << msg_buf->machine_id << "rand: " << msg_buf->random_num << std::endl;

        send_to_everyone();
        b++;
    }
    return b;
}

Message * Processor::make_Message(MSG_TYPE type, int s, int pkt, int id, int rand) {
    Message * m = new Message();
    memset(m->payload, 0, sizeof(m->payload));
    m->type = type;
    m->seq = s;
    m->pkt_idx = pkt;
    m->machine_id = id;
    m->random_num = rand;
    return m;
}

void Processor::deleteMap(std::map<int, Message *> map) {
    for (int i = 0; i < map.size(); i++) {
        delete map[i];
    }
}

//update our own rtr first
//input: number of maximum retransmission
//output: returns number of retransmissions happened
//update token_buf->with new rtrs
int Processor::retransmission(int n) {

    std::vector<int> resent_rtrs;
    std::vector<int> unresent_rtrs;

    for (int i = 0; i < token_buf->rtr_size; i++) {
        if (msg_received_map.count(token_buf->rtr[i]) == 0) {
            unresent_rtrs.push_back(token_buf->rtr[i]);
            if (rtr.find(token_buf->rtr[i]) == rtr.end()) {
                std::cout << "token contains unexpected rtr" << std::endl;
            }
            rtr.insert(unresent_rtrs[i]);
            continue;
        }
        sendto(ssm, msg_received_map[token_buf->rtr[i]], sizeof(Message), 0,(struct sockaddr *)&send_addr, sizeof(send_addr));
        resent_rtrs.push_back(token_buf->rtr[i]);
    }

    return resent_rtrs.size();
}

//for the rest of the input buffer gets copied into the msg_recieved data structure
// purpose 1) wait for to be written, 2) for retransmission cache
//for each round, put it into the queue in the seq order
void Processor::flush_input_buf() {


    //copy everything from input buf to msg_recieved
    for (int i = 0; i < input_buf.size(); i++) {
        //msg_received.push(input_buf[i]);
        msg_received_map.insert(std::make_pair(input_buf[i]->seq,
                                               make_Message(input_buf[i]->type, input_buf[i]->seq, input_buf[i]->pkt_idx, input_buf[i]->machine_id, input_buf[i]->random_num)));
    }
    //empty the input buffer
    for (int i = 0; i < input_buf.size(); i++) {
        delete input_buf[i];
    }
    input_buf.clear();

    //write to file as much as we can from the msg_recieved
    //upper limit is upto agreed_aru
    //lower limit is fwut (file written up to), if it's n, then n sequence numbers have been written
    //so, look for n+1 and increment if yes
    assert(fwut == last_agreed_aru);
    int agreed_aru = std::min(last_token_aru, token_buf->aru);


    int fwut_count = 0;
    for (int i = fwut + 1; i <= agreed_aru; i++) {
        if (agreed_aru == 0) {
            break;
        }
        // i is the sequence number we can write into, i.e. we can find it from the msg_recieved!
        assert(msg_received_map.count(i) == 1);
        int bytes_written = fprintf(fp, "%2d, %8d, %8d\n", msg_received_map[i]->machine_id, msg_received_map[i]->pkt_idx, msg_received_map[i]->random_num);
        std::cout << "Bytes Written to the File: " << bytes_written << std::endl;
        msg_received_map.erase(i);
        //find stack
        fwut_count++;
    }
    fwut = fwut + fwut_count;
    assert(fwut == agreed_aru);
}


//initialize file pointer
void Processor::open_file() {
    fp = fopen("machine_index.txt", "w");
    if (fp == NULL) {
        std::cerr << "Error: file failed to open" << std::endl;
        exit(1);
    }
}
//close file
void Processor::close_file() {
    fclose(fp);
}

int Processor::find_max_messages() {
    //round_balance = GLOBAL_MAXIMUM - token.fcc
    //local_balance = min(LOCAL_MAXMUM, round_balance)
    //The local_balance will be the maximum number of packets that a process can send.
    int round_balance = GLOBAL_MAX - token_buf->fcc;
    int local_balance = std::min(LOCAL_MAX, round_balance);
    return local_balance;
}


void Processor::ring_request_multicast(){
    //check if token recieved
    count = ( count + 1 ) % RING_MCAST_FREQ;

    if(count != 0) return;

    if((!had_token && machine_id != 1) || (machine_id == 1)) {
        // multicast in order let previous neighbor know your address in order to form the ring
        update_msg_buf(MSG_TYPE::REQUEST_RING);
        std::cout << "Ring:             my ip sent "<< my_ip_ << std::endl;
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
    std::cout << "Sending:        machine " << machine_id << " sent token with round number " << token_buf->round << " to " << inet_ntoa(next_addr.sin_addr) << std::endl;
    /*std::cout << "Sent Token Info" << std::endl;
    for (int i = 0; i < DATA_SIZE; i++) {
        std::cout << msg_buf->payload[i] << std::endl;
    }*/
    if(bytes_sent == -1) {
        std::cerr << "Unicast Message Error." << std::endl;
        exit(1);
    }else if(bytes_sent < sizeof (Message)) {
        std::cerr << "Unicast Message Error. Bytes Sent:" << bytes << std::endl;
        return false;
    }
    has_token = false;
    last_token_round = token_buf->round;
    //last_token_aru = token_buf->aru;
    reset_token_timer();
    return true;
}

void Processor::update_msg_buf(MSG_TYPE type) { //when broadcasting new messages
    memset(msg_buf, 0, sizeof(Message));
    msg_buf->type = type;
    msg_buf->machine_id = machine_id;
    if(type == MSG_TYPE::DATA) {
        memset(msg_buf->payload, 0, sizeof(msg_buf->payload)); //TODO: add payload
        msg_buf->random_num = std::rand() % 1000000 + 1;
        msg_buf->pkt_idx = pkt_idx;
        msg_buf->seq = seq;
    }
    if(type == MSG_TYPE::REQUEST_RING) {
        memcpy(msg_buf->payload, my_ip_, strlen(my_ip_)); //send my_ip
        msg_buf->payload[strlen(my_ip)] = 0; // null char
    }
    if(type == MSG_TYPE::TOKEN) {
        memcpy(msg_buf->payload, token_buf, sizeof(Token));
    }
}

void Processor::update_token_buf(int s, int a, int last_aru_setter, std::set<int>& new_rtr, int round, int fcc){
    memset(token_buf, 0 , sizeof(Message));
    token_buf->seq = s;
    token_buf->fcc = fcc;
    token_buf->rtr_size = new_rtr.size();
    token_buf->last_aru_setter = last_aru_setter;
    token_buf->aru = a;
    token_buf->round = round;
    int c = 0;
    for(auto itr = new_rtr.begin(); itr != new_rtr.end() && c < MAX_RTR; ++itr){
        token_buf->rtr[count] = *itr;
        c++;
    }
    if(c >= MAX_RTR) std::cerr << "Request overflow!" << std::endl;
}

void Processor::reset_token_timer(){
    std::cout << "Timer:            set for token with round number " << last_token_round << std::endl;
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
//        if (timestamp.tv_usec - last_token_sent_time.tv_usec >= TOEKN_TIMEOUT_GAP_IN_USEC){
            /* resend token */
            update_msg_buf(MSG_TYPE::TOKEN);
            send_token_to_next();
            std::cout << "Timer:            Timeout! Token resend to machine "<< next_id <<" at " <<  inet_ntoa(next_addr.sin_addr) << std::endl;
            gettimeofday(&last_token_sent_time,NULL);
        }
    }
}

bool Processor::form_ring() {
    switch (recv_buf->type) {
        case MSG_TYPE::TOKEN:
            memcpy(recv_buf->payload, token_buf, sizeof(Token));
            if(token_buf->round == 1) {
                std::cout << "Ring:             Ring is formed!" << std::endl;
                return true;
            }
            std::cout << "Received:       machine " << machine_id << " received token with round number " << token_buf->round << "." << std::endl;
            if(token_buf->round == last_token_round) {
                if(machine_id == 1) {
                    std::cout << "Ring:              Ring is formed!" << std::endl;
                    return true;
                } else {
                    std::cout << "Token:           Already sent token round number"<< last_token_round << std::endl;
                    break;
                }
            }

            if(has_next && !had_token) {
                update_msg_buf(MSG_TYPE::TOKEN);
                send_token_to_next();
                had_token = true;
                //send token to next
            } else {
                std::cout << "Token:           But machine " << machine_id << " does not have next address" << std::endl;
            }
            if(!has_next && !had_token){
                has_token = true;
                had_token = true;
            }
            break;
        case MSG_TYPE::REQUEST_RING:
            if (next_id != recv_buf->machine_id) break;
            if (!has_next ) {
                std::cout << "Ring:             From machine_id : " << recv_buf->machine_id << std::endl;
                char next_ip[strlen((const char *) recv_buf->payload)];
                memcpy(next_ip, recv_buf->payload, strlen((char *) recv_buf->payload));
                next_addr.sin_family = AF_INET;
                next_addr.sin_addr.s_addr = inet_addr(next_ip);// htonl(addr_binary);  /* ucast address */
                next_addr.sin_port = htons(PORT);
                std::cout << "Set next: " << next_ip << std::endl;
                has_next = true;
            }
            if(machine_id == 1 && has_next && !had_token) {
                std::cout << "Sending:       machine 1 is sending token" << std::endl;
                update_token_buf(0, 0, 0, rtr, 0, 0);
                update_msg_buf(MSG_TYPE::TOKEN);
                send_token_to_next();
                had_token = true;
            }else if (has_token && has_next) {
                std::cout << "Sending:       machine " << machine_id << " is sending token" << std::endl;
                update_msg_buf(MSG_TYPE::TOKEN);
                send_token_to_next();
            }
            break;
        case MSG_TYPE::DATA:
            std::cout << "Received:      machine " << machine_id << " received data message with from machine " << recv_buf->machine_id << "." << std::endl;
            std::cout << "Ring:             Ring is formed!" << std::endl;
            return true;
            break;
        default:
            break;
    }
    return false;
}



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
//    int set_resue = 1;
//    if (setsockopt(srm, SOL_SOCKET, SO_REUSEADDR, &set_resue, sizeof(int)) < 0){ //SO_REUSEPORT
//        perror("Mcast: set reuse failed");
//        exit(1);
//    }

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
//    FD_SET( (long)0, &mask );    /* stdin */
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

void Processor::close_sockets() {
    close(srm);
    close(ssm);
    close(ssu);
}