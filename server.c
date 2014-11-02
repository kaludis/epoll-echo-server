/*
  Copyright (c) 2014 Alexander Bezsilko <demonsboots@gmail.com>

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
 */

/**
 * @file server.c
 * @author Alexander Bezsilko
 * @date 1 Nov 2014
 * @brief File containing functions definition for
 * working with sockets and epoll. 
 */

#include <stddef.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "server.h"

void*
get_addrin(const struct sockaddr* addrin)
{
    assert(addrin != NULL);

    if (addrin->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)addrin)->sin_addr);
    } else {
        return &(((struct sockaddr_in6*)addrin)->sin6_addr);
    }
}

int32_t
send_data(int32_t sockfd, char* data, uint32_t* len)
{
    assert(data != NULL);
    assert(len != NULL);

    uint32_t total, remain;
    total = 0;
    remain = *len;
    int32_t bytes;
    bytes = -1;

    while (total < *len) {
        if ((bytes = send(sockfd, data + total, remain, 0)) == -1) {
            break;
        }
        total += bytes;
        remain -= bytes;
    }

    *len = total;

    return bytes == -1 ? -1 : 0;
}

void
nonblock(int32_t sockfd)
{
    assert(sockfd > 0);

    int32_t yes;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int32_t)) == -1) {
        perror("setsockopt");
    }
}

int32_t
create_endpoint(const char* ip, const char* port, struct addrinfo** res)
{
    int32_t listener;
    struct addrinfo hints;

    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(ip, port, &hints, res) != 0) {
        perror("getaddrinfo");
        return -1;
    }


    listener = socket((*res)->ai_family,
                      (*res)->ai_socktype,
                      (*res)->ai_protocol);

    if (listener == -1) {
        perror("socket");
        freeaddrinfo(*res);
        return -1;
    }

    nonblock(listener);

    if (bind(listener, (*res)->ai_addr, (*res)->ai_addrlen) == -1) {
        perror("bind");
        freeaddrinfo(*res);
        return -1;
    }

    return listener;
}

int32_t
start_listen(int32_t listener)
{
    assert(listener > 0);

    if (listen(listener, BACKLOG) == -1) {
        perror("listen");
        return -1;
    }

    return listener;
}

int32_t
accept_client(int32_t listener)
{
    assert(listener > 0);

    int32_t clientfd;
    struct sockaddr_storage addr;
    socklen_t addr_len;
    
    clientfd = accept(listener, (struct sockaddr*)&addr, &addr_len);
    if (clientfd == -1) {
        perror("accept");
        return -1;
    }

    nonblock(clientfd);

    char ipstr[INET6_ADDRSTRLEN];
    fprintf(stdout, "Client has been connected on %d socket from %s.\n\n",
            clientfd,
            inet_ntop(addr.ss_family,get_addrin((struct sockaddr*)&addr),
                      ipstr, INET6_ADDRSTRLEN));

    return clientfd;
}

int32_t
recv_data(int32_t sockfd, char* buf, int32_t length)
{
    assert(buf != NULL);
    assert(length != 0);

    int32_t bytes;
    bytes = recv(sockfd, (void*)buf, length, 0);

    buf[bytes] = '\0';

    return bytes;
}

void
clean(struct epoll_event* equeue, int32_t cnt, struct addrinfo** res)
{
    (uint32_t)cnt;

    if ((equeue != NULL) && (cnt > 0)) {
        size_t idx;
        for (idx = 0; idx < cnt; ++idx) {
            close(equeue->data.fd);
        }
    }
    free(equeue);
    freeaddrinfo(*res);
}

/* epoll utils */

int32_t
create_epoll()
{
    int32_t epfd;
    epfd = epoll_create1(0);
    if (epfd == -1) {
        perror("epoll_create1");
        return EXIT_FAILURE;
    }

    return epfd;
}

int32_t
add_client(int32_t epfd, int32_t socketfd)
{
    assert(epfd > 0);

    struct epoll_event ev;
    ev.data.fd = socketfd;
    ev.events = EPOLLIN | EPOLLET;

    int32_t ret;
    ret = epoll_ctl(epfd, EPOLL_CTL_ADD, socketfd, &ev);
    if (ret) {
        perror("epoll_ctl");
    }

    return ret;
            
}

int32_t remove_client(int32_t epfd, int32_t socketfd)
{
    assert(epfd > 0);
    assert(socketfd > 0);

    struct epoll_event ev;
    ev.data.fd = socketfd;
    ev.events = EPOLLIN | EPOLLET;

    int32_t ret;
    ret = epoll_ctl(epfd, EPOLL_CTL_DEL, socketfd, &ev);
    if (ret) {
        perror("epoll_ctl");
    }

    return ret;
}

