/* include area */
#include "log.hpp"
#include "ipc.hpp"
#include "match.hpp"
#include "queue.hpp"
#include "sigint_handler.hpp"

using std::string;
using std::endl;


/** The name of the queue where the results of the matches are read from. */
static const string RESULTS_QUEUE = "/tmp/match_out";


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

        IPC::Queue<MatchResult> results{ RESULTS_QUEUE, IPC::QueueMode::read };

        while( !eh.has_to_quit() ) {
            MatchResult res = results.remove();
            LOG << "result: " << res << endl;
        }
        
    } catch( const IPC::QueueError& e ) {
        LOG << "Queue error: " << e.what() << endl;
        rv = 1;
    } catch( const IPC::QueueEOF& e ) {
        /* The results queue was closed, just exits with no error */
    } catch( const IPC::Error& e ) {
        LOG << "IPC error: " << e.what() << endl;
        rv = 2;
    }

    return rv;
}
