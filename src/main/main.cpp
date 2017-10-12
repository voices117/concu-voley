#include "argparser.hpp"
#include "barrier.hpp"
#include "queue.hpp"
#include "ipc.hpp"
#include "log.hpp"
#include "match.hpp"
#include "player.hpp"
#include "process.hpp"
#include "shared_mem.hpp"
#include "sigint_handler.hpp"
#include "utils.hpp"
#include <errno.h>
#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <random>


using std::string;
using std::size_t;
using std::endl;
using std::vector;
using IPC::Resource;
using IPC::Barrier;
using IPC::Process;


/** The filename of the IPC queue where the teams for a match are sent to play. */
static const string MATCH_QUEUE = "/tmp/match_in";

/** The filename of the IPC queue where the results of a match are sent to. */
static const string RESULTS_QUEUE = "/tmp/match_out";

/** Name of the barrier */
static const string MATCH_BARRIER = "/tmp/match_barrier";

/** Shared memory where the players table is kept. */
static const string PLAYERS_TABLE = "/dev/null";


/**
 * Runs the process that receives the players and simulates the matches.
 * 
 * \param argc Command line arguments count.
 * \param argv Command line arguments arguments list.
 */
static void _start_match_simulator( int argc, const char *const argv[] ) {
    /* starts the match simulator process */
    pid_t pid = fork();
    if( pid < 0 ) {
        LOG << "Failed creating match simulator" << endl;
        throw IPC::Error( "fork failed: " + static_cast<string>( strerror( errno ) ) );
    } else if( pid == 0 ) {
        char *const env[] = { NULL }; 
        execve( "./target/match", ( char *const* )argv, env );
        throw IPC::Error( "execve failed: " + static_cast<string>( strerror( errno ) ) );
    }
}

static void _start_results_processor( int arg, const char *const argv[] ) {
    /* starts the results processor process */
    pid_t pid = fork();
    if( pid < 0 ) {
        LOG << "Failed creating match simulator" << endl;
        throw IPC::Error( "fork failed: " + static_cast<string>( strerror( errno ) ) );
    } else if( pid == 0 ) {
        char *const env[] = { NULL }; 
        execve( "./target/results", ( char *const* )argv, env );
        throw IPC::Error( "execve failed: " + static_cast<string>( strerror( errno ) ) );
    }
}


static Team find_team( PlayersTable& players ) {
    for( PlayerRO p1: players ) {
        /* has already played the maximum allowed of matches */
        if( p1.num_matches() >= players.max_matches || p1.get_state() != PlayerState::idle ) {
            continue;
        }

        /* finds another player */
        for( PlayerRO p2: players ) {
            if( p2 == p1 || p2.get_state() != PlayerState::idle || p1.has_played_with( p2 ) ) {
                continue;
            }
            
            /* gets mutable instances to change the state */
            Player mp1 = players.get_player( p1.id );
            Player mp2 = players.get_player( p2.id );
            mp1.set_state( PlayerState::playing );
            mp2.set_state( PlayerState::playing );
            
            return Team{ p1.id, p2.id };
        }
    }
    
    // TODO: use another exception type
    throw IPC::Error( "No pairs found" );
}


/**
 * Producer of voley matches.
 */
static void _produce_matches( PlayersTable& players,
                              const string& consumer_name,
                              SIGINT_Handler& eh ) {
    IPC::Queue<Match> consumer{ consumer_name, IPC::QueueMode::write };

    LOG_DBG << "start producing matches" << endl;

    while( !eh.has_to_quit() ) {
        try {
            Match m;
            m.team1 = find_team( players );
            m.team2 = find_team( players );
    
            consumer.insert( m );
        } catch( const IPC::Error& e ) {
            // TODO: REMOVE!!!
            LOG << e.what() << endl;
            sleep( 1 );
        }
    }
}


static void _players_spawner( PlayersTable& players ) {
    for( size_t i = 0; i < 10; i++ ) {
        players.add_player();
    }
}


static void _start_tides( int rows, const string& filename, SIGINT_Handler *eh ) {
    int tide = 0;
    vector<Barrier> tides_barriers;

    /* gets the barrier objects */
    for( int c = 32; c < 32 + rows; c++ ) {
        tides_barriers.push_back( Barrier{ IPC::Key{ filename, ( char )c } } );
    }

    /* increases/descreses the tide randomly */
    while( !eh->has_to_quit() ) {
        if( sleep( 4 ) ) {
            continue;
        }

        /* up or down? */
        bool up = true;
        if( Utils::rand_int( 1, 2 ) == 1 ) {
            up = false;
        }

        if( up && tide >= rows - 1 ) {
            up = false;
        } else if( !up && tide == 0 ) {
            up = true;
        }

        if( up ) {
            /* makes the court processes wait until the tide goes down */
            
            LOG << "~~~~~~~~ tide up ~~~~~~~~" << endl;
            tides_barriers[tide].set( 1 );
            tide += 1;
        } else {
            tide -= 1;
            
            /* let's the processes continue */
            tides_barriers[tide].signal();
            LOG << "~~~~~~~ tide down ~~~~~~~" << endl;
        }

    }
}


int main( int argc, const char *argv[] )
{
    int rv = 0;
    Log::get_instance().set_level( Log::Level::debug );

    try {
        LOG_DBG << "begin" << endl;
        
        ArgParser p{ argc, argv };

        auto max_players = p.get_option( "--max-players", size_t );
        auto max_matches = p.get_option( "--max-matches", size_t );
        int rows = p.get_option( "--rows", int );
        //auto cols = p.get_option( "--cols", size_t );

        /* signal handlers */
        SIGINT_Handler eh;
        SignalHandler::get_instance()->add_handler( SIGINT, &eh );
        SignalHandler::get_instance()->add_handler( SIGPIPE, &eh );
        
        /* creates the IPC resources */
        Resource<IPC::Queue<Match>, string> match_q{ MATCH_QUEUE };
        Resource<IPC::Queue<MatchResult>, string> result_q{ RESULTS_QUEUE };
        Resource<PlayersTable> players_res{ argv[0], max_players * 2, max_matches };
        vector<Resource<Barrier>> tides_barriers;

        /* creates a barrier for each row */
        for( int c = 32; c < 32 + rows; c++ ) {
            tides_barriers.push_back( Resource<Barrier>{ IPC::Key{ argv[0], (char)c }, 0 } );
        }

        /* the table has space for 2*M players */
        PlayersTable players{ argv[0], max_players * 2, max_matches };

        // TODO: include the IO Queue names
        _players_spawner( players );
        _start_match_simulator( argc, argv );
        _start_results_processor( argc, argv );

        Process tides_proc{ [rows, argv, &eh](){ _start_tides( rows, argv[0], &eh ); } };

        _produce_matches( players, MATCH_QUEUE, eh );

        // TODO: do better
        do {
            int status;
            wait( &status );
        } while( errno != ECHILD );
        
    } catch( const ArgParser::Error& e ) {
        std::cout << e.what() << endl;
        rv = 1;
    } catch( const IPC::SharedMemError& e ) {
        LOG << "Shared memory error: " << e.what() << endl;
        rv = 2;
    } catch( const IPC::Process::Exit& e ) {
        /* does nothing */
    } catch( const IPC::Error& e ) {
        LOG << "IPC error: " << e.what() << endl;
        rv = 3;
    }

    return rv;
}
