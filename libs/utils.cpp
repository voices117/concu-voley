/* include area */
#include "utils.hpp"


/**
 * Returns a random integer in the range [min, max).
 * 
 * \param min Minimun number to be generated.
 * \param max Maximum number (not included) to be generated.
 * \return Random number.
 */
int Utils::rand_int( int min, int max ) {
    std::random_device rd;
    std::mt19937 rng( rd() );

    std::uniform_int_distribution<int> uni( min, max );    
    return uni( rng );
}

