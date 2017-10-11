/* include area */
#include "log.hpp"
#include <errno.h>
#include <string>
#include <string.h>
#include <iostream>
#include <sys/types.h>

using std::string;
using IPC::Lock;


/**
 * LogStream move constructor.
 * 
 * \param other Instance to take resources from.
 */
LogStream::LogStream( LogStream&& other ) : lock(std::move(other.lock)), streams(other.streams) {
}


LogStream LogStream::operator<<( std::ostream& ( *manipulator )( std::ostream& ) ) {
    for( const auto s: this->streams ) {
        *s << manipulator;
    }
    return std::move( *this );
}

LogStream& LogStream::operator=( LogStream&& other ) {
    this->lock = std::move( other.lock );
    this->streams = other.streams;
    return *this;
}


/**
 * Static method that returns the singleton's instance.
 * @return The singleton instance.
 */
Log& Log::get_instance() {
    static Log instance;
    return instance;
}

/**
 * Instance constructor.
 */
Log::Log() {
    // TODO: cout should not be automatically registered
    this->streams.push_back( &std::cout );

    this->fd = open( LOG_LOCK_FILE, O_RDWR | O_TRUNC | O_CREAT, 0644 );
    if( this->fd < 0 ) {
        throw IPC::Error( "Could not open log file: " + static_cast<string>( strerror( errno ) ) );
    }
}

/**
 * Messages streamed through this method are logged as \c Log::Level::debug.
 */
Log& Log::debug() {
    /* if debug messages are to be ignored, returns a dummy log */
    if( this->level < Log::Level::debug ) {
        static Log dummy{};
        dummy.remove_listeners();
        return dummy;
    }
    return *this;
}

/**
 * Destructor implementation.
 */
Log::~Log() {
    if( this->fd > 0 ) {
        close( this->fd );
        this->fd = 1;
    }
}

/**
 * Sets the logging level (the messages that will be logged and ignored).
 * If the level set in the log as manipulator is lower than the level set with \c set_level then that
 * message is ignored.
 * 
 * \param level The level to set.
 */
void Log::set_level( Log::Level level ) {
    this->level = level;
}

/**
 * Adds an ostream where the log will redirect the messages (can be called many times).
 *
 * \param os Output stream to register. 
 */
void Log::add_listener( std::ostream& os ) {
    this->streams.push_back( &os );
}

/**
 * Removes all listener streams registered.
 */
void Log::remove_listeners() {
    this->streams.clear();
}


/**
 * Implementation of the output operator for stream manipulators.
 */
LogStream Log::operator<<( std::ostream& ( *manipulator )( std::ostream& ) ) {
    Lock lock{ this->fd, Lock::Mode::write };
    return LogStream{ this->streams, std::move( lock ) } << manipulator;
}
