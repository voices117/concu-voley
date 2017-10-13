#ifndef LOCK_HPP
#define LOCK_HPP

/* include area */
#include "ipc.hpp"
#include <fcntl.h>
#include <string>
#include <unistd.h>


namespace IPC {

    class Lock {
    public:
        class Error : public IPC::Error {
        public:
            Error( const std::string& message ) : IPC::Error(message) {}
            ~Error() {}
        };

        enum class Mode { read, write };

        Lock( const Lock& other ) = delete;
        Lock( int fd, off_t offset, off_t length, Mode mode );
        Lock( int fd, Mode mode );
        Lock( Lock&& other );
        ~Lock();

        Lock& operator=( Lock& other ) = delete;
        Lock& operator=( const Lock& other ) = delete;
        Lock& operator=( Lock&& other );

    private:
        struct flock fl;
        int fd{ -1 };
    };
}


#endif
