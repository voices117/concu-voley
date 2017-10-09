#ifndef PLAYER_HPP
#define PLAYER_HPP

/* include area */
#include <vector>
#include <set>


/* types */
typedef std::size_t player_t;


enum class PlayerState {
    unavailable,
    idle,
    playing,
    done,
};


struct Player {
    /** The ID of the player represented. */
    player_t id;

    /** The current player's state. */
    PlayerState state;

    /** The players with whom has already played with. */
    std::set<player_t> pairs;
};


/**
 * The table with the information about the players.
 */
class PlayersTable {
public:

    /**
     * Iterator for the class.
     */
    class iterator {
    public:
        iterator( PlayersTable& table ) { this->table = &table; }
        iterator( PlayersTable& table, std::size_t index ) : index(index) { this->table = &table; }
        ~iterator() {}

        bool operator==( iterator& other );
        bool operator!=( iterator& other );
        iterator& operator++();
        Player& operator*();
    
    private:
        std::size_t index{0};
        PlayersTable* table;
    };

    PlayersTable();
    ~PlayersTable();

    /* query */
    void add_player();
    bool has_played_with( player_t p1, player_t p2 ) const;
    Player& get_player( player_t id );
    std::size_t size();

    /* iteration */
    iterator begin();
    iterator end();
    
private:

    std::vector<Player> players;
};


#endif
