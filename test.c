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
 * @file test.c
 * @author Alexander Bezsilko
 * @date 1 Nov 2014
 * @brief File containing test code for server.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>

#include "server.h"

#define MIN_PORT 1025
#define MSG_LENGTH 512
#define MAX_CLIENTS 32

const char* cmd_quit = "quit";

int32_t
main_loop(int32_t listener, struct addrinfo* res, struct epoll_event* events, int32_t* cnt);

int
main(int argc, char* argv[])
{
    if (argc != 2) {
        fprintf(stderr, "usage: test port");
        return EXIT_FAILURE;
    } else {
        if (atoi(argv[1]) < MIN_PORT) {
            fprintf(stderr, "please, use port greater then 1024");
            return EXIT_FAILURE;
        }
    }

    struct addrinfo* res;

    int32_t listener = create_endpoint(NULL, argv[1], &res);
    
    if (start_listen(listener) == -1) {
        clean(NULL, 0, &res);
        return EXIT_SUCCESS;
    }


    int32_t cnt = 0;
    struct epoll_event* events;
    events = calloc(MAX_CLIENTS, sizeof(struct epoll_event));

    main_loop(listener, res, events, &cnt);

    clean(events, cnt, &res);

    return EXIT_SUCCESS;
}

int32_t
main_loop(int32_t listener, struct addrinfo* res, struct epoll_event* events, int32_t* cnt)
{
    assert(listener > 0);
    assert(events != NULL);

    int32_t epfd = create_epoll();    

    if (add_client(epfd, STDIN_FILENO) != 0) {
        return EXIT_FAILURE;
    }

    if (add_client(epfd, listener) != 0) {
        return EXIT_FAILURE;
    } 

    while (true) {
        *cnt = epoll_wait(epfd, events, MAX_CLIENTS, -1);

        if (errno == EINTR) {
            continue;
        }
        if (*cnt == -1) {
            perror("epoll_wait");
            return EXIT_FAILURE;
        }

        char msg[MSG_LENGTH];

        int32_t e;
        for (e = 0; e < *cnt; ++e) {
            if ((events[e].events & EPOLLIN) || (events[e].events & EPOLLET)) {

                /* Handling stdin command for normal quit */

                if (events[e].data.fd == STDIN_FILENO) {
                    fgets(msg, MSG_LENGTH, stdin);

                    if (!strncmp(msg, cmd_quit, strlen(cmd_quit))) {
                        return EXIT_SUCCESS;
                    }
                }

                /* Handling clients queries */

                if (events[e].data.fd == listener) {
                    int32_t clientfd;
                    
                    clientfd = accept_client(listener);
                    
                    if (clientfd == -1) {
                        return EXIT_FAILURE;
                    }

                    if (add_client(epfd, clientfd)) {
                        return EXIT_FAILURE;
                    }
                } else {

                    int32_t bytes;
                    
                    bytes = recv_data(events[e].data.fd, msg, MSG_LENGTH);
                    
                    if (bytes == 0) {
                        fprintf(stderr, "Socket %d hang-up\n", events[e].data.fd);
                        remove_client(epfd, events[e].data.fd);
                        continue;
                    } else if (bytes == -1) {
                        fprintf(stderr, "Error on socket %d\n", events[e].data.fd);
                    }

                    if (send_data(events[e].data.fd, msg, &bytes) == -1) {
                        perror("send_data");
                    } else {
                        fprintf(stdout, "%d bytes send to %d socket\n", bytes, events[e].data.fd);
                    }
                } /* if listener or client */
            } /* if event is EPOLLIN */
        } /* for (...) {} events loop */
    } /* while (true) {} main loop */    
}









