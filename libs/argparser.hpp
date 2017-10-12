#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP


/* include area */
#include <iostream>
#include <string>
#include <sstream>
#include <typeinfo>
#include <vector>

using std::size_t;


/**
 * Gets an option from an argument parser.
 * \param  option The name of the option. For example "--opt".
 * \param  type   Type expected for the value associated to \a option.
 * \return        Value associated to \a option.
 */
#define get_option( option, type ) _get_option<type>( option, #type )

/**
 * Same as \c get_option but allows a default parameter in case the option is not present.
 * 
 * \param option  The name of the option.
 * \param default The default value in case the option is not present.
 * \param type    Type expected.
 * \return value  The value or the default.
 */
#define get_optional( option, default, type ) _get_optional<type>( option, default, #type )


/**
 * Argument parser class.
 */
class ArgParser {

public:
    class Error : public std::exception {

    public:
        Error( const std::string& msg ) : msg(msg) {}
        ~Error() {};

        const char *what() const throw() { return msg.c_str(); }

    private:
        std::string msg;
    };

    ArgParser( int argc, const char **argv );
    ~ArgParser();

    /** Returns \c true if the given option is present. */
    bool is_present( const std::string& option ) const;

    /** Counts the number of times an option appeared. */
    size_t count( const std::string& option ) const;
    
    /** DO NOT USE: use macro \a get_option or \a get_optional instead. */
    template<typename T> T _get_option( const std::string& option, const std::string& type ) const;
    template<typename T> T _get_optional( const std::string& option, const T& def, const std::string& type ) const;
    
private:
    int argc { -1 };
    const char **argv { NULL };

    int opt_index( const std::string& option ) const;
};


/**
 * Expects command line arguments.
 */
ArgParser::ArgParser( int argc, const char **argv ) : argc(argc), argv(argv) {
}

/**
 * Destructor implementation.
 */
ArgParser::~ArgParser() {
    this->argc = -1;
    this->argv = NULL;
}


/**
 * Returns \c true if the given option is present.
 * 
 * \param option Option name (for example, --option).
 * 
 * \return true  The option was found.
 * \return false The option was not found.
 */
bool ArgParser::is_present( const std::string& option ) const {
    return ( this->opt_index( option ) >= 0 );
}


/**
 * Finds the option specified and returns the associated value.
 * If not found, or it's not of the correct type, throws an exception.
 * 
 * \param option Option name.
 * \param type The expected type as string.
 */
template<typename T> T ArgParser::_get_option( const std::string& option, const std::string& type ) const {
    /* finds the option name into the list of command line arguments */
    int i = this->opt_index( option );
    if( i < 0 )
        /* not found */
        throw ArgParser::Error( "Expected option " + option + " [" + type + "]" );

    /* checks if there's a parameter after the option (--option param) */
    if( argc <= i + 1 )
        throw ArgParser::Error( "Expected a value of type " + type + " with option " + option );
    
    /* the option was found and has a parameter, now checks the type */
    std::istringstream ss{ this->argv[i + 1] };
    
    T value;
    ss >> value;
    
    if( !ss )
        throw ArgParser::Error( "Option " + option + " should be of type " + type );

    return value;
}

/**
 * Finds the option specified and returns the associated value.
 * If not found, returns a default value.
 * If found but it's not of the correct type, throws an exception.
 * 
 * \param option Option name.
 * \param def Default value in case is not present.
 * \param type The expected type as string.
 */
template<typename T> T ArgParser::_get_optional( const std::string& option, const T& def, const std::string& type ) const {
    try {
        return this->_get_option<T>( option, type );
    } catch( const ArgParser::Error& e ) {
        return def;
    }
}

/**
 * Returns the index in the arguments array of the given option (or -1 if not found).
 * 
 * \param option Option name.
 * \return int   The index of the option.
 */
int ArgParser::opt_index( const std::string& option ) const {
    int i = 0;
    while( i < this->argc ) {
        if( std::string( this->argv[i] ) == option )
            return i;
        i += 1;
    }

    /* not found */
    return -1;
}


/**
 * Counts the number of times an option appeared in the command line arguments.
 * 
 * \param option Option name.
 */
size_t ArgParser::count( const std::string& option ) const {
    size_t count = 0;
    for( int i = 0; i < this->argc; i++ ) {
        if( std::string( this->argv[i] ) == option ) {
            count += 1;
        }
    }
    return count;
}


#endif
