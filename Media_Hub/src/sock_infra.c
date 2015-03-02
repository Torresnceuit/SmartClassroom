/*
 * receiveFile_socket.c
 *
 *  Created on: Jul 9, 2014
 *      Author: d-dragon
 */
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
#include "sock_infra.h"
#include "logger.h"

/*************************************************
 * open stream socket just for listen connection,
 * not include accept() and communication task
 *************************************************/
int openStreamSocket() {

	/*Open server socket*/
	stream_sock_fd = socket(PF_INET, SOCK_STREAM, 0);
	if (stream_sock_fd < 0) {
		appLog(LOG_ERR, "call socket() error:\n");
		return SOCK_ERROR;
	} else {
		appLog(LOG_DEBUG, "Open Sream socket success\n");
	}

	/* Initialize socket structure */
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_port = htons(TCP_PORT);
	serv_addr.sin_family = PF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	socklen = sizeof(remote_addr);

	/*bind socket address to server socket*/
	ret = bind(stream_sock_fd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr));
	if (ret < 0) {
		appLog(LOG_ERR, "call bind() error!\n");
		return SOCK_ERROR;
	} else {
		appLog(LOG_DEBUG, "bind socket success!\n");
	}
	/* Now start listening for the clients, here
	 * process will go in sleep mode and will wait
	 * for the incoming connection
	 */
	ret = listen(stream_sock_fd, BACKLOG);
	if (ret < 0) {
		appLog(LOG_ERR, "call listen() error\n");
		return SOCK_ERROR;
	} else {
		appLog(LOG_DEBUG, "TCP socket is listening incoming connection at %s\n",
				interface_addr);
	}
	return SOCK_SUCCESS;
}

int openDatagramSocket() {

	int ret;
	broadcast_enable = 1;
	appLog(LOG_INFO, "Create UDP Broadcast socket......\n");

	datagram_sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (datagram_sock_fd < 0) {
		appLog(LOG_ERR, "Open UDP Broadcast socket failed\n");
		return SOCK_ERROR;
	} else {
		appLog(LOG_DEBUG, "Open UDP Broadcast socket success\n");
	}
	ret = setsockopt(datagram_sock_fd, SOL_SOCKET, SO_BROADCAST,
			&broadcast_enable, sizeof(broadcast_enable));
	memset(&udp_server_address, 0, sizeof(udp_server_address));

	//Config UDP Server sock address
	udp_server_address.sin_family = AF_INET;
	udp_server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	udp_server_address.sin_port = htons(atoi(UDP_PORT));
	appLog(LOG_DEBUG, "Config UDP Server sock address success\n");

	//Bind address to UDP server and listen for incoming client connection
	ret = bind(datagram_sock_fd, (struct sockaddr *) &udp_server_address,
			sizeof(struct sockaddr));
	if (ret < 0) {
		appLog(LOG_ERR, "bind address to UDP server socket failed!\n");
		return SOCK_ERROR;
	} else {
		appLog(LOG_DEBUG, "bind address to UDP server socket success!\n");
	}
	return SOCK_SUCCESS;
}

char *getInterfaceAddress() {

	int ret;
	interface_addr = calloc(32, sizeof(char));

	/*get all interface info*/
	if (getifaddrs(&ifaddr) == -1) {
		appLog(LOG_ERR, "get interface address failed\n");
		return NULL;
	}
	/* Walk through linked list, maintaining head pointer so we
	 can free list later */
	for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
		if (ifa->ifa_addr == NULL)
			continue;
		/*just only get AF_NET interface address*/
		if (ifa->ifa_addr->sa_family == AF_INET) {
			ret = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
					interface_addr, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
			if (ret == 0) {
				appLog(LOG_DEBUG, "address of %s: %s\n",
						ifa->ifa_name, interface_addr);
				if (strncmp(LOOPBACK_DEFAULT, interface_addr,
						sizeof(LOOPBACK_DEFAULT)) == 0) {
					continue;
				}
				return interface_addr;
			}
		}

	}
	return NULL;
}

int openMulRecvSocket() {

	mul_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (mul_fd < 0) {
		appLog(LOG_DEBUG, "Creating multicast socket failed\n");
		close(mul_fd);
		return SOCK_ERROR;
	} else {
		appLog(LOG_DEBUG, " open multicast socket success\n");
	}
	int reuse = 1; //multiple applications can use one port on datagram local addr
	if (setsockopt(mul_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuse,
			sizeof(reuse)) < 0) {
		appLog(LOG_DEBUG, "setting REUSEADDR failed\n");
		close(mul_fd);
		return SOCK_ERROR;
	}

	memset(&mul_sock, 0, sizeof(mul_sock));
	mul_sock.sin_family = AF_INET;
	mul_sock.sin_port = htons(5102);
	mul_sock.sin_addr.s_addr = INADDR_ANY;
	if (bind(mul_fd, (struct sockaddr*) &mul_sock, sizeof(mul_sock))) {

		appLog(LOG_DEBUG, "binding mul socket failed\n");
		close(mul_fd);
		return SOCK_ERROR;
	}

	/* join the multicast to group 239.255.1.111 on local address.
	 * Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received.
	 */
	mul_group.imr_multiaddr.s_addr = inet_addr((char *) MULTICAST_ADDR);
	mul_group.imr_interface.s_addr = inet_addr(interface_addr);
	if (setsockopt(mul_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mul_group,
			sizeof(mul_group))) {
		appLog(LOG_DEBUG, "joing multicast group %s on the %s failed\n",
				MULTICAST_ADDR, interface_addr);
		close(mul_fd);
		return SOCK_ERROR;
	} else {
		appLog(LOG_DEBUG, "joined multicast group %s on the %s succes\n",
				MULTICAST_ADDR, interface_addr);
	}
	return SOCK_SUCCESS;
}

/*
 * connect to tcp socket
 * addr: remote address
 * port: connection port
 * return: socket file descriptor
 */
int connecttoStreamSocket(char *addr, char *port) {

	int sd_sock;
	struct sockaddr_in serv_addr;

	sd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sd_sock < 0) {
		appLog(LOG_DEBUG, "open socket failed!");
		return SOCK_ERROR;
	}

	memset(&serv_addr, 0x00, sizeof(struct sockaddr_in));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(port));
	serv_addr.sin_addr.s_addr = inet_addr(addr);

	if (connect(sd_sock, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) < 0) {
		appLog(LOG_DEBUG, "connect to station failed");
		return SOCK_ERROR;
	}
	return sd_sock;
}

