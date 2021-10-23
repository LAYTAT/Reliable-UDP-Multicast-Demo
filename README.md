# Reliable-UDP-Multicast-Demo
## General Approach
Using the unreliable UDP/IP protocol, our design aims to multicast packets between N machines that reside on the same network segment using C++, so that one multicast message may get to all of them according to the agreed order as well as fulfilling safe delivery. This will be implemented using a token-based algorithm where the token is circulated among the processes where the token-holder has the privilege to broadcast packets. In other words, a version of the Single Ring Protocol will be implemented. 

## Single Ring Protocol
A token gets passed around in a ring of machines, whereupon receiving a token, that machine with a token can multicast messages to all other machines. After broadcasting is over as well as updating its relevant variables, it passes the token to the next machine in a ring, and it does the exact same thing as the previous token holder. The sequence number of each message sent is derived from the token_sequence field of the token. Thus, the delivery of messages according to this “global” sequence number guarantees the agreed order delivery. In addition, the single ring protocol guarantees that the minimum of the aru field of the token this round and last round mean the agreed aru of all the machines (i.e. they all have messages with sequence number upto this token aru), satisfying the safe order criteria. Our protocol assumes that the processor failures do not occur, but handles the possibility of token loss from one machine to another. 

## Benchmarks
### for 0 percent loss rate over a 1 GB network.
around **_700 Mbits/sec_**
### for 20 percent loss rate over a 1 GB network.
around **_300 Mbits/sec_**

<img width="602" alt="Screen Shot 2021-10-19 at 2 05 44 AM" src="https://user-images.githubusercontent.com/23161882/137853054-3b9155a5-71c2-432f-9486-366c028e4188.png">


## Data Structure
    Message{ 
    MSG_TYPE type ;   //indicates the type of message         
            int seq;           	//sequence number of the message 
        int pkt_idx;  		//packet index
    int machine_id;	//id of machine that originally sent this message	int random_num;  	//random number generated from 1-1,000,000
            unsigned char payload[1400]; 	//payload
    };
    enum class MSG_TYPE {
    start_mcast = -1; 		//message signals start_mcast stage
    request_ring = 1;		//message signals ring formation stage
    token = -2;			//message signals it’a token
    data = 2;			//message signals its a regular message
    exit = 3;			//message signals exit stage
     };

    Token{
          int token_seq;	//largest sequence number ever sent 
    int token_aru; 	//all-received upto for all the machines if min is taken for this round aru and last round aru
    int last_aru_setter 	//machine-id of the machine that lowered aru
    int rtr_size         	//number of retransmission request
    int rtr[300]		//sequence numbers of “nacks”, i.e. retrans. req.
    int round		//current round number 
    int fcc			//flow control count, i.e. number of messages + retransmissions sent in this round so far	
    }

    Processor{			//important local variables 			
    int my_aru ;		//local all-received-upto      
    set<int>::rtr;		//all the retransmissions needed
    map<int, message *> received_msg_map   //map of received messages and its sequence number 
    bool mcast_recieved  //state of mcast_recieved or not
    bool ring_formed 	  //state of ring formed or not
    bool has_token 	  //state of has token or not
    bool had_token 	 //state of had token or not
    bool has_next 	 //state of has next address in the ring
    bool is_all_data_recieved //state of all data transferred or not
    }

## Ring Initialization

	If the processor in a mcast_recieved state, it enters into ring initialization stage where after ring is formed, ring_formed turns true and the process enters the data transfer process. Since all participating machines know its index (from 1 to N), each machine will try to send multicast and receive multicast in order to find the next machine’s ip address. Our algorithm for ring initialization is as follows:

## Data Transferring Stage
### About Sending Token
Unicast to next machine in the ring formation (same as in the ring initialization stage)
Timeout:
	Each token holder will become a token sender, and after the token is sent,
there should be a timeout for the token with a round number(used set the processor’s local variable timeout_round), and start the timer.
The timeout for round number R only stops when a token that contains round number R+1(For machine[1:9]) or R is received(For machine 0).

### Special TODOs for node 0
Whenever received a token with round number R, update the round number to (R+1);
Reset the fcc to 0 for every circle.

## Flow Control 
GLOBAL_MAXIMUM: constant variable that indicates total number of messages that can be sent in the whole round (including retransmission)
LOCAL_MAXIMUM: constant variable that indicates total number of messages that can be sent by one machine
flow control is as follows:
find max number of messages (local_balance) that can be sent by the processor via 
int round_balance = GLOBAL_MAX - fcc
int local_balance = min(round_balance, LOCAL_MAX)
count number of retransmissions (r) happened subtract it from local_balance. Call this m2.
for m2, broadcast new messages and count the number of broadcasts (b).
update fcc of the token as fcc = fcc + r + b

## One Round of Token Handling For Data Transferring Stage

Upon receiving a MSG_TYPE::token during data_transfer stage:
cancel token timer
special TODOs for node 0
check if everyone is ready to exit, and if yes return true
looking at the agreed_aru = minimum of previous aru and this token aru, get rid of all messages with the message sequences upto agreed_aru from the received_msg_map
union your own rtr with the token_rtr
retransmit as much as you are allowed to (from the flow control)
broadcast as much as you are allowed to (from the flow control)
increment packet index as well as token_seq
put the messages sent into the msg_received_map
update rtr and aru with the new broadcast
broadcast!
update token parameters as follows:
if (local_aru < token_aru or local_id == last_aru_setter or last_aru_setter = 0):
if (token_aru == token_seq):
last aru setter = 0
else:
last aru setter = local_id
send the token to next

## Broadcast Message(Data) Received
If token retransmission timeout is set, check if this broadcast message’s machine-id is the set one, if so, cancel the token retransmission timeout.
ignore the message if you already have this sequence number
store the message into the msg_received_map
update rtr and aru using this message’s sequence number
if this sequence number is in rtr, get rid of this from rtr
all the non-consecutive sequence numbers not received goes to rtr
largest consecutive sequence number is the aru
Write the consecutive data in msg_received into the file

## Token Timeout
When a timeout for a token occurs, retransmit the token
Then set another round of token timeout.

## Termination
### Intuition
When nobody is still requesting any retransmission, that is when it is ok for a processor to exit. But the thing is if you exit, you will break the ring, someone else might be yet to know that there is no more retransmission request anymore. So we might only need one processor to know when it is ok for EVERYONE to leave. Then he let others exit before he exit the ring.

## PS
This is a demo for what is possible with a layer of protocal on top of UDP which enables it to run in a way that is both fast and reliable for multicast which require agreed and safe delivery.

## How to run it
* first make it
* then you run it as follow
```
mcast <num_of_packets> <machine_index> <number of machines> <loss rate>
```
The demo consists of two programs: 
	1. mcast - the tool's process. 
	2. start_mcast - a process signaling the mcast processes to start.
The mcast process should be run as follows:


The <number of machines> indicates how many mcast machines will be run. 
The maximal number of <number of machines> should be a constant and should be 
set to 10. Note that in a particular execution there may be less than or equal 
to 10 <number of machines>.  <loss rate> is the percentage loss rate for this 
execution ([0..20] see below). Note also that machine_index will be in the 
range [1..<number of machines>] and that you can assume exactly 
<number of machines> mcast programs, each with a different index, will be 
ready to run before start_mcast is executed.
	
## Running Result Example
It might look some like this, 
![IMG_2748](https://user-images.githubusercontent.com/23161882/137853336-218075a0-a4d5-4ff4-b464-7ed382b11a5b.PNG)
	

After you running these on 8 machine where 6 of them are sending 160000 packet(each 1412 bytes) to all other 7 processor with a loss rate of 20:
```
mcast 168000 1 8 20
```
