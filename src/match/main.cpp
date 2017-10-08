/* include area*/
#include "argparser.hpp"
#include "barrier.hpp"
#include "queue.hpp"
#include "functional"
#include "ipc.hpp"
#include "log.hpp"
#include "process.hpp"
#include "signal_handler.hpp"
#include <iostream>
#include <unistd.h>
#include <vector>
#include <wait.h>

using std::size_t;
using std::string;

/* IO FIFOs */
static const string INPUT = "/tmp/match_input";
static const string OUTPUT = "/tmp/match_output";


/**
 * Handler for SIGINT signals.
 */
class SIGINT_Handler : public EventHandler {

private:
    bool quit{ false };

public:

    SIGINT_Handler() {
    }

    ~SIGINT_Handler() {
    }

    virtual int handle_signal( int signum ) override {
        this->quit = true;
        return 0;
    }

    bool has_to_quit() const {
        return this->quit;
    }

};


/**
 * Reads from the input queue and simulates voley matches using the players
 * read from the queue.
 * 
 * \param row The row of the queue.
 * \param eh Event handler for the received signals.
 * \param barrier Initialization barrier for the parent process.
 * \param input Name of the input Queue.
 * \param output Name of the output Queue.
 */
void _consume_matches( int row, SIGINT_Handler& eh, IPC::Barrier& barrier, const string& input, const string& output ) {
    try {
        IPC::Queue<int> in( input, IPC::QueueMode::read );
        IPC::Queue<float> out( output, IPC::QueueMode::write );

        /* signals the parent process that the queues are already open */
        barrier.signal();

        while( !eh.has_to_quit() ) {
            int t = in.remove();

            float rv = 3.15 * t;
            out.insert( rv );
        }
    } catch( IPC::QueueError& e ) {
        LOG << e.what() << std::endl;
    } catch ( IPC::QueueEOF& e ) {}
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
void _create_courts( size_t nrows,
                     size_t ncols,
                     const string& input,
                     const string& output,
                     SIGINT_Handler& eh ) {
    IPC::Barrier barrier{ "./target/match", nrows * ncols };

    /* stores the sub processes in a vector so they are destroyed when this function exits */
    std::vector<IPC::Process> childs;

    /* creates a pool of sub-processes, one for each court */
    for( size_t i = 0; i < nrows; i++ ) {
        for( size_t j = 0; j < ncols; j++ ) {
            auto callback = std::bind( _consume_matches,
                                       i,
                                       eh,
                                       barrier,
                                       input,
                                       output );
            childs.push_back( IPC::Process{ callback } );
        }
    }

    /* creates the queues */
    IPC::Queue<int> tasks( input, IPC::QueueMode::write );
    IPC::Queue<float> results( output, IPC::QueueMode::read );

    /* waits until all processes are intialized */
    barrier.wait();

    /* inserts some data */
    for( size_t i = 0; i < 10; i++ ) {
        tasks.insert( i * 3 );
    }

    /* reads the results */
    for( size_t i = 0; i < 10; i++ )
        ( void )results.remove();
}


int main( int argc, const char *argv[] ) {
    int rv = 0;

    try {
        Log::get_instance().set_level( Log::Level::debug );

        /* parses the options from the command line arguments */
        ArgParser p{ argc, argv };
        auto nrows = p.get_option( "--rows", size_t );
        auto ncols = p.get_option( "--cols", size_t );
        auto input = p.get_optional( "--in", INPUT, string );
        auto output = p.get_optional( "--out", OUTPUT, string );
        
        /* handles signals */
        SIGINT_Handler eh;
        SignalHandler::get_instance()->add_handler( SIGINT, &eh );
        SignalHandler::get_instance()->add_handler( SIGPIPE, &eh );

        /* creates the IPC resources */
        IPC::Resource<IPC::Queue<int>> input_queue{ input };
        IPC::Resource<IPC::Queue<float>> output_queue{ output };
        IPC::Resource<IPC::Barrier> barrier{ argv[0] };
        
        /* creates the processes for the matches */
        _create_courts( nrows, ncols, input, output, eh );
        
    } catch( const ArgParser::Error& e ) {
        LOG << e.what() << std::endl;
        rv = 1;
    } catch( const IPC::QueueError& e ) {
        LOG << "Queue error: " << e.what() << std::endl;
        rv = 2;
    } catch( const IPC::QueueEOF& e ) {
        LOG << "Queue EOF" << std::endl;
        rv = 3;
    } catch( const IPC::Barrier::Error& e) {
        LOG << "Barrier error: " << e.what() << std::endl;
        rv = 4;
    } catch( const IPC::Process::Exit& e ) {
        /* a forked process has finished, this is not an error */
    } catch( const IPC::Error& e ) {
        LOG << "IPC Error: " << e.what() << std::endl;
        rv = 5;
    }

    return rv;
}
