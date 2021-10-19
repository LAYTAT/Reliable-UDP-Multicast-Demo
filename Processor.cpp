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

long long diff_ms(timeval t1, timeval t2)
{
    struct timeval diff;
    timersub(&t1, &t2, &diff);
    return (diff.tv_sec * 1000 + diff.tv_usec / 1000);
}

long long diff_us(timeval t1, timeval t2)
{
    struct timeval diff;
    timersub(&t1, &t2, &diff);
    return (diff.tv_sec * 1000000 + diff.tv_usec);
}

Performance Processor::start_mcast(){
    // get my address for later sending
    open_file();
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
    struct timeval started_timestamp;

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
                    gettimeofday(&started_timestamp, nullptr);
                    continue;
                }

                bytes = recv_dbg(srm, (char*)recv_buf, sizeof(Message), 0);
                if (bytes == -1) {
                    std::cerr << "Received Message Err" << std::endl;
                } else if (bytes > 0 && bytes < sizeof(Message)) {
                    std::cerr << "Received Message Corrupted. Bytes Received:" << bytes << std::endl;
                } else if(bytes == 0) {
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
            }
        }
        if(mcast_received && !ring_formed ){
            ring_request_multicast();           //keep multicast until token received
        }
        check_timeout(); //timeout for token
    }

    struct timeval ended_timestamp;
    gettimeofday(&ended_timestamp, nullptr);
    Performance ret;
    std::cout << "Total Retransmission :    " << total_rtr_count << " times." << std::endl;
    std::cout << "Retransmission rate:         " << total_rtr_count * 100 / aru << "%" << std::endl;
    ret.msec = diff_ms(ended_timestamp, started_timestamp);
    ret.total_packet = aru;
    ret.pakcet_size_in_bytes = sizeof(Message);
    return ret;
}

void Processor::store_to_input() {
    msg_received_map.insert(std::make_pair(recv_buf->seq, make_Message(MSG_TYPE::DATA, recv_buf->seq, recv_buf->pkt_idx, recv_buf->machine_id, recv_buf->random_num)));
}

void Processor::update_rtr_with_token_seq() {
    //update rtr by checking received_token_buf->seq
    for(int i = aru + 1; i <= received_token_buf->seq; ++i) {
        if(input_set.count(i)==0) {
//            assert(i!=0);
            rtr.insert(i);
        }
    }
}

//update_rtr_aru_with_msg wanted action
// recieved 1, 2, 4, //3 will go to the rtr, aru = 2, 1,2,4 is in the input buffer
// recieved 5, 6, 8 // aru = 2, rtr = 3,7, and 1,2,4,5,6,8 in input buffer
// recieved 3, 9 //aru = 6, rtr =,,,,//update_rtr_aru_with_msg wanted action
//            // recieved 1, 2, 4, //3 will go to the rtr, aru = 2, 1,2,4 is in the input buffer
//            // recieved 5, 6, 8 // aru = 2, rtr = 3,4,7, and 1,2,4,5,6,8 in input buffer
//            // recieved 3, 9 //aru = 6, rtr =,,,,
//            // sort buffer, aru = last continous integer in the buffer, rtr = from aru (4) to input_buf last element (10)...
// sort buffer, aru = last continous integer in the buffer, rtr = from aru (4) to input_buf last element (10)...
void Processor::update_rtr_aru_with_msg(int msg_seq) {
    //update aru if this one connected
//    assert(msg_seq != 0);
    input_set.insert(msg_seq);
    rtr.erase(msg_seq);
    for(auto itr = input_set.find(msg_seq); itr != input_set.end(); itr++){
        if(*itr == aru + 1) {
            aru++;
        } else break;
    }
    // update rtr
    for(int i = aru + 1; i < msg_seq; ++i) {
        if(input_set.count(i)==0) {
//            assert(i!=0);
            rtr.insert(i);
        }
    }
}

void Processor::update_rtr_aru_with_new_broadcast(int brdcst_msg_seq) {
    //update aru if this one connected
    input_set.insert(brdcst_msg_seq);
    for(auto itr = input_set.find(brdcst_msg_seq); itr != input_set.end(); itr++){
        if(*itr == aru + 1) {
            aru++;
//            assert(aru <= seq);
        } else break;
    }
    // update rtr
    for(int i = aru + 1; i < brdcst_msg_seq; ++i) {
        if(input_set.count(i)==0) {
//            assert(i!=0);
            rtr.insert(i);
        }
    }
}

bool Processor::data_tranfer(){


    switch (recv_buf->type) {
        case MSG_TYPE::DATA: {
            //get message means someone already have the token
            reset_token_timer();
            //we recieved a multicast data
            //temp seq is the message seq
            int temp_seq = 0;
            temp_seq = recv_buf->seq ;

            //ignore data you already have
            if (temp_seq <= aru) {
                break;
            }


            //push new message into the received_map
            store_to_input();

            update_rtr_aru_with_msg(temp_seq);
            write_to_file();
            break;
        }
        case MSG_TYPE::TOKEN: {
            received_token_buf = (Token*)(recv_buf->payload);
            if(machine_id == 1) {
                if(received_token_buf -> round != last_token_round) break;
            } else if(received_token_buf -> round <= last_token_round) {
                break;
            }

            int token_aru_received = received_token_buf->aru;

            if(check_if_everybody_ready_to_exit()) {
                return true;
            }

            //cancel timer before send token
            cancel_token_timer();

            /*
             * Updating data structures
             */
            flush_input_buf(); // writes to a file, updates msg_received, input empty
            //std::cout << "input flush success!" <<std::endl;

            //find max number of messages that can be sent by this processor
            int m = find_max_messages();

            //update retransmission request by looking at token seq
            update_rtr_with_token_seq();

            //r is the number of retranmission happened
            int r = retransmission(m);
            total_rtr_count += r;
//            std::cout << "Retransmission success! number of retransmission was: " << r << std::endl;
            //subtract number of retransmissions from m, call it m2
            int m2 = m - r;
            int b = 0;

            if (received_token_buf->seq == received_token_buf->aru) {//TODO: new message sending problem
                //updates token_aru, local_aru, and token_seq as broadcasting messages
                b = broadcasting_new_messages(m2);
                received_token_buf->last_aru_setter = 0;
            }

            //update token parameters
            if (aru < received_token_buf->aru || machine_id == received_token_buf->last_aru_setter || received_token_buf->last_aru_setter == 0) {
                received_token_buf->aru = aru;
//                assert(received_token_buf->seq >= received_token_buf->aru);
                if (received_token_buf->aru == received_token_buf->seq) {
                    received_token_buf->last_aru_setter = 0;
                } else {
                    received_token_buf->last_aru_setter = machine_id;
                }
            }
            //token_aru updated, token_last setter updated, token seq updated, update token rtr now before sending a token
            //iterate through set rtr and put it into received_token_buf rtr
            //update_sending_token_buf(int seq, int aru, int last_aru_setter, std::set<int>& new_rtr, int round, int fcc){
            int token_seq = received_token_buf->seq; int token_aru = received_token_buf->aru; int last_aru_setter = received_token_buf->last_aru_setter;
            int round = received_token_buf->round;
            int fcc = received_token_buf->fcc;
            if (machine_id == 1) { //handles the machine id = 1, round update and fcc update
                round = received_token_buf->round + 1;
                auto this_round_time = last_round_time;
                gettimeofday(&last_round_time, nullptr);
                fcc = 0;
            }
            fcc = fcc + r + b;
            last_token_aru = token_aru_received;

            //update token_bu
            int rtr_size = rtr.size();
            if (rtr_size >= MAX_RTR) {
                rtr_size = MAX_RTR;
            }

            update_sending_token_buf(token_seq, token_aru, last_aru_setter, rtr_size, rtr, round, fcc);
            update_msg_buf(MSG_TYPE::TOKEN);
            send_token_to_next(); //this will reset timer
            break;
        }
        case MSG_TYPE::EXIT: {
            return true;
        }
        default:
            break;
    }

    return false;
}

void Processor::broadcast_exit_messages() {
    for (int i = 0; i < BROADCASTING_TIMES; i++) {
        update_msg_buf(MSG_TYPE::EXIT);
        send_to_everyone();
    }
}


//broadcasting new messages
//think about pkt_index, seq_num, recieved_msg,
int Processor::broadcasting_new_messages(int m2) {

    int b =0;
    for (int i = 0; i < m2; i++) {
        if (pkt_idx == nums_packets) {
            break;
        }
        received_token_buf->seq++; pkt_idx++;
        seq = received_token_buf->seq;
        update_msg_buf(MSG_TYPE::DATA);
        msg_received_map.insert(std::make_pair(msg_buf->seq,
                                               make_Message(msg_buf->type, msg_buf->seq, msg_buf->pkt_idx, msg_buf->machine_id, msg_buf->random_num)));
        update_rtr_aru_with_new_broadcast(received_token_buf->seq);
        write_to_file();
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

void Processor::deleteMap(std::unordered_map<int, Message *> map) {
    for (int i = 0; i < map.size(); i++) {
        delete map[i];
    }
}

//update our own rtr first
//input: number of maximum retransmission
//output: returns number of retransmissions happened
//update received_token_buf->with new rtrs
int Processor::retransmission(int m) {
    int count_resend = 0;
    int num_retrans = std::min(m, (int)received_token_buf->rtr_size);

    for (int i = 0; i < num_retrans; i++) {
        if (msg_received_map.count(received_token_buf->rtr[i]) == 0) {
                rtr.insert(received_token_buf->rtr[i]);
            continue;
        }
        sendto(ssm, msg_received_map[received_token_buf->rtr[i]], sizeof(Message), 0, (struct sockaddr *)&send_addr, sizeof(send_addr));
        count_resend++;
    }

    return count_resend;
}

void Processor::flush_input_buf() {
    int agreed_aru = std::min(last_token_aru, received_token_buf->aru);

    for (int i = fwut + 1; i <= agreed_aru; i++) {
        if (agreed_aru == 0) {
            break;
        }
        msg_received_map.erase(i);
        fwut++;
    }
    fwut = agreed_aru;
}

//initialize file pointer
void Processor::open_file() {

    std::string filename = std::to_string(machine_id) + ".txt";

    fp = fopen(filename.c_str(), "w");
    if (fp == NULL) {
        std::cerr << "Error: file failed to open" << std::endl;
        exit(1);
    }
}
//close file
void Processor::close_file() {
    fflush(fp);
    fclose(fp);
}

int Processor::find_max_messages() {
    //round_balance = GLOBAL_MAXIMUM - token.fcc
    //local_balance = min(LOCAL_MAXMUM, round_balance)
    //The local_balance will be the maximum number of packets that a process can send.
    int round_balance = GLOBAL_MAX - received_token_buf->fcc;
    int local_balance = std::min(LOCAL_MAX, round_balance);
    return local_balance;
}


// multicast in order let previous neighbor know your address in order to form the ring
void Processor::ring_request_multicast(){
    //check if token recieved
    count = ( count + 1 ) % RING_MCAST_FREQ_FACTOR;
    if(count != 0) return;

    if((!had_token && machine_id != 1) || (machine_id == 1)) {
        update_msg_buf(MSG_TYPE::REQUEST_RING);
//        std::cout << "Ring:             my ip sent "<< my_ip_ << std::endl;
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
//    assert(has_next);
    long unsigned int bytes_sent = sendto(ssu, msg_buf, sizeof(Message), 0,(struct sockaddr *)&next_addr, sizeof(next_addr) );
    if(bytes_sent == -1) {
        std::cerr << "Unicast Message Error." << std::endl;
        exit(1);
    }else if(bytes_sent < sizeof (Message)) {
        std::cerr << "Unicast Message Error. Bytes Sent:" << bytes << std::endl;
        return false;
    }
    has_token = false;
    last_token_round = sending_token_buf->round;
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
        return;
    }
    if(type == MSG_TYPE::REQUEST_RING) {
        memcpy(msg_buf->payload, my_ip_, strlen(my_ip_)); //send my_ip
        msg_buf->payload[strlen(my_ip)] = 0; // null char
        return;
    }
    if(type == MSG_TYPE::TOKEN) {
//        assert(received_token_buf->aru <= received_token_buf->seq);
        memcpy(msg_buf->payload, sending_token_buf, sizeof(Token));
        return;
    }
    if(type == MSG_TYPE::EXIT) {
        return;
    }
}

void Processor::update_sending_token_buf(int s, int a, int last_aru_setter, int rtr_size, std::set<int>& new_rtr, int round, int fcc){
//    assert(new_rtr.count(0) == 0);
    memset(sending_token_buf, 0 , sizeof(Token));
    sending_token_buf->seq = s;
    sending_token_buf->fcc = fcc;
    sending_token_buf->rtr_size = rtr_size;
    sending_token_buf->last_aru_setter = last_aru_setter;
    sending_token_buf->aru = a;
    sending_token_buf->round = round;
    int c = 0;
    for(auto itr = new_rtr.begin(); itr != new_rtr.end() && c < rtr_size; ++itr){
//        assert(*itr != 0);
        sending_token_buf->rtr[c] = *itr;
        c++;
    }
    if(c >= MAX_RTR) std::cerr << "Request overflow!" << std::endl;
}

void Processor::reset_token_timer(){
//    std::cout << "Timer:            set for token with round number " << last_token_round << std::endl;
    token_flag = true;
    gettimeofday(&last_token_sent_time, nullptr);
}

void Processor::cancel_token_timer(){
    token_flag = false;
}

void Processor::check_timeout(){
    if(token_flag){
        gettimeofday(&timestamp, NULL);
        if ( diff_us(timestamp, last_token_sent_time) >= TOKEN_TIMEOUT_GAP_IN_USECONDS){
            sending_token_buf->round = last_token_round;
            update_msg_buf(MSG_TYPE::TOKEN);
            send_token_to_next();
            gettimeofday(&last_token_sent_time,NULL);
        }
    }
}

bool Processor::form_ring() {
    switch (recv_buf->type) {
        case MSG_TYPE::TOKEN:
            received_token_buf = (Token*)recv_buf->payload;
            if(received_token_buf->round == 1) {
                std::cout << "Ring:             Ring is formed!" << std::endl;
                return true;
            }

            if(received_token_buf->round == last_token_round) {
                if(machine_id == 1) {
                    std::cout << "Ring:              Ring is formed!" << std::endl;
                    return true;
                } else {
                    break;
                }
            }

            if(has_next && !had_token) {
                update_msg_buf(MSG_TYPE::TOKEN);
                send_token_to_next();
                had_token = true;
                //send token to next
            }

            if(!has_next && !had_token){
                has_token = true;
                had_token = true;
            }
            break;
        case MSG_TYPE::REQUEST_RING:
            if (next_id != recv_buf->machine_id) break;
            if (!has_next ) {
//                std::cout << "Ring:             From machine_id : " << recv_buf->machine_id << std::endl;
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
                update_sending_token_buf(0, 0, 0, 0, rtr, 0, 0);
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
//            std::cout << "Received:      machine " << machine_id << " received data message with from machine " << recv_buf->machine_id << "." << std::endl;
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
    //225.1.1.40	10040		Chanha Kim
    //225.1.2.80	10280		Junjie Lei
    mcast_addr = 225 << 24 | 1 << 16 | 2 << 8 | 80; /* (225.1.2.80) mcast IP group*/

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
    FD_SET( srm, &mask );
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

bool Processor::check_if_everybody_ready_to_exit(){
    if(received_token_buf->seq == seq && received_token_buf->aru == received_token_buf->seq) {
        seq_equal_last_seq_and_aru_equal_seq_count ++;
    } else {
        seq_equal_last_seq_and_aru_equal_seq_count = 0;
    }
    if(seq_equal_last_seq_and_aru_equal_seq_count >= ENDING_COUNT) {
        broadcast_exit_messages();
        return true;
    }
    return false;
}

void Processor::write_to_file(){
    //write to file from last local aru to local aru
    for (int i = last_local_aru + 1; i <= aru; i++) {
        fprintf(fp, "%2d, %8d, %8d\n", msg_received_map[i]->machine_id, msg_received_map[i]->pkt_idx, msg_received_map[i]->random_num);
    }
    last_local_aru = aru;
}