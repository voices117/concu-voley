/* include area */
#include "ipc.hpp"
#include "player.hpp"
#include "process.hpp"
#include "str_utils.hpp"
#include <iostream>
#include <string>
#include <exception>

using std::cout;
using std::endl;
using std::size_t;
using std::string;
using IPC::Resource;


#define ASSERT( e ) if( !( e ) ) { throw AssertError{ XSTR( __LINE__ ) ": " #e " is not true" }; }


class AssertError : public std::exception {
public:
    AssertError( const string& message ) : message(message) {}
    ~AssertError() {}

    virtual const char* what() const throw() { return this->message.c_str(); }

private:
    string message;
};


static void _play( PlayersTable* players ) {
    for( size_t i = 4; i < 15; i++ ) {
        players->add_player();
        
        ASSERT( players->size() == i );
    }
    
    ASSERT( players->size() == 14 );
    
    Player p5 = players->get_player( 5 );
    ASSERT( p5.id == 5 );

    Player p10 = players->get_player( 10 );
    ASSERT( p10.id == 10 );

    ASSERT( p5.has_played_with( p10 ) == false );
    ASSERT( p5.num_matches() == 0 );
    ASSERT( p10.num_matches() == 0 );
    
    p5.set_pair( p10 );
    ASSERT( p5.has_played_with( p10 ) == true );
    ASSERT( p10.has_played_with( p5 ) == true );
}


int main( int argc, const char *argv[] ) {
    int rv = 0;
    Log::get_instance().set_level( Log::Level::debug );

    try {
        size_t max_players = 15;
        size_t max_matches = 8;

        Resource<IPC::SharedMem<size_t>> players_mem{ argv[0], max_players * ( max_matches + 2 ) * 2 + 1 };

        IPC::SharedMem<size_t> mem{ argv[0], max_players * ( max_matches + 2 ) * 2 + 1 };
        PlayersTable players{ max_players * 2 + 1, max_matches, mem };

        ASSERT( players.size() == 0 );
        
        players.add_player();
        ASSERT( players.size() == 1 );

        Player p1 = players.get_player( 1 );
        ASSERT( p1.id == 1 );
        ASSERT( p1.get_state() == PlayerState::idle );
        ASSERT( p1.num_matches() == 0 );
        
        players.add_player();
        ASSERT( players.size() == 2 );
        
        Player p2 = players.get_player( 2 );
        ASSERT( p2.id == 2 );
        ASSERT( p2.get_state() == PlayerState::idle );
        ASSERT( p2.num_matches() == 0 );
        
        ASSERT( p1.has_played_with( p2 ) == false );
        
        p1.set_pair( p2 );
        ASSERT( p1.has_played_with( p2 ) == true );
        ASSERT( p2.has_played_with( p1 ) == true );
        ASSERT( p1.num_matches() == 1 );
        ASSERT( p2.num_matches() == 1 );
        
        players.add_player();
        ASSERT( players.size() == 3 );
        Player p3 = players.get_player( 3 );
        ASSERT( p3.id == 3 );
        ASSERT( p3.num_matches() == 0 );
        ASSERT( p3.has_played_with( p1 ) == false );
        
        p3.set_pair( p1 );
        ASSERT( p3.has_played_with( p1 ) == true );
        ASSERT( p3.has_played_with( p2 ) == false );
        ASSERT( p1.has_played_with( p3 ) == true );
        ASSERT( p1.num_matches() == 2 );
        
        /* creates players in another process */
        IPC::Process{ [&players](){ _play( &players ); } };
        
        /* checks that the memory is really shared */
        ASSERT( players.size() == 14 );
        Player p5 = players.get_player( 5 );
        Player p10 = players.get_player( 10 );
        ASSERT( p5.num_matches() == 1 );
        ASSERT( p10.num_matches() == 1 );
        ASSERT( p5.has_played_with( p10 ) );
        
        Player p6 = players.get_player( 6 );
        ASSERT( p6.num_matches() == 0 );
        
        PlayerRO rp7 = players.get_player_ro( 7 );
        ASSERT( rp7.num_matches() == 0 );

        PlayerRO rp10 = players.get_player_ro( 10 );
        ASSERT( rp10.num_matches() == p10.num_matches() );
        ASSERT( rp10.has_played_with( p5 ) == p10.has_played_with( p5 ) );
        ASSERT( rp10.has_played_with( p3 ) == p10.has_played_with( p3 ) );

    } catch( const AssertError& e ) {
        cout << "Assertion error at " << e.what() << endl;
        rv = 1;
    } catch( const IPC::Process::Exit& e ) {
        /* ok */
    } catch( const IPC::Error& e ) {
        cout << "IPC error: " << e.what() << endl;
        rv = 5;
    }

    if( rv == 0 ) {
        LOG << "OK!" << endl;
    }

    return rv;
}
