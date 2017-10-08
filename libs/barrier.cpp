/* include area */
#include "barrier.hpp"
#include "log.hpp"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <utility>


/**
 * Creates a new Barrier resource
 * 
 * \param filename The filename to use for the IPC resource for the barrier.
 * \param n Number of "signals" until the processes waiting the barrier are released.
 */
void IPC::Barrier::Create( const std::string& filename ) {
    key_t key = ftok( filename.c_str(), 0 );
    if( key < 0 )
        throw IPC::Barrier::Error( static_cast<std::string>( "ftok" ) + strerror( errno ) );

    /* creates the semaphore */
    int semid = semget( key, 1, 0644 | IPC_CREAT | IPC_EXCL );
    if( semid < 0 )
        throw IPC::Barrier::Error( static_cast<std::string>( "semget" ) + strerror( errno ) );
}

/**
 * Destroys a semaphore created before, deallocating the OS resources.
 * 
 * \param filename Filename of the semaphore to destroy.
 */
void IPC::Barrier::Destroy( const std::string& filename ) {
    key_t key = ftok( filename.c_str(), 0 );
    if( key < 0 ) {
        LOG_DBG << strerror( errno );
        return;
    }

    /* gets the semaphore */
    int semid = semget( key, 1, 0644 );
    if( semid < 0 ) {
        LOG_DBG << strerror( errno );
        return;
    }

    LOG_DBG << "destroy" << std::endl;

    semctl( semid, 0, IPC_RMID, NULL );
}


/**
 * Constructor implementation.
 *
 * \param n Number of processes to barrier.
 */
IPC::Barrier::Barrier( const std::string& filename, size_t n ) : n{n} {
    key_t key = ftok( filename.c_str(), 0 );
    if( key < 0 )
        throw IPC::Barrier::Error( static_cast<std::string>( "ftok: " ) + strerror( errno ) );

    /* opens the semaphore (expecting it's already created) */
    this->semid = semget( key, 1, 0644 | IPC_CREAT );
    if( this->semid < 0 )
        throw IPC::Barrier::Error( static_cast<std::string>( "semget: " ) + strerror( errno ) );

    /* initializes the semaphore */
    if( semctl( this->semid, 0, SETVAL, n ) )
        throw IPC::Barrier::Error( static_cast<std::string>( "semctl: ") + strerror( errno ) );
}

/**
 * Copy constructor
 * 
 * \param other Instance to copy from.
 */
IPC::Barrier::Barrier( const Barrier& other ) : n{other.n}, semid{other.semid}  {
}

/**
 * Move constructor.
 */
IPC::Barrier::Barrier( Barrier&& other ) : n{other.n}, semid{other.semid} {
    other.semid = -1;
    other.n = 0;
}

/**
 * Copy constructor.
 */
IPC::Barrier& IPC::Barrier::operator=( Barrier&& other ) {
    std::swap( this->semid, other.semid );
    std::swap( this->n, other.n );
    return *this;
}

/**
 * Destructor implementation.
 */
IPC::Barrier::~Barrier() {
}


/**
 * Blocks the calling process until the expected number of processes have reached the barrier.
 */
void IPC::Barrier::wait() {
    struct sembuf sops;
    sops.sem_flg = SEM_UNDO;
    sops.sem_num = 0;

    /* waits for the value to be 0 */
    sops.sem_op = 0;

    int rv = semop( this->semid, &sops, 1 );
    if( rv < 0 )
        throw IPC::Barrier::Error( static_cast<std::string>( "wait" ) + strerror( errno ) );
}

/**
 * Signals that a process has reached the barrier.
 */
void IPC::Barrier::signal() {
    struct sembuf sops;
    sops.sem_flg = SEM_UNDO;
    sops.sem_num = 0;

    /* decrements the semaphore's counter */
    sops.sem_op = -1;

    int rv = semop( this->semid, &sops, 1 );
    if( rv < 0 )
        throw IPC::Barrier::Error( static_cast<std::string>( "signal" ) + strerror( errno ) );
}
