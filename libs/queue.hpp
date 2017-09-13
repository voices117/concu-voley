#ifndef QUEUE_HPP

/* include area */
#include <errno.h>
#include <fcntl.h>
#include <string>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


struct QueueException : public std::exception {
    std::string message;
    QueueException( std::string message ) : message(message) {}
    ~QueueException() throw () {}
    const char* what() const throw() { return message.c_str(); }
};


/**
 * An inter-process queue for a specific data type T.
 */
template<class T> class Queue {

public:
    /** Creates/destroys a Queue that will be accessible from other modules. */
    static void Create( const std::string& filename );
    static void Destroy( const std::string& filename );

    Queue( const std::string& filename );
    Queue& operator=( const Queue& other );
    ~Queue();

    void insert( T elem );
    T remove();

private:
    std::string filename;
    int fd { -1 };

};


/**
 * Queue constructor implementation.
 */
template <class T> Queue<T>::Queue( const std::string& filename ) : filename(filename) {
    this->fd = open( this->filename.c_str(), O_RDWR );
    if( this->fd == -1 )
        throw QueueException( strerror( errno ) );
}

/**
 * Queue destructor implementation.
 */
template <class T> Queue<T>::~Queue() {
    close( this->fd );
    this->fd = -1;
}


/**
 * Creates the Queue so it's available to the other processes.
 */
template <class T> void Queue<T>::Create( const std::string& filename ) {
    if( mknod( static_cast<const char*>( filename.c_str() ), S_IFIFO | 0666, 0 ) )
        throw QueueException( strerror( errno ) );
}

/**
 * Destroys the Queue (no other processes will be able to connect again).
 */
template <class T> void Queue<T>::Destroy( const std::string& filename ) {
    unlink( static_cast<const char*>( filename.c_str() ) );
}


/**
 * Inserts an element into the Queue.
 *
 * \param elem Element to insert.
 */
template <class T> void Queue<T>::insert( T elem ) {
    if( write( this->fd, &elem, sizeof( T ) ) == -1 )
        throw QueueException( strerror( errno ) );
}

/**
 * Gets an element from the Queue.
 *
 * \param elem Element to insert.
 */
template <class T> T Queue<T>::remove() {
    T rv;
    if( read( this->fd, &rv, sizeof( T ) ) != sizeof( T ) )
        throw QueueException( strerror( errno ) );

    /* TODO: use move semantics */
    return rv;
}


#endif
