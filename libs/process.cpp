/* include area */
#include "process.hpp"
#include "log.hpp"
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

using std::string;
using std::endl;


/**
 * Constructor implementation.
 */
IPC::Process::Process( std::function<void()> callable ) {
    this->ppid = getpid();
    this->pid = fork();
    if( this->pid < 0 ) {
        throw IPC::Process::Error( "fork error: " + static_cast<string>( strerror( errno ) ) );
    } else if( this->pid == 0 ) {
        LOG_DBG << "start process with parent " << this->ppid << endl;
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
    
    LOG_DBG << "waiting child " << this->pid << endl;
    // TODO: inform status
    int status;
    waitpid( this->pid, &status, 0 );
}


/**
 * Sets the process's group ID.
 * 
 * \param pgid Process group ID.
 */
void IPC::Process::set_group( pid_t pgid ) {
    if( setpgid( this->pid, pgid ) < 0 ) {
        throw IPC::Process::Error( static_cast<string>( "setpgid: " ) + strerror( errno ) );
    }
}
