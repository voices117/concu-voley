#ifndef EVENT_HANDLER_HPP
#define EVENT_HANDLER_HPP


class EventHandler {

public:
    virtual int handle_signal( int signum ) = 0;
};


#endif
