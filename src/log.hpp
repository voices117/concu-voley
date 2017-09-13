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
#include <vector>


/**
 * Short access to the global log that also prints the source file and line where the message was
 * printed.
 *
 * Examples:
 *
 *   LOG << "message: " << 123 << std::endl;
 */
#define LOG Log::get_instance() << BLUE_TEXT( __FILE__ ":" XSTR( __LINE__ ) )


/**
 * Log singleton.
 */
class Log {

public:
    /** Returns the singleton instance. */
    static Log& get_instance();

    /** Output operator to give the log an \a ostream interface. */
    template <typename T> Log& operator<<( const T&& t );
    template <typename T> Log& operator<<( const T& t );
    Log& operator<<( std::ostream& ( *manipulator )( std::ostream& ) );

private:
    Log();

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
