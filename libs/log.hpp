/**
 * An IPC secure log.
 */

#ifndef LOG_HPP
#define LOG_HPP


/* include area */
#include "str_utils.hpp"
#include "lock.hpp"
#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>


/** File used to lock the log. */
#ifndef LOG_LOCK_FILE
#  define LOG_LOCK_FILE "/tmp/cv_log.lck"
#endif

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
 * Object to stream data into the log singleton.
 */
class LogStream {
    friend class Log;

public:
    LogStream( LogStream&& other );
    ~LogStream() {}

    template <typename T> LogStream operator<<( const T& t );
    LogStream& operator=( LogStream&& other );
    LogStream operator<<( std::ostream& ( *manipulator )( std::ostream& ) );
    
private:
    LogStream( std::vector<std::ostream *>& streams, IPC::Lock&& lock ) : lock(std::move(lock)), streams(streams) {}
    LogStream( const LogStream& other ) = delete;
    LogStream& operator=( const LogStream& other ) = delete;

    /** list of streams to output. */
    IPC::Lock lock;
    std::vector<std::ostream *>& streams;
};


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

    /** Destructor. */
    ~Log();

    /** Sets the level of the logger that filters messages. */
    void set_level( Level level );
    void add_listener( std::ostream& os );
    void remove_listeners();

    /** Used to log debug messages. */
    Log& debug();
    
    /** Output operator to give the log an \a ostream interface. */
    template <typename T> LogStream operator<<( const T& t );
    LogStream operator<<( std::ostream& ( *manipulator )( std::ostream& ) );

private:
    /** Defines the level of the logged messages. */
    Level level{ Level::normal };

    Log();
    Log( Log& log ) = delete;

    /** Vector containing the other streams that the log writes to. */
    std::vector<std::ostream *> streams{};

    /** Log file descriptor. */
    int fd;
};



/**
 * Redirects output to all the streams in the internal list.
 * 
 * \param t Object to stream.
 * \return Itself.
 */
template <typename T> LogStream LogStream::operator<<( const T& t ) {
    for( const auto s: this->streams ) {
        *s << t;
    }
    return std::move( *this );
}

template <typename T> LogStream Log::operator<<( const T& t ) {    
    /* creates a LogStream and returns it.
     * The LogStream will hold the lock until destroyed so no other process will be able to use
     * the log. */
    IPC::Lock lock{ this->fd, IPC::Lock::Mode::write };
    return LogStream{ this->streams, std::move( lock ) } << t;
}


#endif
