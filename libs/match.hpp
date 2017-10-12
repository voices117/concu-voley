#ifndef MATCH_HPP
#define MATCH_HPP

/* include area */
#include "player.hpp"
#include <iostream>


/**
 * Team representation (a pair of 2 players).
 */
struct Team {
    player_t player1;
    player_t player2;

    friend std::ostream& operator<<( std::ostream& os, const Team& t ) {
        os << "p1=" << t.player1 << ", p2=" << t.player2;
        return os;
    }
};

/**
 * A match between 2 teams.
 */
struct Match {
    Team team1;
    Team team2;

    friend std::ostream& operator<<( std::ostream& os, const Match& m ) {
        return os << "team1(" << m.team1 << ") vs team2(" << m.team2 << ")";
    }
};

/**
 * The status of a match played.
 */
enum class Status {
    played,
    interrupted,
};

/**
 * Match result representation.
 */
struct MatchResult {
    /** The match that was played. */
    Match match;
    /** The status of the match. */
    Status status;

    /** The sets won by each team. */
    int team1_sets;
    int team2_sets;

    friend std::ostream& operator<<( std::ostream& os, const MatchResult& r ) {
        if( r.status == Status::interrupted ) {
            os << "[interrupted] ";
        }
        return os << r.match << " = (" << r.team1_sets << ", " << r.team2_sets << ")";
    }
};


#endif
