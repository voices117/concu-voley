/* include area */
#include "player.hpp"

using std::size_t;
using std::string;
using IPC::Lock;
using IPC::SharedMem;


/* each player has an array of 'max_matches' (to store the IDs of the other players it
 * has played with), the number of matches it has already played and it's state.
 * an extra size_t is used by the table to hold the number of players initialized */
#define PLAYERS_TABLE_SIZE( max_players, max_matches ) ( max_players * ( max_matches + 2 ) + 1 )


/**
 * Creates a shared table that holds the players state.
 * 
 * \param key The key of the shared resource.
 */
void PlayersTable::Create( IPC::Key key, size_t max_players, size_t max_matches ) {
    
    size_t size = PLAYERS_TABLE_SIZE( max_players, max_matches );

    SharedMem<size_t>::Create( key, size );
    SharedMem<size_t> mem{ key, size };

    /* initialize to zero */
    mem.set_zero();
}

/**
 * Destroys the players table shared resource.
 * 
 * \param key The key of the shared resource.
 */
void PlayersTable::Destroy( IPC::Key key ) {
    SharedMem<size_t>::Destroy( key );
}


/**
 * Constructor implementation.
 * 
 */
PlayersTable::PlayersTable( IPC::Key key, size_t max_players, size_t max_matches ) : max_players(max_players),
                                                                                                     max_matches(max_matches),
                                                                                                     storage(key, PLAYERS_TABLE_SIZE( max_players, max_matches ) ) {
}

/**
 * Destructor implementation.
 */
PlayersTable::~PlayersTable() {
}

/**
 * Adds a new player.
 */
void PlayersTable::add_player() {
    player_t id = ++this->storage[0];

    /* initializes the memory */
    size_t *data = this->get_ptr( id );
    data[0] = static_cast<size_t>( PlayerState::idle );
    data[id + 1] = 0; // num_matches
    for( size_t i = 0; i < this->max_matches; i++ ) {
        data[id + i + 2] = 0;
    }

    LOG_DBG << "added player " << id << std::endl;
}


/**
 * Gets a player from the table.
 * 
 * \param id ID of the player.
 * \return Player associated to the given ID.
 */
Player PlayersTable::get_player( player_t id ) {
    if( id == 0 ) {
        throw IPC::Error( "Invalid player id" );
    }

    off_t offset = ( off_t )id;
    off_t len = 1;
    // TODO: change fd
    return Player{ id, this->get_ptr( id ), Lock{ 1, offset, len, Lock::Mode::write } };
}


/**
 * Gets a player from the table.
 * 
 * \param id ID of the player.
 * \return A read-only player instance associated to the given ID.
 */
PlayerRO PlayersTable::get_player_ro( player_t id ) {
    if( id == 0 ) {
        throw IPC::Error( "Invalid player id" );
    }

    off_t offset = ( off_t )id;
    off_t len = 1;
    // TODO: change fd
    return PlayerRO{ id, this->get_ptr( id ), Lock{ 1, offset, len, Lock::Mode::read } };
}

/**
 * Returns the number if players in the table.
 * 
 * \return Number of players.
 */
size_t PlayersTable::size() {
    return this->storage[0];
}

size_t *PlayersTable::get_ptr( player_t id ) {
    try {
        return this->storage.get_ptr( ( id - 1 ) * ( this->max_matches + 2 ) + 1 );
    } catch( const IPC::SharedMemError& e ) {
        LOG_DBG << e.what() << " id: " << id << std::endl;
        throw;
    }
}

/**
 * Gets an iterator for the table.
 * 
 * \return Iterator at the beggining of the table.
 */
PlayersTable::iterator PlayersTable::begin() {
    return PlayersTable::iterator{ *this, 1 };
}

/**
 * Returns an iterator representing the end of the table.
 * 
 * \return End iterator.
 */
PlayersTable::iterator PlayersTable::end() {
    return PlayersTable::iterator{ *this, this->size() + 1 };
}

/**
 * Iterator API.
 */
PlayersTable::iterator& PlayersTable::iterator::operator++() {
    this->index++;
    return *this;
}

PlayerRO PlayersTable::iterator::operator*() {
    return this->table->get_player_ro( this->index );
}

bool PlayersTable::iterator::operator==( PlayersTable::iterator& other ) {
    return ( this->index == other.index );
}

bool PlayersTable::iterator::operator!=( PlayersTable::iterator& other ) {
    return !( *this == other );
}


/**
 * Player
 */


bool Player::has_played_with( const Player& other ) const {
    for( size_t i = 0; i < *this->num_pairs; i++ ) {
        if( this->pairs[i] == other.id ) {
            return true;
        }
    }
    return false;
}

size_t Player::num_matches() const {
    return *this->num_pairs;
}

void Player::set_state( PlayerState new_state ) {
    *this->state = static_cast<size_t>( new_state );
}

PlayerState Player::get_state() {
    return static_cast<PlayerState>( *this->state );
}

void Player::set_pair( Player& other ) {
    if( this->has_played_with( other ) ) {
        throw IPC::Error( "Pair repeated" );
    }

    this->pairs[*this->num_pairs] = other.id;
    other.pairs[*other.num_pairs] = this->id;

    *this->num_pairs += 1;
    *other.num_pairs += 1;
}
