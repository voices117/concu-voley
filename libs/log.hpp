/**
 * An IPC secure log.
 */

#ifndef LOG_HPP
#define LOG_HPP


/* include area */
#include "str_utils.hpp"
#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

/** Internal macro to show information about the process logging the message. */
#define MSG_INFO "(" << getpid() << ") " __FILE__ ":" XSTR( __LINE__ )

/**
 * Short access to the global log that also prints the source file and line where the message was
 * printed.
 *
 * Examples:
 *
 *   LOG << "message: " << 123 << std::endl;
 */
#define LOG Log::get_instance() << GREEN_TEXT( MSG_INFO )

/**
 * Short access to the global log to print debug messages.
 * Same usage as \c LOG.
 */
#define LOG_DBG Log::get_instance().debug() << BLUE_TEXT( MSG_INFO ) 


/**
 * Log singleton.
 */
class Log {

public:
    /** Log level manipulator. */
    enum class Level {
        normal,
        debug,
    };

    /** Returns the singleton instance. */
    static Log& get_instance();

    /** Sets the level of the logger that filters messages. */
    void set_level( Level level );
    void remove_listeners();

    /** Used to log debug messages. */
    Log& debug();
    
    /** Output operator to give the log an \a ostream interface. */
    template <typename T> Log& operator<<( const T&& t );
    template <typename T> Log& operator<<( const T& t );
    Log& operator<<( std::ostream& ( *manipulator )( std::ostream& ) );

private:
    /** Defines the level of the logged messages. */
    Level level{ Level::normal };

    Log();
    Log( Log& log ) = delete;

    /** Vector containing the other streams that the log writes to. */
    std::vector<std::ostream *> streams{};
};


/**
 * Implementation of the stream-like interface for the Log object.
 */
template <typename T> Log& Log::operator<<( const T&& t ) {
    // TODO: make IPC safe access
    for( const auto s: this->streams )
        *s << t;
    return *this;
}

template <typename T> Log& Log::operator<<( const T& t ) {    
    // TODO: make IPC safe access
    for( const auto s: this->streams )
        *s << t;
    return *this;
}


#endif
