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
 * Implementation of the output operator for stream manipulators.
 */
Log& Log::operator<<( std::ostream& ( *manipulator )( std::ostream& ) ) {
    std::cout << manipulator;
    return *this;
}
