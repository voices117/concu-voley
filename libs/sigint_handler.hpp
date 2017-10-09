#ifndef SIGINT_HANDLER_HPP
#define SIGINT_HANDLER_HPP

/* include area */
#include "event_handler.hpp"
#include "signal_handler.hpp"


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


#endif
