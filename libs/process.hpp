/**
 * Wraps fork to make it easier and safer to use.
 */

#ifndef PROCESS_HPP
#define PROCESS_HPP

/* include area */
#include "ipc.hpp"
#include <functional>
#include <string>
#include <sys/types.h>
#include <unistd.h>


namespace IPC {
    
    /**
     * A wrapper around fork. When the child process finishes, throws an exception
     * to interrupt the flow of the code (simulating an immediate exit).
     * The object's destructor waits for the child process to finish (only in the parent).
     */
    class Process {
        
    public:
        /**
         * Signals that a child process has finished.
         */
        class Exit : public std::exception {};
        
        /**
         * Signals that the child process has finished unexpectedly.
         * 
         * \param message The error message.
         */
        class Error : public IPC::Error {
        public:
            Error( const std::string& message ) : IPC::Error( message ) {}
            ~Error() {}
        };

        
        Process() {}

        /** The callable received as parameter is executed in another process (the child).
         *  After it finishes, throws an ExitProcess exception. 
         *  The parent process continues normally.*/
        Process( std::function<void()> callable );

        /** On the parent, waits for the child to finish.
         *  On the child, does nothing. */
        ~Process();

        /* move constructor */
        Process( const Process& other ) = delete;
        Process( Process&& other ) : pid{other.pid}, ppid{other.ppid} {
            other.pid = 0;
            other.ppid = -1;
        }
        Process& operator=( Process&& other ) {
            std::swap( this->ppid, other.ppid );
            std::swap( this->pid, other.pid );
            return *this;
        }

        /* query */
        pid_t get_pid() { return this->pid; }
        void set_group( pid_t pgid );

    private:
        /* the value returned by fork. */
        pid_t pid{0};
        /* the value of the parent process. */
        pid_t ppid{-1};

    };

    
    /**
     * Wrapper to fork + execve similar to Process.
     */
    class ExecProcess {
    public:
        
    };
}

#endif
