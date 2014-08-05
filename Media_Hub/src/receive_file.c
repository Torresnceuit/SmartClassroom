/*
 * receive_file.c
 *
 *  Created on: Jul 12, 2014
 *      Author: d-dragon
 */

#include "receive_file.h"
#include "sock_infra.h"
#include "logger.h"

FILE *createFileStream(char *str_file_name) {

	FILE *tmp_file;
	char *file_name;
	file_name = calloc(50, sizeof(char));
	file_name = str_file_name;

	syslog(LOG_INFO, "FileName: %s\n", file_name);
	syslog(LOG_INFO,"create path to file:%s\n", path_to_file);
	strcat(path_to_file, DEFAULT_PATH);
	strcat(path_to_file, file_name);
	syslog(LOG_INFO, "path: %s\n", path_to_file);

	syslog(LOG_DEBUG, "create file\n");
	tmp_file = fopen(path_to_file, "w");
	if (tmp_file == NULL) {
		syslog(LOG_ERR, "fopen failed");
		return NULL;
	}
	syslog(LOG_DEBUG,"File stream was created successfully\n");
	return tmp_file;
}

void writetoFileStream() {

	int flag_check_mod = 0;
	FILE *file_store_audio;


	while (1) {

		num_byte_read = read(child_stream_sock_fd, file_buff, sizeof file_buff);
		if (num_byte_read < 0) {
			syslog(LOG_ERR,"read() call receive failed!");
		} else if (num_byte_read == 0) {
			syslog(LOG_DEBUG,"client connection closed!");
			flag_check_mod = 0;
			close(child_stream_sock_fd);
			exit(0);
		} else {
			//					printf("%d bytes received\n", numRead);
		}
		if (!flag_check_mod) {
			syslog(LOG_INFO,"File Info: %s\n", file_buff);

			if ((file_store_audio = createFileStream((char*) file_buff)) != NULL) {
				if (send(child_stream_sock_fd, "OK", sizeof("OK"), 0) <= 0) {
					syslog(LOG_ERR,"sending respond to client failed!");
				} else {
					syslog(LOG_DEBUG,"Starting receive file>>>>>>>>>>>");
					flag_check_mod = 1;
				}
			} else {
				send(child_stream_sock_fd, "AGAIN", sizeof("AGAIN"), 0);
			}
		} else {
			if (num_byte_read == 3) {
				syslog(LOG_DEBUG,"Close stream file!");
				if (strcmp(file_buff, "end") == 0) {
					syslog(LOG_DEBUG, "EOF-File Stream closed!\n");
					bzero(file_buff, sizeof file_buff);
					memset(path_to_file, 0, sizeof(path_to_file));
					flag_check_mod = 0;
					fclose(file_store_audio);
				} else {
					syslog(LOG_DEBUG, "File not closed\n");
				}
			}
		}
		if (flag_check_mod) {
			int szwrite = fwrite(file_buff, 1, num_byte_read, file_store_audio);
			//				printf("%d bytes was written\n", szwrite);
			if (szwrite < num_byte_read) {
				syslog(LOG_ERR, "write data to stream file failed!");
			} else if (szwrite == num_byte_read) {
				syslog(LOG_DEBUG,"Finish receive file session! %d\n", num_byte_read);
				bzero(file_buff, sizeof file_buff);
			}

			if ((num_byte_read == 0)) {
				syslog(LOG_DEBUG, "File transfer complete\n");
				flag_check_mod = 0;
				fclose(file_store_audio);
			}

		}
	}

}
void *recvFileThread() {

	path_to_file = calloc(256, sizeof(char));
	int ret = openStreamSocket();
	if(ret == SOCK_SUCCESS){
		syslog(LOG_DEBUG,"TCP socket successfully opened--------\n");
		syslog(LOG_DEBUG,"waiting for new connection!\n");
	}else{
		syslog(LOG_DEBUG,"TCP socket open failed\n");
	}
	while (1) {
		child_stream_sock_fd = accept(stream_sock_fd,
				(struct sockaddr *) &remote_addr, &socklen);
		if (child_stream_sock_fd < 0) {
			syslog(LOG_ERR, "accept call error");
			continue;
		}
		syslog(LOG_INFO, "server: got connection from %s\n",
				inet_ntoa(remote_addr.sin_addr));
		pid_t pid = fork();
		switch (pid) {
		case 0:
			writetoFileStream();
			break;
		default:
			syslog(LOG_DEBUG,"parent process runing from here\n");
			break;
		}
	}

}
