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
        os << "team1(" << r.match.team1.player1 << ", " << r.match.team1.player2 << ") + " << r.team1_points;
        os << " and team2(" << r.match.team2.player1 << ", " << r.match.team2.player2 << ") + " << r.team2_points << std::endl;
        return os;
    }
};


#endif
