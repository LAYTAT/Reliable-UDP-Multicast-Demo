//
// Created by 59108 on 10/11/2021.
//
#include "mcast_include.h"

int main(){
    int ssm;                          //sending & receiving socket fd for multicast
    struct sockaddr_in serv_addr;         // storing own addr, use for binding
    struct sockaddr_in name;
    struct sockaddr_in send_addr;
    struct ip_mreq mreq;
    unsigned char ttl_val;
    int mcast_addr;

    mcast_addr = 225 << 24 | 0 << 16 | 1 << 8 | 1; /* (225.0.1.1) mcast IP group*/
    mreq.imr_multiaddr.s_addr = htonl( mcast_addr );

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


    Message * msg = new Message();
    msg->type = -1; // mcast_start
    std::cout << "before sendto" << std::endl;
    sendto(ssm, msg, sizeof(Message), 0,(struct sockaddr *) &send_addr, sizeof(send_addr));

    return true;
}