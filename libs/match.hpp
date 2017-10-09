#ifndef MATCH_HPP
#define MATCH_HPP

/* include area */
#include "player.hpp"
#include <iostream>


struct Team {
    player_t player1;
    player_t player2;
};

struct Match {
    Team team1;
    Team team2;
};

enum class Status {
    played,
    interrupted,
};

struct MatchResult {
    Match match;
    Status status;
    int team1_points;
    int team2_points;

    friend std::ostream& operator<<( std::ostream& os, const MatchResult& r ) {
        os << "{ " << r.team1_points << ", " << r.team2_points << " }";
        return os;
    }
};


#endif
