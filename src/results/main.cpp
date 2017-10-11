/* include area */
#include "argparser.hpp"
#include "log.hpp"
#include "ipc.hpp"
#include "match.hpp"
#include "process.hpp"
#include "queue.hpp"
#include "sigint_handler.hpp"
#include <map>
#include <set>
#include <iomanip>
#include <functional>
#include <algorithm>

using std::string;
using std::pair;
using std::map;
using std::multimap;
using std::set;
using std::function;
using std::endl;
using std::setfill;
using std::setw;
using IPC::Resource;


/** The name of the queue where the results of the matches are read from. */
static const string RESULTS_QUEUE = "/tmp/match_out";

/** Queue to redirect the results. */
static const string REDIRECT_QUEUE = "/tmp/redirect";

/** For 2 teams teamX and teamY, the table gives the points of teamX as sets_to_points[teamY.sets][teamX.sets] */
static const int sets_to_points[][4] = {
    { -1, -1, -1,  3 },
    { -1, -1, -1,  3 },
    { -1, -1, -1,  2 },
    {  0,  0,  1, -1 },
};


static void _scoreboard( SIGINT_Handler* eh ) {
    IPC::Queue<MatchResult> input{ REDIRECT_QUEUE, IPC::QueueMode::read };

    map<player_t, int> scores;
    while( !eh->has_to_quit() ) {
        MatchResult res = input.remove();

        int team1_points = sets_to_points[res.team2_sets][res.team1_sets];
        int team2_points = sets_to_points[res.team1_sets][res.team2_sets];

        if( team1_points < 0 || team2_points < 0 ) {
            LOG_DBG << "Bad points: " << res << endl;
        }

        if( res.status != Status::played ) {
            continue;
        }
        
        /* updates the scores */
        scores[res.match.team1.player1] += team1_points;
        scores[res.match.team1.player2] += team1_points;

        scores[res.match.team2.player1] += team2_points;
        scores[res.match.team2.player2] += team2_points;

        /* descending order multimap */
        multimap<int, player_t, std::greater<int>> players_per_score;
        for( auto e: scores ) {
            players_per_score.insert( pair<int, player_t>( e.second, e.first ) );
        }

        /* displays the scores */
        /* gets the logger object to hols the lock while printing the table */
        LogStream logger = Log::get_instance() << endl
           << "+---- RANKING ----+" << endl
           << "| player | score  |" << endl
           << "+-----------------+" << endl;
        for( auto entry: players_per_score ) {
            logger = logger << "| " << setfill(' ') << setw(6) << entry.second << " | " << setfill(' ') << setw(6) << entry.first << " |" << endl;
        }
        logger << "+-----------------+" << endl;
    }
}


int main( int argc, const char *argv[] ) {
    int rv = 0;
    Log::get_instance().set_level( Log::Level::debug );

    try {
        LOG_DBG << "begin" << endl;

        ArgParser p{ argc, argv };

        auto max_players = p.get_option( "--max-players", size_t );
        auto max_matches = p.get_option( "--max-matches", size_t );

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

        // TODO: filename!!!
        PlayersTable players{ argv[0], max_players * 2, max_matches };

        while( !eh.has_to_quit() ) {
            MatchResult res = results.remove();
            LOG << "result: " << res << endl;

            Player p1_1 = players.get_player( res.match.team1.player1 );
            Player p2_1 = players.get_player( res.match.team1.player2 );
            p1_1.set_state( PlayerState::idle );
            p2_1.set_state( PlayerState::idle );

            Player p1_2 = players.get_player( res.match.team2.player1 );
            Player p2_2 = players.get_player( res.match.team2.player2 );
            p1_2.set_state( PlayerState::idle );
            p2_2.set_state( PlayerState::idle );
                        
            if( res.status == Status::played ) {
                p1_1.set_pair( p2_1 );
                p1_2.set_pair( p2_2 );
                redirect_q.insert( res );
            }
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
