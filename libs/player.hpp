#ifndef PLAYER_HPP
#define PLAYER_HPP

/* include area */
#include "lock.hpp"
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
class PlayersRO;

class Player {
    friend class PlayerRO;
    friend class PlayersTable;

public:
    Player( Player&& other ) : lock(std::move(other.lock)) {
        this->id = other.id;
        this->state = other.state;
        this->pairs = other.pairs;
        this->num_pairs = other.num_pairs;
        
        other.id = 0;
        other.state = nullptr;
        other.pairs = nullptr;
        other.num_pairs = nullptr;
    }
    virtual ~Player() {}

    void operator=( Player& other ) = delete;
    void operator=( const Player& other ) = delete;
    
    /** The ID of the player represented. */
    player_t id;
    
    /** Returns \c true if has player with the other player. */
    bool has_played_with( const Player& other ) const;
    
    void set_state( PlayerState new_state );
    PlayerState get_state();
    
    /** Indicates that two players have already been paired. */
    void set_pair( Player& other );
    
    /** Returns the number of matches played. */
    size_t num_matches() const;

    bool operator==( Player& other ) { return this->id == other.id; }
    
protected:
    Player( player_t id, size_t *data, IPC::Lock lock ) : lock(std::move(lock)) {
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

    /** The memory lock. */
    IPC::Lock lock;
};


/**
 * Read-only player interface.
 * Locks the table while the object is alive.
 */
class PlayerRO {
    friend class PlayersTable;

public:
    PlayerRO( PlayerRO&& other ) : player(std::move( other.player)) { this->id = other.id; }
    ~PlayerRO() {}
    
    void operator=( PlayerRO& other ) = delete;
    void operator=( const PlayerRO& other ) = delete;
    
    /** The ID of the player represented. */
    player_t id{0};
    
    /** Returns \c true if has player with the other player. */
    bool has_played_with( const PlayerRO& other ) const { return this->player.has_played_with( other.player ); }
    bool has_played_with( const Player& other ) const { return this->player.has_played_with( other ); }
    
    PlayerState get_state() { return this->player.get_state(); }
    
    /** Returns the number of matches played. */
    size_t num_matches() const { return this->player.num_matches(); }
    
    bool operator==( PlayerRO& other ) { return this->id == other.id; }
    
protected:
    PlayerRO( player_t id, size_t *data, IPC::Lock&& lock ) : id(id), player(id, data, std::move(lock)) {} 
    Player player;

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
        PlayerRO operator*();
        
        size_t index{1};
    private:
        PlayersTable* table{nullptr};
    };

    static void Create( const std::string& filename, size_t max_players, size_t max_matches );
    static void Destroy( const std::string& filename );
    
    PlayersTable( const std::string& filename, size_t max_players, size_t max_matches );
    ~PlayersTable();
    
    /* query */
    void add_player();
    Player get_player( player_t id );
    PlayerRO get_player_ro( player_t id );
    size_t size();
    
    /* iteration */
    iterator begin();
    iterator end();
    
    size_t max_players{0};
    size_t max_matches{0};

private:

    IPC::SharedMem<size_t> storage;
    size_t *get_ptr( player_t id );
};


#endif
