/*
 * acpHandler.c Application Communication Protocol
 *
 *  Created on: Sep 22, 2014
 *      Author: duyphan
 */

#include "acpHandler.h"
#include "logger.h"
#include "sock_infra.h"
#include "receive_file.h"
#include <endian.h>

/*******************************************************
 * This thread is responsible for receive and accept
 * connection from other side application then fork new
 * process for each connection.
 *******************************************************/
void *waitingConnectionThread() {

	int ret;

	getInterfaceAddress();
	ret = openStreamSocket();
	if (ret == SOCK_SUCCESS) {
		appLog(LOG_DEBUG, "TCP socket successfully opened--------\n");
		appLog(LOG_DEBUG, "waiting for new connection!\n");
	} else {
		appLog(LOG_DEBUG, "TCP socket open failed\n");
	}

	/*start UDP socket for advertise server info*/
	appLog(LOG_DEBUG, "<call sem_post> to active init UDP sock\n");
	sem_post(&sem_sock);

	while (1) {
		child_stream_sock_fd = accept(stream_sock_fd,
				(struct sockaddr *) &remote_addr, &socklen);
		if (child_stream_sock_fd < 0) {
			appLog(LOG_ERR, "accept call error\n");
			exit(0);
			continue;
		}
		appLog(LOG_INFO, "server: got connection from %s\n",
				inet_ntoa(remote_addr.sin_addr));
		pid_t pid = fork();
		switch (pid) {
		case 0:
			recvnhandlePackageLoop();
			break;
		default:
			appLog(LOG_DEBUG, "parent process runing from here\n");
			break;
		}
	}
}

/*******************************************************
 * Function used to waiting - receiving and handling
 * the received package
 *******************************************************/
void recvnhandlePackageLoop() {

	int ret;
	char *PackBuff;
	char *FileBuff;

	g_RecvFileFlag = 0;

	PackBuff = calloc(MAX_PACKAGE_LEN, sizeof(char));
	FileBuff = calloc(MAX_FILE_BUFF_LEN, sizeof(char));

	memset(PackBuff, 0x00, sizeof(PackBuff));
	memset(FileBuff, 0x00, sizeof(FileBuff));

	while (1) {
		if (g_RecvFileFlag != RECV_FILE_ENABLED) {
			memset(PackBuff, 0x00, sizeof(PackBuff));
			num_byte_read = read(child_stream_sock_fd, PackBuff,
					sizeof(PackBuff));
			if (num_byte_read < 0) {
				appLog(LOG_ERR, "read() call receive failed!\n");
			} else if (num_byte_read == 0) {
				appLog(LOG_DEBUG, "client connection closed!\n");
				g_RecvFileFlag = RECV_FILE_DISABLED;
				close(child_stream_sock_fd);
				exit(0);
			} else {
				parsePackageContent(PackBuff);
			}

		} else {
			/*################ write data to file here #####################*/
			memset(FileBuff, 0x00, sizeof(FileBuff));
			num_byte_read = read(child_stream_sock_fd, FileBuff,
					sizeof(FileBuff));
			if (num_byte_read < 0) {
				appLog(LOG_ERR, "read() call receive failed!\n");
			} else if (num_byte_read == 0) {
				appLog(LOG_DEBUG, "client connection closed!\n");
				g_RecvFileFlag = RECV_FILE_DISABLED;
				close(child_stream_sock_fd);
				exit(0);
			} else {
				if (isEOFPackage(FileBuff)) {
				//	closeFileStream();
					g_RecvFileFlag = RECV_FILE_DISABLED;
				}else{
				//	writeDatatoFile(FileBuff);
				}
			}
		}

	}
}

/*******************************************************
 * Function used for parse package content then call
 * corresponding handler
 *******************************************************/
void parsePackageContent(char *packageBuff) {

	char package_header;
	char package_type;
	short int package_len;
//	int cmd;
//	int num_args;
	int ret, i;
	for (i = 0; i < sizeof(packageBuff); i++) {
		appLog(LOG_DEBUG, "%x|", packageBuff[i]);
	}
//check whether package header is valid or not (1st byte)
	package_header = *packageBuff;
	appLog(LOG_DEBUG, "package_header %x\n", package_header);
	if (package_header != PACKAGE_HEADER) {
		appLog(LOG_ERR, "received invalid package!!!\n");
		return;
	}
	packageBuff++;

//analyse package type (2nd byte) and take package length
	package_type = *packageBuff;
	appLog(LOG_DEBUG, "package_type %x", package_type);

	memcpy(&package_len, ++packageBuff, sizeof(package_len));
//convert byte order to little endian
	package_len = be16toh(package_len);
	appLog(LOG_DEBUG, "package_len %d\n", package_len);
	packageBuff = packageBuff + 2;

//parse package type
	switch (package_type) {
	case PACKAGE_CONTROL:
		appLog(LOG_DEBUG, "package type: PACKAGE_CONTROL\n");
		ret = ControlHandler(packageBuff, package_len);
		break;
	case PACKAGE_RESQUEST:
		appLog(LOG_DEBUG, "package type: PACKAGE_REQUEST\n");
		//TODO PACKAGE_REQUEST
		break;
	default:
		appLog(LOG_DEBUG, "package_type invalid\n");
		break;
	}

	return;
}

/************************************************************
 * This handler used for call/process corresponding function
 * for each control command
 ************************************************************/
int ControlHandler(char *ctrlBuff, short int length) {

	appLog(LOG_DEBUG, "inside ControlHandler......\n");
	switch (*ctrlBuff) {
	case CMD_CTRL_PLAY_AUDIO:

		printf("play audio\n");
		break;
	case CMD_SEND_FILE:
		createFileStream(*ctrlBuff);
		break;
	}

	return CTRL_SUCCESS;
}

/*Check if package is EOF control command*/

int isEOFPackage(char *packBuff){

	char *tmpBuff;
	tmpBuff = calloc(5, sizeof(char));

	memcpy(tmpBuff, packBuff, sizeof(tmpBuff));

	if(*tmpBuff != PACKAGE_HEADER){
		return 0;
	}else{
		if(*(++tmpBuff) != PACKAGE_CONTROL){
			return 0;
		}else{
			if(*(tmpBuff+2) != CMD_CTRL_EOF){
				return 0;
			}
		}
	}
	free(tmpBuff);
	return 1;
}

