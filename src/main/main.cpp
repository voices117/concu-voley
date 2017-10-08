#include "queue.hpp"
#include "log.hpp"
#include "mutex.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <sys/wait.h>


#define QUEUE_FILE "/tmp/algo"


int main( int argc, const char *argv[] )
{
    LOG << "Init" << std::endl;

    /* allocates resources */
    IPC::Queue<int>::Create( QUEUE_FILE );

    /* destroys resources */
    IPC::Queue<int>::Destroy( QUEUE_FILE );
}
