#include "argparser.hpp"
#include "barrier.hpp"
#include "queue.hpp"
#include "ipc.hpp"
#include "log.hpp"
#include "match.hpp"
#include "player.hpp"
#include "shared_mem.hpp"
#include "sigint_handler.hpp"
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
using IPC::Resource;
using std::vector;


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


static Team find_team( PlayersTable& players, size_t max_matches ) {
    for( Player p1: players ) {
        /* has already played the maximum allowed of matches */
        if( p1.num_matches() >= max_matches || p1.get_state() != PlayerState::idle )
            continue;

        /* finds another player */
        for( Player p2: players ) {
            if( p2 == p1 || p2.get_state() != PlayerState::idle || p1.has_played_with( p2 ) )
                continue;

            p1.set_state( PlayerState::playing );
            p2.set_state( PlayerState::playing );
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
            m.team1 = find_team( players, players.max_matches );
            m.team2 = find_team( players, players.max_matches );
    
            consumer.insert( m );
        } catch( const IPC::Error& e ) {
            // REMOVE!!!
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


int main( int argc, const char *argv[] )
{
    int rv = 0;
    Log::get_instance().set_level( Log::Level::debug );

    try {
        LOG_DBG << "begin" << endl;
        
        ArgParser p{ argc, argv };

        auto max_players = p.get_option( "--max-players", size_t );
        auto max_matches = p.get_option( "--max-matches", size_t );
        //auto rows = p.get_option( "--rows", size_t );
        //auto cols = p.get_option( "--cols", size_t );

        /* signal handlers */
        SIGINT_Handler eh;
        SignalHandler::get_instance()->add_handler( SIGINT, &eh );
        SignalHandler::get_instance()->add_handler( SIGPIPE, &eh );
        
        /* creates the IPC resources */
        Resource<IPC::Queue<Match>> match_q{ MATCH_QUEUE };
        Resource<IPC::Queue<MatchResult>> result_q{ RESULTS_QUEUE };
        Resource<IPC::SharedMem<size_t>> players_mem{ argv[0], max_players * ( max_matches + 2 ) * 2 + 1 };

        IPC::SharedMem<size_t> mem{ argv[0], max_players * ( max_matches + 2 ) * 2 + 1 };
        PlayersTable players{ max_players * 2 + 1, max_matches, mem };

        // TODO: include the IO Queue names
        _players_spawner( players );
        _start_match_simulator( argc, argv );
        _start_results_processor( argc, argv );

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
    } catch( const IPC::Error& e ) {
        LOG << "IPC error: " << e.what() << endl;
        rv = 3;
    }

    return rv;
}
