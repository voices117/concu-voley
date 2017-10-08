/* include area */
#include "process.hpp"
#include "log.hpp"
#include <string.h>
#include <sys/wait.h>


/**
 * Constructor implementation.
 */
IPC::Process::Process( std::function<void()> callable ) {
    this->ppid = getpid();
    this->pid = fork();
    if( this->pid < 0 ) {
        throw IPC::Process::Error( "fork error: " + static_cast<std::string>( strerror( errno ) ) );
    } else if( this->pid == 0 ) {
        callable();
        throw IPC::Process::Exit();
    }
}

/**
 * Destructor implementation.
 */
IPC::Process::~Process() {
    /* checks that the parent process is the one calling the destructor */
    if( this->pid == 0 || this->ppid != getpid() )
    return;
    
    LOG_DBG << "waiting child " << this->pid << std::endl;
    // TODO: inform status
    int status;
    waitpid( this->pid, &status, 0 );
}
