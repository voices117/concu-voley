#ifndef SHARED_MEM_HPP
#define SHARED_MEM_HPP

/* include area */
#include "ipc.hpp"
#include "log.hpp"
#include <errno.h>
#include <string>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

using std::size_t;


namespace IPC {

    using std::string;

    /**
     * Shared memory errors.
     */
    class SharedMemError : public IPC::Error {
    public:
        SharedMemError( const string& message ) : IPC::Error( message ) {}
        ~SharedMemError() {}
    };

    template <typename T> class SharedMem {
    public:
        
        /** Static methods to create and destroy shared memory resources. */
        static void Create( const string& filename, size_t n );
        static void Destroy( const string& filename );
        
        /** Class methods. */
        SharedMem( const string& filename, size_t n );
        ~SharedMem();

        void write( size_t index, const T* elems, size_t num_elems );
        void read( size_t index, T* elems, size_t num_elems );

        /** Returns a pointer to the given index */
        T* get_ptr( size_t index );

        /** Initializes allocated memory with zeros. */
        void set_zero();

        /** Derreference operator so this class simulates a pointer. */
        T& operator*();
        T& operator[]( size_t index );

    private:
        /** shared mempry ID */
        int	shmid{ -1 };
        /** pointer to the data */
        T* data{ nullptr };
        /** Number of elements allocated. */
        size_t n{0};
    };
}


/**
 * Creates a shared memory resource in the OS.
 * 
 * \param filename File associated to the resource.
 */
template <typename T> void IPC::SharedMem<T>::Create( const std::string& filename, size_t n ) {
    key_t key = ftok( filename.c_str(), 'a' );
    if( key == -1 )
        throw IPC::SharedMemError( "ftok: " + static_cast<std::string>( strerror( errno ) ) );

    /* gets the resource ID */
    int shmid = shmget( key, sizeof( T ) * n, 0644 | IPC_CREAT | IPC_EXCL );
    if( shmid < 0 )
        throw IPC::SharedMemError( "create shmget: " + static_cast<std::string>( strerror( errno ) ) );

    LOG_DBG << "new shared mem: " << shmid << std::endl;
}

/**
 * Destroys a shared memory resource (see \c Create).
 * 
 * \param filename File associated to the resource.
 */
template <typename T> void IPC::SharedMem<T>::Destroy( const std::string& filename ) {
    key_t key = ftok( filename.c_str(), 'a' );
    if( key == -1 ) {
        LOG_DBG << strerror( errno ) << std::endl;
        return;
    }

    /* gets the resource ID */
    int shmid = shmget( key, sizeof( T ), 0644 );
    if( shmid < 0 ) {
        LOG_DBG << strerror( errno ) << std::endl;
        return;
    }

    /* destroys the resource */
    if( shmctl( shmid, IPC_RMID, NULL ) < 0 )
        LOG_DBG << strerror( errno ) << std::endl;

    LOG_DBG << "destroy: " << shmid << std::endl;
}


/**
 * Constructor implementation.
 */
template <typename T> IPC::SharedMem<T>::SharedMem( const std::string& filename, size_t n ) : n(n) {
    key_t key = ftok( filename.c_str(), 'a' );
    if( key == -1 )
        throw IPC::SharedMemError( "ftok: " + static_cast<std::string>( strerror( errno ) ) );

    /* gets the resource ID */
    int shmid = shmget( key, sizeof( T ) * n, 0644 );
    if( shmid < 0 )
        throw IPC::SharedMemError( "shmget: " + static_cast<std::string>( strerror( errno ) ) );

    /* attaches to the shared memory */
    void* ptr = shmat( shmid, NULL, 0 );
    if ( ptr == ( void *) -1 ) {
        throw IPC::SharedMemError( "shmid: " + static_cast<std::string>( strerror( errno ) ) );
    }

    /* initializes the object */
    this->shmid = shmid;
    this->data = static_cast<T *>( ptr );
}

/**
 * Destructor implementation.
 */
template <typename T> IPC::SharedMem<T>::~SharedMem() {
    if( this->shmid < 0 )
        return;
    
    /* detaches from the shared shared memory */
    if( shmdt( ( void *) this->data ) < 0 )
        LOG_DBG << strerror( errno ) << std::endl;

    this->shmid = -1;
    this->data = nullptr;
}


template <typename T> void IPC::SharedMem<T>::write( size_t index, const T* elems, size_t num_elems ) {
    if( num_elems > this->n - index )
        throw IPC::SharedMemError( "Out of bounds" );

    memcpy( this->data + index, elems, sizeof( T ) * num_elems );
}


template <typename T> void IPC::SharedMem<T>::read( size_t index, T* elems, size_t num_elems ) {
    if( num_elems > this->n - index )
        throw IPC::SharedMemError( "Out of bounds" );
    
    memcpy( elems, this->data + index, sizeof( T ) * num_elems );
}


template <typename T> T* IPC::SharedMem<T>::get_ptr( size_t index ) {
    if( index >= this->n ) {
        throw IPC::SharedMemError( "Out of bounds: " + std::to_string( index ) );
    }
    return this->data + index;
}


template <typename T> void IPC::SharedMem<T>::set_zero() {
    memset( this->data, 0, this->n );
}


/**
 * Operators
 */

 /**
  * Returns a reference to the object in the shared memory.
  * 
  * \return Reference to the object of type \c T.
  */
template <typename T> T& IPC::SharedMem<T>::operator*() {
    return *this->data;
}

template <typename T> T& IPC::SharedMem<T>::operator[]( size_t index ) {
    return this->data[index];
}


#endif
