#include "net_include.h"

int gethostname(char*,size_t);

int main()
{
    struct sockaddr_in name;
    struct sockaddr_in send_addr;
    struct hostent     h_ent, *p_h_ent;
    char               host_name[80] = {'\0'};
    long               host_num;
    int                ss,sr;
    fd_set             mask;
    fd_set             read_mask,write_mask, excep_mask;
    int                bytes;
    int                num;
    char               mess_buf[MAX_MESS_LEN];
    char               input_buf[80];
    long               on=1;

    sr = socket(AF_INET, SOCK_DGRAM, 0); /* socket for receiving */
    if (sr<0) {
        perror("Bcast: socket");
        exit(1);
    }

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(PORT);

    if ( bind( sr, (struct sockaddr *)&name, sizeof(name) ) < 0 ) {
        perror("Bcast: bind");
        exit(1);
    }

    ss = socket(AF_INET, SOCK_DGRAM, 0);  /* socket for sending */
    if (ss<0) {
        perror("Bcast: socket");
        exit(1);
    }

    if (setsockopt(ss, SOL_SOCKET, SO_BROADCAST, (char *)&on, sizeof(on)) < 0) 
    {
        perror("Bcast: setsockopt error \n");
        exit(1);
    } 

    gethostname(host_name,sizeof(host_name));

    p_h_ent = gethostbyname(host_name);
    if ( p_h_ent == NULL ) {
        printf("Bcast: gethostbyname error.\n");
        exit(1);
    }

    memcpy( &h_ent, p_h_ent, sizeof(h_ent));
    memcpy( &host_num, h_ent.h_addr_list[0], sizeof(host_num) );

    send_addr.sin_family = AF_INET;
    /* bcast address */
    /* send_addr.sin_addr.s_addr = (host_num & htonl(0xffffff00)) | htonl(0xff); */
    send_addr.sin_addr.s_addr = htonl( (128 << 24) | (220 << 16) | (224 << 8) | 127 );
     send_addr.sin_port = htons(PORT);

    FD_ZERO( &mask );
    FD_ZERO( &write_mask );
    FD_ZERO( &excep_mask );
    FD_SET( sr, &mask );
    FD_SET( (long)0, &mask );    /* stdin */
    for(;;)
    {
        read_mask = mask;
        num = select( FD_SETSIZE, &read_mask, &write_mask, &excep_mask, NULL);
        if (num > 0) {
            if ( FD_ISSET( sr, &read_mask) ) {
                bytes = recv( sr, mess_buf, sizeof(mess_buf), 0 );
                mess_buf[bytes] = 0;
                printf( "received : %s\n", mess_buf );
            }else if( FD_ISSET(0, &read_mask) ) {
                bytes = read( 0, input_buf, sizeof(input_buf) );
                input_buf[bytes] = 0;
                printf( "there is an input: %s\n", input_buf );
                sendto( ss, input_buf, strlen(input_buf), 0, 
                       (struct sockaddr *)&send_addr, sizeof(send_addr) );
            }
        }
    }

    return 0;
}
