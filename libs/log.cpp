/* include area */
#include "log.hpp"
#include <iostream>


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
 * Removes all listener streams registered.
 */
void Log::remove_listeners() {
    this->streams.clear();
}


/**
 * Implementation of the output operator for stream manipulators.
 */
Log& Log::operator<<( std::ostream& ( *manipulator )( std::ostream& ) ) {
    // TODO: use internal vector
    for( const auto s: this->streams )
        *s << manipulator;
    return *this;
}
