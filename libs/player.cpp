/* include area */
#include "player.hpp"

using std::size_t;
using std::string;


/**
 * Constructor implementation.
 * 
 */
PlayersTable::PlayersTable( size_t max_players, size_t max_matches, IPC::SharedMem<size_t>& storage ) : max_players(max_players),
                                                                                                     max_matches(max_matches),
                                                                                                     storage(storage) {
    /* the first element is the number of players in the table.
     * initializes the table with zero players */
    *this->storage.get_ptr( 0 ) = 0;
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
    
    Player p{ id, this->get_ptr( id ) };
    return p;
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
    return this->storage.get_ptr( ( id - 1 ) * ( this->max_matches + 2 ) + 1 );
}

/**
 * Gets an iterator for the table.
 * 
 * \return Iterator at the beggining of the table.
 */
PlayersTable::iterator PlayersTable::begin() {
    return PlayersTable::iterator( *this, 1 );
}

/**
 * Returns an iterator representing the end of the table.
 * 
 * \return End iterator.
 */
PlayersTable::iterator PlayersTable::end() {
    return PlayersTable::iterator( *this, this->size() );
}

/**
 * Iterator API.
 */
PlayersTable::iterator& PlayersTable::iterator::operator++() {
    this->index++;
    return *this;
}

Player PlayersTable::iterator::operator*() {
    return this->table->get_player( this->index );
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


bool Player::has_played_with( Player& other ) const {
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
