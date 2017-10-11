#ifndef QUEUE_HPP
#define QUEUE_HPP

/* include area */
#include "ipc.hpp"
#include "log.hpp"
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


namespace IPC {    

    /**
     * Signals errors when handling a Queue object.
     */
    struct QueueError : public IPC::Error {
        QueueError( const std::string& message ) : IPC::Error( message ) {}
        ~QueueError() {}
    };
    
    /**
     * Exception thrown when the end of file was reached.
     */
    struct QueueEOF : public IPC::Error {
        QueueEOF() : IPC::Error( "" ) {}
    };

    /**
     * Used to specify the mode to open a Queue.
     */
    enum class QueueMode {
        read = O_RDONLY,
        write = O_WRONLY,
    };

    /**
     * An inter-process queue for a specific data type T.
     */
    template<class T> class Queue {
        
    public:
        
        /** Creates/destroys a Queue that will be accessible from other modules. */
        static void Create( const std::string& filename );
        static void Destroy( const std::string& filename );
        
        Queue( const std::string& filename, QueueMode mode );
        Queue( const std::string& filename, QueueMode mode, bool uninterrupted );
        Queue& operator=( const Queue& other );
        ~Queue();
        
        void insert( T elem );
        T remove();
        
    private:
        std::string filename;
        int fd { -1 };
        bool uninterrupted;
    };
    
}

/**
 * Implementation
 */


/**
 * Creates the Queue so it's available to the other processes.
 */
template <class T> void IPC::Queue<T>::Create( const std::string& filename ) {
    if( mknod( static_cast<const char*>( filename.c_str() ), S_IFIFO | 0644, 0 ) )
        throw IPC::QueueError( strerror( errno ) );
}

/**
 * Destroys the Queue (no other processes will be able to connect again).
 */
template <class T> void IPC::Queue<T>::Destroy( const std::string& filename ) {
    unlink( static_cast<const char*>( filename.c_str() ) );
    LOG_DBG << "destroy" << std::endl;
}


/**
 * Queue constructor implementation.
 */
template <class T> IPC::Queue<T>::Queue( const std::string& filename, IPC::QueueMode mode ) : Queue(filename, mode, false) {
}

template <typename T> IPC::Queue<T>::Queue( const std::string& filename, IPC::QueueMode mode, bool uninterrupted ) : filename(filename) {
    this->fd = open( this->filename.c_str(), static_cast<int>( mode ) );
    if( this->fd == -1 )
        throw IPC::QueueError( strerror( errno ) );
    LOG_DBG << "Queue " << filename << " opened in mode " << ( mode == IPC::QueueMode::read ? "read" : "write" ) << std::endl;
    this->uninterrupted = uninterrupted;
}

/**
 * Queue destructor implementation.
 */
template <class T> IPC::Queue<T>::~Queue() {
    if( this->fd != -1 )
        close( this->fd );
    this->fd = -1;
}


/**
 * Inserts an element into the Queue.
 *
 * \param elem Element to insert.
 */
template <class T> void IPC::Queue<T>::insert( T elem ) {
    do {
        if( write( this->fd, &elem, sizeof( T ) ) == -1 ) {
            if( errno == EINTR && this->uninterrupted ) {
                /* retries */
                continue;
            }
            throw IPC::QueueError( strerror( errno ) );
        }
    } while( errno == EINTR );
}

/**
 * Gets an element from the Queue.
 *
 * \param elem Element to insert.
 */
template <class T> T IPC::Queue<T>::remove() {
    T rv;

    do {
        int bytes_read = read( this->fd, &rv, sizeof( T ) );
        if( bytes_read == 0 ) {
            throw IPC::QueueEOF();
        }
    
        if( bytes_read != sizeof( T ) ) {
            if( errno == EINTR && this->uninterrupted ) {
                /* retries */
                continue;
            }
            throw IPC::QueueError( strerror( errno ) );
        }
    } while( errno == EINTR );

    /* TODO: use move semantics */
    return rv;
}


#endif
