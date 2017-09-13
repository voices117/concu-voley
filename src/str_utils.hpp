/**
 * Miscellaneous utilities for string manipulation.
 */

#ifndef STR_UTILS_HPP
#define STR_UTILS_HPP


/** Converts \a s to a string. */
#define STR( s ) #s

/** Expands and converts \a s to a string. */
#define XSTR( s ) STR( s )

/** Wrapper for strings so the output is ANSI colored. */
#define BLUE_TEXT( text ) "\x1B[1;92m[ " text " ]\x1B[0m "


#endif
