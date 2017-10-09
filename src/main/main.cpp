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
    //Resource<IPC::Barrier> barrier{ MATCH_BARRIER };

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

static void _start_results_processor() {
    /* starts the results processor process */
    pid_t pid = fork();
    if( pid < 0 ) {
        LOG << "Failed creating match simulator" << endl;
        throw IPC::Error( "fork failed: " + static_cast<string>( strerror( errno ) ) );
    } else if( pid == 0 ) {
        char *const env[] = { NULL }; 
        char *const argv[] = { NULL }; 
        execve( "./target/results", argv, env );
        throw IPC::Error( "execve failed: " + static_cast<string>( strerror( errno ) ) );
    }
}


static Team find_team( PlayersTable& players ) {
    for( Player& p1: players ) {
        /* has already played the maximum allowed of matches */
        if( p1.pairs.size() >= 20 || p1.state != PlayerState::idle )
            continue;

        /* finds another player */
        player_t other_id = 0;
        for( player_t id: p1.pairs ) {
            Player& p2 = players.get_player( other_id );
            
            if( other_id < id && p2.state == PlayerState::idle ) {
                p1.state = PlayerState::playing;
                p2.state = PlayerState::playing;
                return Team{ p1.id, p2.id };
            } else {
                other_id++;
            }
        }

        /* no pair available for this player */
        if( other_id >= players.size() ) {
            continue;
        }

        for( player_t id = other_id; id < players.size(); id++ ) {
            Player& p2 = players.get_player( id );
            p1.state = PlayerState::playing;
            p2.state = PlayerState::playing;
            return Team{ p1.id, p2.id };
        }
    }

    throw IPC::Error( "No pairs found" );
}


/**
 * Producer of voley matches.
 */
static void _produce_matches( PlayersTable& players, const string& consumer_name ) {
    IPC::Queue<Match> consumer{ consumer_name, IPC::QueueMode::write };
    
    for( size_t i = 0; i < 11; i++ ) {
        Match m;
        m.team1 = find_team( players );
        m.team2 = find_team( players );

        consumer.insert( m );
    }
}


static void _players_spawner( PlayersTable& players ) {
    for( size_t i = 0; i < 20; i++ ) {
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

        //auto max_players = p.get_option( "--max-players", size_t );
        //auto max_matches = p.get_option( "--max-matches", size_t );
        //auto rows = p.get_option( "--rows", size_t );
        //auto cols = p.get_option( "--cols", size_t );

        /* signal handlers */
        SIGINT_Handler eh;
        SignalHandler::get_instance()->add_handler( SIGINT, &eh );
        SignalHandler::get_instance()->add_handler( SIGPIPE, &eh );
        
        /* creates the IPC resources */
        Resource<IPC::Queue<Match>> match_q{ MATCH_QUEUE };
        Resource<IPC::Queue<MatchResult>> result_q{ RESULTS_QUEUE };

        PlayersTable players{};

        // TODO: include the IO Queue names
        _players_spawner( players );
        _start_match_simulator( argc, argv );
        _start_results_processor();

        _produce_matches( players, MATCH_QUEUE );

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
