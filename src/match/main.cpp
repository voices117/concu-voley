/* include area*/
#include "argparser.hpp"
#include "barrier.hpp"
#include "queue.hpp"
#include "functional"
#include "ipc.hpp"
#include "log.hpp"
#include "match.hpp"
#include "process.hpp"
#include "sigint_handler.hpp"
#include "utils.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>
#include <wait.h>

using std::size_t;
using std::string;
using std::endl;

/* IO FIFOs */
static const string INPUT = "/tmp/match_in";
static const string OUTPUT = "/tmp/match_out";


/**
 * Returns a random (but valid) result of a voley match.
 * 
 * \return The sets won by each team.
 */
static std::pair<int, int> _rand_result() {
    int result = Utils::rand_int( 1, 4 );

    int team1_sets, team2_sets;
    switch( result ) {
        /* team 1 won 3:0 or 3:1 */
        case 1:
            team1_sets = 3;
            team2_sets = Utils::rand_int( 0, 1 );
            break;
        /* team 2 won 3:0 or 3:1 */
        case 2:
            team1_sets = Utils::rand_int( 0, 1 );
            team2_sets = 3;
            break;
        /* team 1 won 3:2 */
        case 3:
            team1_sets = 3;
            team2_sets = 2;
            break;
        /* team 2 won 3:2 */
        case 4:
            team1_sets = 2;
            team2_sets = 3;
            break;
    }

    return std::pair<int, int>( team1_sets, team2_sets );
}


/**
 * Reads from the input queue and simulates voley matches using the players
 * read from the queue.
 * 
 * \param row The row of the queue.
 * \param eh Event handler for the received signals.
 * \param input Name of the input Queue.
 * \param output Name of the output Queue.
 */
void _consume_matches( int row, SIGINT_Handler& eh, const string& input, const string& output ) {
    IPC::Queue<Match> in( input, IPC::QueueMode::read, true );
    IPC::Queue<MatchResult> out( output, IPC::QueueMode::write, true );

    /* gets the barrier that corresponds to this row */
    IPC::Barrier tide{ IPC::Key{ "./target/main", ( char )( 32 + row ) } };  // TODO: filename!

    while( !eh.has_to_quit() ) {
        try {
            tide.wait();
            
            Match m = in.remove();

            int match_duration = Utils::rand_int( 3, 6 );
            LOG << "Match: " << m << " in row " << row << " taking " << match_duration << " seconds" << endl;
            unsigned int sleep_rv = sleep( match_duration );

            MatchResult r;
            r.match = m;

            /* checks if it was interrupted */
            if( sleep_rv > 0 ) {
                r.status = Status::interrupted;
            } else {
                r.status = Status::played;

                auto team_sets = _rand_result();
                r.team1_sets = team_sets.first;
                r.team2_sets = team_sets.second;
            }

            out.insert( r );
        } catch( IPC::QueueError& e ) {
            LOG << "Queue error: " << e.what() << endl;
        } catch( IPC::Barrier::Error& e ) {
            LOG << "Barrier error: " << e.what() << endl;
            return;
        } catch ( IPC::QueueEOF& e ) {
            /* if the queue was closed, exits */
            return;
        }
    }
}


/**
 * Initializes the courts (child processes) where the matches will be played.
 * 
 * \param nrows Number of rows.
 * \param ncols Number of columns.
 * \param input The name of the input Queue.
 * \param input The name of the output Queue.
 * \param eh Events handler.
 */
void _create_courts( int nrows,
                     int ncols,
                     const string& input,
                     const string& output,
                     SIGINT_Handler& eh ) {
    /* stores the sub processes in a vector so they are destroyed when this function exits */
    std::vector<IPC::Process> childs;

    /* creates a pool of sub-processes, one for each court */
    for( int i = 0; i < nrows; i++ ) {
        for( int j = 0; j < ncols; j++ ) {
            auto callback = std::bind( _consume_matches, i, eh, input, output );
            childs.push_back( IPC::Process{ callback } );
        }
    }
}


int main( int argc, const char *argv[] ) {
    int rv = 0;

    try {
        
        /* parses the options from the command line arguments */
        ArgParser p{ argc, argv };
        auto nrows = p.get_option( "--rows", int );
        auto ncols = p.get_option( "--cols", int );
        auto input = p.get_optional( "--in", INPUT, string );
        auto output = p.get_optional( "--out", OUTPUT, string );
        
        size_t verbosity = p.count( "-v" );
        if( verbosity >= 1 ) {
            Log::get_instance().add_listener( std::cout );
        }
        if( verbosity >= 2 ) {
            Log::get_instance().set_level( Log::Level::debug );
        }

        LOG_DBG << "begin" << endl;

        /* handles signals */
        SIGINT_Handler eh;
        SignalHandler::get_instance()->add_handler( SIGINT, &eh );
        SignalHandler::get_instance()->add_handler( SIGPIPE, &eh );
        SignalHandler::get_instance()->add_handler( SIGCHLD, &eh );
        SignalHandler::get_instance()->add_handler( SIGTERM, &eh );
        
        /* creates the processes for the matches */
        _create_courts( nrows, ncols, input, output, eh );

    } catch( const ArgParser::Error& e ) {
        std::cout << argv[0] << " " << e.what() << endl;
        rv = 1;
    } catch( const IPC::QueueError& e ) {
        LOG << "Queue error: " << e.what() << endl;
        rv = 2;
    } catch( const IPC::QueueEOF& e ) {
        LOG << "Queue EOF" << endl;
        rv = 3;
    } catch( const IPC::Barrier::Error& e) {
        LOG << "Barrier error: " << e.what() << endl;
        rv = 4;
    } catch( const IPC::Process::Exit& e ) {
        /* a forked process has finished, this is not an error */
    } catch( const IPC::Error& e ) {
        LOG << "IPC Error: " << e.what() << endl;
        rv = 5;
    }

    return rv;
}
