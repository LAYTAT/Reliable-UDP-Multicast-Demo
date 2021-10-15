#include "mcast_include.h"
#include "Processor.h"

int main(int argc, char * argv[])
{

    std::stringstream s1(argv[1]);
    std::stringstream s2(argv[2]);
    std::stringstream s3(argv[3]);
    std::stringstream s4(argv[4]);
    int machine_index = 0;
    int loss_rate = 0;
    int num_of_packets;
    int number_of_machines;
    s1 >> num_of_packets;
    s2 >> machine_index;
    s3 >> number_of_machines;
    s4 >> loss_rate;


    Processor* p = new Processor(machine_index, loss_rate, num_of_packets, number_of_machines);


    p->socket_init();
//    p->start_chat();


    //p->open_file();
    p->start_mcast(); // main loop
    p->close_file();


    p->close_sockets();
    p->deleteMap(p->msg_received_map);

    delete p;
    return 0;
}
