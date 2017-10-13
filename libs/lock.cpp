/* include area */
#include "lock.hpp"
#include "log.hpp"
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>

using std::string;
using std::endl;
using std::cout;
using IPC::Lock;


/**
 * Constructor implementation
 */
Lock::Lock( int fd, off_t offset, off_t length, Lock::Mode mode ) : fd(fd) {
    if( this->fd < 0 ) {
        throw Lock::Error( strerror( errno ) );
    }

    this->fl.l_type = ( mode == Lock::Mode::read ? F_RDLCK : F_WRLCK );
	this->fl.l_whence = SEEK_SET;
	this->fl.l_start = offset;
    this->fl.l_len = length;
    this->fl.l_pid = getpid();
	if( fcntl( this->fd, F_SETLKW, &this->fl ) == -1 ) {
        throw Lock::Error( strerror( errno ) );
    }

    string mode_s{ mode == Lock::Mode::read ? "read" : "write" };
    //cout << "Lock: fd=" << fd << " - " << mode_s << " mode - offset: " << offset << " len: " << length << endl;
}

/**
 * Lock the whole file.
 * 
 * \param fd   File descriptor to lock.
 * \param mode Lock mode.
 */
Lock::Lock( int fd, Mode mode ) : Lock( fd, 0, 0, mode ) {
}

/**
 * Move constructor.
 */
Lock::Lock( Lock&& other ) {
    std::swap( this->fd, other.fd );
    this->fl = other.fl;
}

/**
 * Destructor implementation.
 */
Lock::~Lock() {
    if( this->fd < 0 ) {
        return;
    }
    
    this->fl.l_type = F_UNLCK;
    if( fcntl( this->fd, F_SETLK, &this->fl ) == -1 ) {
        //LOG_DBG << "fcntl: " << strerror( errno ) << endl;
    }

    //cout << getpid() << " Lock: released: " << this->fd << endl;
}

Lock& Lock::operator=( Lock&& other ) {
    this->fd = other.fd;
    this->fl = other.fl;
    other.fd = -1;
    return *this;
}
