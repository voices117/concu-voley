/* include area */
#include "player.hpp"


/**
 * Constructor implementation.
 * 
 */
PlayersTable::PlayersTable() {
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
    Player p;
    p.id = this->players.size();
    p.state = PlayerState::idle;

    this->players.push_back( p );
}

/**
 * Gets a player from the table.
 * 
 * \param id ID of the player.
 * \return Player associated to the given ID.
 */
Player& PlayersTable::get_player( player_t id ) {
    return this->players.at( id );
}

/**
 * Returns the number if players in the table.
 * 
 * \return Number of players.
 */
std::size_t PlayersTable::size() {
    return this->players.size();
}

/**
 * Gets an iterator for the table.
 * 
 * \return Iterator at the beggining of the table.
 */
PlayersTable::iterator PlayersTable::begin() {
    return PlayersTable::iterator( *this );
}

/**
 * Returns an iterator representing the end of the table.
 * 
 * \return End iterator.
 */
PlayersTable::iterator PlayersTable::end() {
    return PlayersTable::iterator( *this, this->players.size() );
}

/**
 * Iterator API.
 */
PlayersTable::iterator& PlayersTable::iterator::operator++() {
    this->index++;
    return *this;
}

Player& PlayersTable::iterator::operator*() {
    return this->table->players.at( this->index );
}

bool PlayersTable::iterator::operator==( PlayersTable::iterator& other ) {
    return ( this->index == other.index ) && ( this->table == other.table );
}

bool PlayersTable::iterator::operator!=( PlayersTable::iterator& other ) {
    return !( *this == other );
}
