#ifndef PLAYER_HPP
#define PLAYER_HPP

/* include area */
#include "log.hpp"
#include "shared_mem.hpp"
#include <vector>
#include <set>
#include <string>


using std::size_t;


/* types */
typedef size_t player_t;


enum class PlayerState {
    unavailable = 0,
    idle = 1,
    playing = 2,
    done = 3,
};

/** Forward declaration. */
class PlayersTable;

class Player {
    friend class PlayersTable;

public:
    Player( Player&& other ) {
        this->id = other.id;
        this->state = other.state;
        this->pairs = other.pairs;
        this->num_pairs = other.num_pairs;
        
        other.id = 0;
        other.state = nullptr;
        other.pairs = nullptr;
        other.num_pairs = nullptr;
    }
    ~Player() {}

    void operator=( Player& other ) = delete;
    void operator=( const Player& other ) = delete;
    
    /** The ID of the player represented. */
    player_t id;
    
    /** Returns \c true if has player with the other player. */
    bool has_played_with( Player& other ) const;
    
    void set_state( PlayerState new_state );
    PlayerState get_state();
    
    /** Indicates that two players have already been paired. */
    void set_pair( Player& other );
    
    /** Returns the number of matches played. */
    size_t num_matches() const;

    bool operator==( Player& other ) { return this->id == other.id; }
    
private:
    Player( player_t id, size_t *data ) {
        this->id = id;
        this->state = &data[0];
        this->num_pairs = &data[1];
        this->pairs = &data[2];
    }
    
    /** The current player's state. */
    size_t *state{nullptr};

    /** Array with the pairs that this player had. */
    player_t *pairs{nullptr};

    /** Number of pairs this player had. */
    size_t *num_pairs{nullptr};
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
        iterator( PlayersTable& table, size_t index ) : index(index) { this->table = &table; }
        ~iterator() {}

        bool operator==( iterator& other );
        bool operator!=( iterator& other );
        iterator& operator++();
        Player operator*();
        
    private:
        size_t index{1};
        PlayersTable* table;
    };
    
    PlayersTable( size_t max_players, size_t max_matches, IPC::SharedMem<size_t>& storage );
    ~PlayersTable();
    
    /* query */
    void add_player();
    Player get_player( player_t id );
    size_t size();
    
    /* iteration */
    iterator begin();
    iterator end();
    
    size_t max_players{0};
    size_t max_matches{0};

private:

    IPC::SharedMem<size_t>& storage;
    size_t *get_ptr( player_t id );
};


#endif
