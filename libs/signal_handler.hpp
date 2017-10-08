#ifndef SIGNAL_HANDLER_HPP
#define SIGNAL_HANDLER_HPP


/* include area */
#include "event_handler.hpp"
#include <signal.h>
#include <stdio.h>
#include <memory.h>


class SignalHandler {

    private:
        static EventHandler* signal_handlers [ NSIG ];

        SignalHandler ( void );
        static void dispatcher ( int signum );

    public:
        static SignalHandler* get_instance();
        static void destroy();
        EventHandler* add_handler( int signum, EventHandler* eh );
        int remove_handler( int signum );

};


#endif
