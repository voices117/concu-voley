/* include area */
#include "log.hpp"
#include "ipc.hpp"
#include "match.hpp"
#include "process.hpp"
#include "queue.hpp"
#include "sigint_handler.hpp"
#include <map>
#include <set>
#include <functional>
#include <algorithm>

using std::string;
using std::pair;
using std::map;
using std::set;
using std::function;
using std::endl;
using IPC::Resource;


/** The name of the queue where the results of the matches are read from. */
static const string RESULTS_QUEUE = "/tmp/match_out";

/** Queue to redirect the results. */
static const string REDIRECT_QUEUE = "/tmp/redirect";


static void _scoreboard( SIGINT_Handler* eh ) {
    IPC::Queue<MatchResult> input{ REDIRECT_QUEUE, IPC::QueueMode::read };

    map<player_t, int> scores;
    while( !eh->has_to_quit() ) {
        MatchResult res = input.remove();
        
        /* updates the scores */
        scores[res.match.team1.player1] += res.team1_points;
        scores[res.match.team1.player2] += res.team1_points;

        scores[res.match.team2.player1] += res.team2_points;
        scores[res.match.team2.player2] += res.team2_points;

        /* sorts by highest score */
        typedef function<bool(pair<player_t, int>, pair<player_t, int>)> Comparator;

        Comparator cmp = []( pair<player_t, int> elem1, pair<player_t, int> elem2 ) {
            return elem1.second > elem2.second;
        };

        set<pair<player_t, int>, Comparator> players_set( scores.begin(), scores.end(), cmp );

        /* displays the scores */
        //for( auto entry: players_set ) {
        //    LOG << "- player " << entry.first << ": " << entry.second << " points." << endl;
        //}
        //LOG << "----------" << endl;
    }
}


int main( int argc, const char *argv[] ) {
    int rv = 0;
    Log::get_instance().set_level( Log::Level::debug );

    try {
        LOG_DBG << "begin" << endl;

        /* handles signals */
        SIGINT_Handler eh;
        SignalHandler::get_instance()->add_handler( SIGINT, &eh );
        SignalHandler::get_instance()->add_handler( SIGTERM, &eh );
        SignalHandler::get_instance()->add_handler( SIGPIPE, &eh );

        /* allocates resources */
        Resource<IPC::Queue<MatchResult>> redirect{ REDIRECT_QUEUE };
        
        /* creates the scroreboard process */
        IPC::Process scoreboard( [&eh]() { _scoreboard( &eh ); } );
        
        /* opens the IO queues */
        IPC::Queue<MatchResult> redirect_q{ REDIRECT_QUEUE, IPC::QueueMode::write };
        IPC::Queue<MatchResult> results{ RESULTS_QUEUE, IPC::QueueMode::read };

        while( !eh.has_to_quit() ) {
            MatchResult res = results.remove();
            LOG << "result: " << res << endl;

            redirect_q.insert( res );
        }
        
    } catch( const IPC::QueueError& e ) {
        LOG << "Queue error: " << e.what() << endl;
        rv = 1;
    } catch( const IPC::QueueEOF& e ) {
        /* The results queue was closed, just exits with no error */
    } catch( const IPC::Process::Exit &e ) {
        /* not an error */
    } catch( const IPC::Error& e ) {
        LOG << "IPC error: " << e.what() << endl;
        rv = 2;
    }

    return rv;
}
