/* include area */
#include "signal_handler.hpp"


EventHandler* SignalHandler:: signal_handlers [ NSIG ];

SignalHandler :: SignalHandler () {
}

SignalHandler* SignalHandler:: get_instance () {
    static SignalHandler instance;
    return &instance;
}

void SignalHandler:: destroy () {
}

EventHandler* SignalHandler:: add_handler( int signum, EventHandler* eh ) {
    EventHandler* old_eh = SignalHandler:: signal_handlers [ signum ];
    SignalHandler :: signal_handlers [ signum ] = eh;

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SignalHandler :: dispatcher;
    sigemptyset ( &sa.sa_mask );    // inicializa la mascara de seniales a bloquear durante la ejecucion del handler como vacio
    sigaddset ( &sa.sa_mask,signum );
    sigaction ( signum,&sa,0 ); // cambiar accion de la senial

    return old_eh;
}

void SignalHandler:: dispatcher ( int signum ) {
    if ( SignalHandler :: signal_handlers [ signum ] != 0 )
        SignalHandler :: signal_handlers [ signum ]->handle_signal ( signum );
}

int SignalHandler:: remove_handler( int signum ) {
    SignalHandler :: signal_handlers [ signum ] = NULL;
    return 0;
}
