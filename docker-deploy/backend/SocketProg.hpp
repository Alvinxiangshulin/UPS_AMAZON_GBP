#ifndef __SOCKETPROG_HPP__
#define __SOCKETPROG_HPP__

#include <netdb.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

int connectServer(const char * hostname, const char * port)
{
    // initialize host info
    struct addrinfo host_info;
    struct addrinfo * host_info_list;
    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    // get host information
    int status = getaddrinfo(hostname, port, &host_info, &host_info_list);
    if(status != 0) 
    {
        std::cerr << "resolve address error" << std::endl;
        exit(EXIT_FAILURE);
    } 

    // create socket
    int server_fd = socket(host_info_list->ai_family, 
                            host_info_list->ai_socktype,
                            host_info_list->ai_protocol);
    if(server_fd == -1) 
    {
        std::cerr << "socket creation error" << std::endl;
        freeaddrinfo(host_info_list);
        exit(EXIT_FAILURE);
    }

    // connect a socket to a server
    status = connect(server_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if(status == -1) 
    {
        std::cerr << "connect server error" << std::endl;
        freeaddrinfo(host_info_list);
        exit(EXIT_FAILURE);
    } 

    // free address infor list
    freeaddrinfo(host_info_list);

    return server_fd;
}



#endif
