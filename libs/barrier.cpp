/* include area */
#include "barrier.hpp"
#include "log.hpp"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <utility>

using std::endl;
using std::string;


/**
 * Creates a new Barrier resource
 * 
 * \param filename The filename to use for the IPC resource for the barrier.
 */
void IPC::Barrier::Create( IPC::Key key ) {
    /* creates the semaphore */
    int semid = semget( key.value, 1, 0644 | IPC_CREAT | IPC_EXCL );
    if( semid < 0 )
        throw IPC::Barrier::Error( static_cast<string>( "semget: " ) + strerror( errno ) );

    LOG_DBG << "semid: " << semid << endl;
}

void IPC::Barrier::Create( IPC::Key key, size_t n ) {
    Barrier::Create( key );

    /* opens the semaphore (expecting it's already created) */
    int semid = semget( key.value, 1, 0644 );
    if( semid < 0 )
        throw IPC::Barrier::Error( static_cast<string>( "semget: " ) + strerror( errno ) );

    /* initializes the semaphore */
    if( semctl( semid, 0, SETVAL, n ) )
        throw IPC::Barrier::Error( static_cast<string>( "semctl: ") + strerror( errno ) );
}

/**
 * Destroys a barrier created before, deallocating the OS resources.
 * 
 * \param filename Filename of the barrier to destroy.
 * \param proj_id  ASCII char to get the token.
 */
void IPC::Barrier::Destroy( IPC::Key key ) {
    /* gets the semaphore */
    int semid = semget( key.value, 1, 0644 );
    if( semid < 0 ) {
        LOG_DBG << strerror( errno ) << " - key=" << key << endl;
        return;
    }

    LOG_DBG << "destroy: " << semid << " - key=" << key << endl;
    semctl( semid, 0, IPC_RMID, NULL );
}


/**
 * Constructor implementation.
 *
 * \param n Number of processes to barrier.
 */
IPC::Barrier::Barrier( IPC::Key key, size_t n ) : Barrier(key) {
    this->n = n;

    /* initializes the semaphore */
    this->set( n );
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

IPC::Barrier::Barrier( IPC::Key key ) {
    /* opens the semaphore (expecting it's already created) */
    this->semid = semget( key.value, 1, 0644 );
    if( this->semid < 0 )
        throw IPC::Barrier::Error( static_cast<string>( "semget: " ) + strerror( errno ) );

    LOG_DBG << "barrier create: " << this->semid << endl;
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
        throw IPC::Barrier::Error( static_cast<string>( "barrier wait: " ) + strerror( errno ) );
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
        throw IPC::Barrier::Error( static_cast<string>( "barrier signal" ) + strerror( errno ) );
}

/**
 * Sets the barrier to the original value.
 */
void IPC::Barrier::reset() {
    this->set( this->n );
}

void IPC::Barrier::set( size_t n ) {
    /* sets the semaphore to the initial value */
    if( semctl( this->semid, 0, SETVAL, n ) ) {
        throw IPC::Barrier::Error( static_cast<string>( "semctl: ") + strerror( errno ) );
    }
}