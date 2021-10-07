//
// Created by lay on 9/30/2021.
//
#include "mcast_include.h"

#define SERVERPORT 4950 // the port users will be connecting to

int main(int argc, char *argv[])
{
    // test for queue usage
    std::queue<int> queue;
    for (int i = 0 ; i < 10; ++i) {
        queue.push(i);
    }
    queue.pop();
    std::cout << " after one pop, queue size = " << queue.size() << " queue constains = [ ";
    for (int i = 0 ; !queue.empty() ; ++i) {
        queue.pop();
        std::cout << queue.front() << ", ";
    }

    int sockfd;
    struct sockaddr_in their_addr; // connector's address information
    struct hostent *he;
    int numbytes;
    int broadcast = 1;
    //char broadcast = '1'; // if that doesn't work, try this

    if (argc != 3) {
        fprintf(stderr,"usage: broadcaster hostname message\n");
        exit(1);
    }

    if ((he=gethostbyname(argv[1])) == NULL) {  // get the host info
        perror("gethostbyname");
        exit(1);
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    // this call is what allows broadcast packets to be sent:
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast,
                   sizeof broadcast) == -1) {
        perror("setsockopt (SO_BROADCAST)");
        exit(1);
    }

    their_addr.sin_family = AF_INET;     // host byte order
    their_addr.sin_port = htons(SERVERPORT); // short, network byte order
    their_addr.sin_addr = *((struct in_addr *)he->h_addr);
    memset(their_addr.sin_zero, '\0', sizeof their_addr.sin_zero);

    if ((numbytes=sendto(sockfd, argv[2], strlen(argv[2]), 0,
                         (struct sockaddr *)&their_addr, sizeof their_addr)) == -1) {
        perror("sendto");
        exit(1);
    }

    printf("sent %d bytes to %s\n", numbytes,
           inet_ntoa(their_addr.sin_addr));

    close(sockfd);

    return 0;
}
