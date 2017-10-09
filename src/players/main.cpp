/* include area */
#include "argparser.hpp"
#include "ipc.hpp"
#include "log.hpp"
#include "player.hpp"
#include "sigint_handler.hpp"

using std::endl;
using std::string;
using std::vector;


struct Message {
    int type;
    char payload[1024];
};


int main( int argc, const char *argv[] ) {
    int rv = 0;

    try {
        ArgParser p{ argc, argv };

        /* gets the name of the IO Queues */
        auto input = p.get_option( "--in", string );
        auto output = p.get_option( "--out", string );

        /* signal handlers */
        SIGINT_Handler eh;
        SignalHandler::get_instance()->add_handler( SIGINT, &eh );
        SignalHandler::get_instance()->add_handler( SIGPIPE, &eh );

        vector<Player> players{};

        
        /* handles requests */
        while( !eh.has_to_quit() ) {
            
        }
        
    } catch( const IPC::Error& e ) {
        LOG << "IPC error: " << e.what() << endl;
    }
    
    return rv;
}
