#include "mcast_include.h"
#include "Processor.h"

int main()
{
    int machine_id = 0;
    int loss_rate = 0;
    int nums_packets = NUM_OF_TOTAL_PACKETS;
    Processor* p = new Processor(machine_id, loss_rate, nums_packets);
    p->socket_init();
    p->start_chat();
    delete p;
    return 0;
}
