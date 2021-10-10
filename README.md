# Distributed System Exercise II
###### Junjie Lei
###### Chanha Kim

## General Approach
Using the unreliable UDP/IP protocol, our design aims to multicast packets between N machines that reside on the same network segment, so that one multicast message may get to all of them according to the agreed order. This will be implemented using a token-based algorithm where the token is circulated among the processes where the token-holder has the privilege to broadcast packets. In other words, the Single Ring Protocol will be implemented.

## Message Formats 
Broadcast messages and token messages will be used. Each carries different information with different algorithms associated with it.

## Broadcast Message
Broadcast Messages contain information about application content
packet_index (the sequence number of the packet): corresponds to a position in the ordering
machine_index: this will be the id of the process that sent this message. This id will also be used to know the position of the process in the ring.
payload: 1400 byte content of the message
round number (?):
random number (?):

## Token Message Fields
seq: the sequence number of the last message sent
aru: determines when the broadcast messages have been received by all processes (acts as acknowledgment)
rtr: list of sequence numbers of the packets that need to be retransmitted
fcc: flow control (?)

## Single Ring Protocol

Basically, a token gets passed around in a ring of machines, whereupon receiving a token, that machine multicasts while it holds the token, updates the token message accordingly, and passes the token to the next machine in a ring.

One Round of Token Handling
Retransmission Stage
This stage only take place if (token.aru < local.aru)
This is where the broadcaster tries to resend the requested packets, and after this preprocessing, the broadcaster will resend all the packets other than the unsent ones.

Even though the current broadcaster might not be able to send to all the requested packets(for that it might not have some of the requested packets itself), it will still send all the requested messages in the range of [0, local.aru]. The retransmission packets will not be sent immediately, it will be added to the queue.

## Token Update
### Seq
Seq number indicates the newest message sent by the broadcaster, which can be smaller than the previous token.seq if the broadcaster is resending packets. So when the broadcaster sends new packets, it will be updated to keep up.
### Aru
#### If (token.aru == token.seq)
then the broadcaster will send new data packets, and update the token.aru and the token. seq both to the newest sent packets to be sent. Packets will be put into a queue, and the actual sending will happen later.
when does the broadcast take place? before or after the token unicast? We at the moment do not know how will this affect the overall protocol, we are currently doing a broadcast before sending the token to the next side of the ring.
#### If (token.aru < token.seq && token.aru < local.aru)
##### If(token.aru.who_lower_it_last_time == current.machine_id)
So it is the one that lowered the aru, and the token.aru is still the same, should set token.aru to its local aru.
##### else
Enter the Retransmission Stage and the token.aru stays unchanged.

#### If(token.aru > local.aru)
Lower the token.aru to the local aru. And update the token.to with the lost packets’ seq_nums.
#### fcc
This should be used to control the bound of data flow. fcc itself is a count of the number of packets broadcast by all processors during the previous rotation of the token. It stays unchanged if no packets are to be sent, and adds with the new packets if there are packets to be sent.

### Flush Stage
Send all the data in the queue
This is sending within the limit defined by fcc and a constant GLOBAL_MAXIMUM:  token.fcc - GLOBAL_MAXIMUM
Unicast token
Send the token to the next one in the ring circle.

## Ring Initialization
Start each mcast process with each respective index(which is in the range of [1,10]),  and specify the total number of processes there are going to be for this ring.
Each mcast process will bind with itself a local address and a port.
They will start sending an initial packet immediately when running.
When received an init packet, it will take the machine_id and the address from it, and check to see if it is the one that’s next in the ring. If it is, it will store the next neighbor in the ring as a future unicast destination when passing the token.
When the start_mcast process tries to start the sending by sending a ring_check packet, all the processes that have not got their next neighbor in the ring’s address will respond with a not_ready packet, and other ones will send the ready packet.
When the ring is formed(start_mcast gets all the ready packets from all of the mcast processes), start_mcast will send a ring_start packet, then the machine with the machine_id of 1 will initialize a token, multicast data packets and pass on the token to the next neighbor on the ring.

## Ring Ending
Each participant has a vector for knowing the newest seq number for each machine when it sees that all the seq numbers for each machine reach the <num_of_packet>. It will flush the data in the queue into the file and start sending a done_packet, it will constantly send this done_packet util that everyone elses’ done_packet is received by itself.