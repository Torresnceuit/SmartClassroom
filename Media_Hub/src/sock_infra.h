/*
 * receiveFile_socket.h
 *
 *  Created on: Jul 9, 2014
 *      Author: d-dragon
 */

#ifndef SOCK_INFRA_H_
#define SOCK_INFRA_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ifaddrs.h>
#include <semaphore.h>


/*define and declare server socket*/
#define TCP_PORT 1991
#define UDP_PORT "1992"
#define BROADCAST_DEST_ADDR "255.255.255.255"
#define LOOPBACK_DEFAULT "127.0.0.1"
#define BACKLOG	10
#define SOCK_ERROR -1
#define SOCK_SUCCESS 0

//interface address info
struct ifaddrs *ifaddr, *ifa;
char *interface_addr;

//Declare Stream socket variable
int stream_sock_fd, child_stream_sock_fd;
int ret;
struct sockaddr_in serv_addr;
struct sockaddr_in remote_addr;
socklen_t socklen;
size_t num_byte_read;
char file_buff[102400];

//Declare Datagram socket variable
int datagram_sock_fd;
struct sockaddr_in udp_server_address, udp_client_address;
unsigned int gudp_cli_addr_len;
int udp_byte_read;
int broadcast_enable;

/*Declare socket semaphore
 * UDP socket will start after TCP socket was created
 */
sem_t sem_sock;

//Declare sock function
int openStreamSocket();
int openDatagramSocket();
char *getInterfaceAddress();


#endif /* SOCK_INFRA_H_ */
