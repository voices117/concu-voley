/**
 * Common IPC utilities.
 */

#ifndef IPC_HPP
#define IPC_HPP

/* include area */
#include <string>
#include <sys/types.h>
#include <unistd.h>


using std::size_t;


namespace IPC{

    /**
     * Creates objects that hold and destroy IPC resources of a certain class.
     * The class must have static methods \c Destroy and \c Create.
     * The resource is released when the object is destroyed **in the process that created it**
     * (so forked childs do not release it accidentally).
     * This way, a resource can be released using RAII to avoid unexpected leaks.
     */
    template <typename T> class Resource {
    public:
        Resource( const std::string& filename );
        Resource( const std::string& filename, size_t n );
        Resource( const std::string& filename, size_t n, size_t m );
        ~Resource();

        Resource operator=( const Resource &other ) = delete;

    private:
        /* the filename of the IPC resource */
        std::string filename;

        /* the PID of the processes that owns this resource */
        pid_t creator_pid{-1};
    };

    /** IPC exceptions */
    class Error : public std::exception {
    public:
        Error( const std::string& message ) : message(message) {}
        virtual ~Error() {};

        const char* what() const throw() { return message.c_str(); }

    private:
        std::string message;
    };
}


/**
 * Constructor implementation.
 */
template <typename T> IPC::Resource<T>::Resource( const std::string& filename ) : filename(filename) {
    this->creator_pid = getpid();
    T::Create( this->filename );
}

template <typename T> IPC::Resource<T>::Resource( const std::string& filename, size_t n ) : filename(filename) {
    this->creator_pid = getpid();
    T::Create( this->filename, n );
}

template <typename T> IPC::Resource<T>::Resource( const std::string& filename, size_t n, size_t m ) : filename(filename) {
    this->creator_pid = getpid();
    T::Create( this->filename, n, m );
}

/**
 * Destructor implementation.
 */
template <typename T> IPC::Resource<T>::~Resource() {
    /* destroys the object only if the destructor was called from the creator process
     * this prevents forked childs from destroying resources that they don't own */
    if( this->creator_pid > 0 && this->creator_pid == getpid() )
        T::Destroy( this->filename );
}


#endif
