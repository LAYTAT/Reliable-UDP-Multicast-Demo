#include "Processor.h"
bool Processor::start(){

    // socket and bind
    if(!socket_init()) return false;

    if(!send_in_queue()) return false;

    return false;
}

bool Processor::socket_init(){
    return false;
}

bool Processor::send_in_queue(){
    return false;
}