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
 * @file server.h
 * @author Alexander Bezsilko
 * @date 1 Nov 2014
 * @brief File containing functions declarations for
 * working with sockets and epoll.
 */

#ifndef SERVER_H_
#define SERVER_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/epoll.h>

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define DEFAULT_PORT 9000

#define BACKLOG 10

/**
 * @brief Convert address struct.
 *
 * @detailed Convert address struct depends ip version.
 *
 * @param[in] addr Sock address structire pointer.
 */
void* get_addrinfo(const struct sockaddr* addr);

/**
 * @brief Write data to socket.
 *
 * @detailed Send all data from buffer to socket.
 *
 * @param[in] sockfd Socket descriptor.
 * @param[in] data Message buffer.
 * @param[out] len Length of sent data.
 * @return 0 on success, -1 on failure.
 */
int32_t send_data(int32_t sockfd, char* data, uint32_t* len);

/**
 * @brief Create endpoint.
 *
 * @detailed Create listener socket, bind to specified
 * ip and port  and set it to nonblock mode.
 *
 * @param[in] ip Ip address.
 * @param[in] port Port number.
 * @param[out] res Pointer to struct.
 * @return Listener socket descritor, -1 on failure.
 */
int32_t
create_endpoint(const char* ip, const char* port, struct addrinfo** res);

/**
 * @brief Set socket to nonblock mode.
 *
 * @detailed Set socket to nonblock mode.
 *
 * @param[in] sockfd Socket descriptor.
 */
void nonblock(int32_t sockfd);

/**
 * @brief Set socket to listen mode.
 *
 * @detailed Set socket to listen mode.
 *
 * @param[in] ip Socket descriptor.
 * @return Socket descritor, -1 on failure.
 */
int32_t start_listen(int32_t listener);

/**
 * @brief Accept client connection.
 *
 * @detailed Accept client connection.
 *
 * @param[in] ip Socket descriptor.
 * @return Socket descritor, -1 on failure.
 */
int32_t accept_client(int32_t listener);

/**
 * @brief Recieve data from socket.
 *
 * @detailed Recieve data from socket.
 *
 * @param[in] sockfd Socket descriptor for data recieving.
 * @param[in] buf Buffer for recievind data storing.
 * @param[in] length Length of buffer.
 * @return Length of recieved bytes, -1 on failure.
 */
int32_t recv_data(int32_t sockfd, char* buf, int32_t length);

/**
 * @brief Release all resources.
 *
 * @detailed Close descriptors and free all alocated memory.
 *
 * @param[in] equeue Pointer to epoll_event array.
 * @param[in] cnt Count clients in epoll_evet array.
 * @param[in] res Pointer to addrinfo structures array.
 */
void clean(struct epoll_event* equeue, int32_t cnt, struct addrinfo** res);

/* epoll utils */

/**
 * @brief Create new epoll context.
 *
 * @detailed Create new epoll context.
 *
 * @return Epoll descriptor, -1 on failure.
 */
int32_t create_epoll();

/**
 * @brief Add new client for monitoring.
 *
 * @detailed Add new client for monitoring.
 *
 * @param[in] epfd Epoll descriptor.
 * @param[in] sockfd Socket descriptor for monitoring.
 * @return 0 on success, -1 on failure.
 */
int32_t add_client(int32_t epfd, int32_t socketfd);

/**
 * @brief Delete client from monitoring.
 *
 * @detailed Delete client from monitoring.
 *
 * @param[in] epfd Epoll descriptor.
 * @param[in] sockfd Socket descriptor for monitoring.
 * @return 0 on success, -1 on failure. 
 */
int32_t remove_client(int32_t epfd, int32_t socketfd);

#endif /* SERVER_H_ */



















