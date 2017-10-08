#ifndef BARRIER_HPP
#define BARRIER_HPP


/* include area */
#include "ipc.hpp"
#include <string>
#include <sys/ipc.h>
#include <sys/sem.h>

using std::size_t;


namespace IPC {

    /**
     * Creates a barrier that blocks a process until N other processes arrive at it.
     */
    class Barrier {

    public:
        /**
         * Class used for Barrier exceptions.
         */
        class Error : public IPC::Error {
            public:
                Error( const std::string& message ) : IPC::Error( message ) {}
                ~Error() {}
        };

        /** Static methods used to create and destroy the IPC resources */
        static void Create( const std::string& s );
        static void Destroy( const std::string& s );
        
        /** Creates a barrier object an intializes the barrier */
        explicit Barrier( const std::string& filename, size_t n );
        /** Creates a barrier object (already initialized by another process) */
        explicit Barrier( const std::string& filename );

        Barrier( const Barrier& other );
        Barrier( Barrier&& other );
        Barrier& operator=( Barrier&& other );
        ~Barrier();

        /** Waits for the barrier to hold the expected number of processes. */
        void wait();
        /** Signals that the process reached the barrier. */
        void signal();

    private:
        /** Number of processes to barrier */
        size_t n{ 0 };
        /** The internal semaphore ID */
        int semid{ -1 };

    };
}

#endif
