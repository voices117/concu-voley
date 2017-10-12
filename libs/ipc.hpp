/**
 * Common IPC utilities.
 */

#ifndef IPC_HPP
#define IPC_HPP

/* include area */
#include <errno.h>
#include <iostream>
#include <string>
#include <string.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>


using std::size_t;


namespace IPC{
    
    /** IPC exceptions */
    class Error : public std::exception {
    public:
        Error( const std::string& message ) : message(message) {}
        virtual ~Error() {};

        const char* what() const throw() { return message.c_str(); }

    private:
        std::string message;
    };

    /**
     * Key class to allocate resources.
     */
    class Key {
    public:
        Key( const char *s ) : Key(static_cast<std::string>( s ), 'a') {}
        Key( const std::string& s ) : Key(s, 'a') {}
        Key( const std::string& s, char id ) : filename(s), id(id) {
            key_t key = ftok( filename.c_str(), id );
            if( key < 0 ) {
                throw IPC::Error( static_cast<std::string>( "ftok: " ) + strerror( errno ) + " " + s + std::to_string( id ) );
            }
            this->value = key;
        }
        ~Key() { this->value = -1; }

        friend std::ostream& operator<<( std::ostream& os, Key key ) {
            return os << "{ fname=" << key.filename << ", id=" << key.id << " ket_t=" << key.value << " }";
        }
        
        std::string filename;
        char id;
        key_t value{-1};
    };

    /**
     * Creates objects that hold and destroy IPC resources of a certain class.
     * The class must have static methods \c Destroy and \c Create.
     * The resource is released when the object is destroyed **in the process that created it**
     * (so forked childs do not release it accidentally).
     * This way, a resource can be released using RAII to avoid unexpected leaks.
     */
    template <typename T, typename K=IPC::Key> class Resource {
    public:
        Resource( K key );
        Resource( K key, size_t n );
        Resource( K key, size_t n, size_t m );
        Resource( Resource<T, K>&& other );
        ~Resource();

        Resource operator=( const Resource &other ) = delete;

    private:
        /* the filename of the IPC resource */
        K key;

        /* the PID of the processes that owns this resource */
        pid_t creator_pid{-1};
    };
}


/**
 * Constructor implementation.
 */
template <typename T, typename K> IPC::Resource<T, K>::Resource( K key ) : key(key) {
    this->creator_pid = getpid();
    T::Create( this->key );
}

template <typename T, typename K> IPC::Resource<T, K>::Resource( K key, size_t n ) : key(key) {
    this->creator_pid = getpid();
    T::Create( this->key, n );
}

template <typename T, typename K> IPC::Resource<T, K>::Resource( K key, size_t n, size_t m ) : key(key) {
    this->creator_pid = getpid();
    T::Create( this->key, n, m );
}

template <typename T, typename K> IPC::Resource<T, K>::Resource( Resource<T, K>&& other ) : key(other.key) {
    std::swap( this->creator_pid, other.creator_pid );
}

/**
 * Destructor implementation.
 */
template <typename T, typename K> IPC::Resource<T, K>::~Resource() {
    /* destroys the object only if the destructor was called from the creator process
     * this prevents forked childs from destroying resources that they don't own */
    if( this->creator_pid > 0 && this->creator_pid == getpid() )
        T::Destroy( this->key );
}


#endif
