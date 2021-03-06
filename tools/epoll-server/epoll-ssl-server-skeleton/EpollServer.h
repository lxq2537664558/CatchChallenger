#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H

#ifdef SERVERNOSSL

#include <sys/socket.h>

#include "BaseClassSwitch.h"

class EpollServer : public BaseClassSwitch
{
public:
    EpollServer();
    ~EpollServer();
    bool tryListen(char *port);
    void close();
    int accept(sockaddr *in_addr,socklen_t *in_len);
    int getSfd();
    Type getType() const;
private:
    int sfd;
};

#endif

#endif // EPOLL_SERVER_H
