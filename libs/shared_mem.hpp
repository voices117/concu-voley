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
        static void Create( const string& filename );
        static void Destroy( const string& filename );
        
        /** Class methods. */
        SharedMem( const string& filename );
        ~SharedMem();

        /** Derreference operator so this class simulates a pointer. */
        T& operator*();

    private:
        /** shared mempry ID */
        int	shmid{ -1 };
        /** pointer to the data */
        T* data{ nullptr };
    };
}


/**
 * Creates a shared memory resource in the OS.
 * 
 * \param filename File associated to the resource.
 */
template <typename T> void IPC::SharedMem<T>::Create( const std::string& filename ) {
    key_t key = ftok( filename.c_str(), 0 );
    if( key == -1 )
        throw IPC::SharedMemError( "ftok: " + static_cast<std::string>( strerror( errno ) ) );

    /* gets the resource ID */
    int shmid = shmget( key, sizeof( T ), 0644 | IPC_CREAT | IPC_EXCL );
    if( shmid < 0 )
        throw IPC::SharedMemError( "shmget: " + static_cast<std::string>( strerror( errno ) ) );

    LOG_DBG << "new shared mem: " << shmid << std::endl;
}

/**
 * Destroys a shared memory resource (see \c Create).
 * 
 * \param filename File associated to the resource.
 */
template <typename T> void IPC::SharedMem<T>::Destroy( const std::string& filename ) {
    key_t key = ftok( filename.c_str(), 0 );
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
template <typename T> IPC::SharedMem<T>::SharedMem( const std::string& filename ) {
    key_t key = ftok( filename.c_str(), 0 );
    if( key == -1 )
        throw IPC::SharedMemError( "ftok: " + static_cast<std::string>( strerror( errno ) ) );

    /* gets the resource ID */
    int shmid = shmget( key, sizeof( T ), 0644 );
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


#endif
