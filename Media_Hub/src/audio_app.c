/*
 ============================================================================
 Name        : EnglishTest.c
 Author      : PHAN DUY
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
/*#include "Socket.h"
#include "sock_infra.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

#define SUCCESS 1
#define ERROR 0
#define DEFAULTPATH "/home/root/AudioEnglishTest/"

char *pathtoFile;
FILE *FileReceived;

int createFileStream(char *);

int main() {

	printf("Init Inet socket\n");
	init_TCPNetwork();
	if (openStreamSocket() == SOCK_SUCCESS){
		printf("Open stream socket successed\n");
	}else{
		printf("open stream socket failed\n");
		exit(1);
	}

//	get_Hostname();
//	get_ifaddress();

	char buf[2000000];

	int flag = 0;

	pathtoFile = calloc(100, sizeof(char));

	printf("wait new connection!\n");
	while (1) {

		child_stream_sock_fd = accept(stream_sock_fd, (struct sockaddr *) &remote_addr, &socklen);
		if (child_stream_sock_fd < 0) {
			perror("accept");
			continue;
		}
		printf("server: got connection from %s\n",
				inet_ntoa(remote_addr.sin_addr));

		pid_t pid = fork();
		switch (pid) {
		case 0:
			while (1) {

				numRead = read(child_stream_sock_fd, buf, sizeof buf);
				if (numRead < 0) {
					perror("Receive Error");
				} else if (numRead == 0) {
					printf("client closed\n");
					flag = 0;
					close(child_stream_sock_fd);
					exit(0);
				} else {
//					printf("%d bytes received\n", numRead);
				}
				if (!flag) {
					printf("'File Info: %s\n", buf);

					if (createFileStream_((char*) buf) == SUCCESS) {
						if (send(child_stream_sock_fd, "OK", sizeof("OK"), 0) <= 0) {
							perror("Sending error");
						} else {
							printf("Start receive file!\n");
							flag = 1;
						}
					} else {
						send(child_stream_sock_fd, "AGAIN", sizeof("AGAIN"), 0);
					}
				} else {
					if (numRead == 3) {
						printf("Close file %s\n", buf);
						if (strcmp(buf, "end") == 0) {
							printf("EOF-File Stream closed!\n");
							bzero(buf, sizeof buf);
							memset(pathtoFile,0,sizeof(pathtoFile));
							flag = 0;
							fclose(FileReceived);
						} else {
							printf("File not closed\n");
						}
					}
				}
				if (flag) {
					int szwrite = fwrite(buf, 1, numRead, FileReceived);
//				printf("%d bytes was written\n", szwrite);
					if (szwrite < numRead) {
						perror("File write");
					} else if (szwrite == numRead) {
						printf("Finish receive file session! %d\n", numRead);
						bzero(buf, sizeof buf);
					}

					if ((numRead == 0)) {
						printf("File transfer complete\n");
						flag = 0;
						fclose(FileReceived);
					}

				}
			}
			break;

		}

	}
	return 0;
}

int createFileStream_(char *StrFileInfo) {

	printf("'start createFileStream()\n");
	char *FileName;
//	int FileNameLenght;
	FileName = calloc(50, sizeof(char));
//	printf("cal file lenght\n");
//	FileNameLenght = strlen(StrFileInfo) - strlen(strchr(StrFileInfo, ' '));
	FileName = StrFileInfo;

//	printf("create file name\n");
//	strncpy(FileName, StrFileInfo, FileNameLenght);
	printf("FileName: %s\n", FileName);
	printf("create path to file:%s\n", pathtoFile);
	strcat(pathtoFile, DEFAULTPATH);
	strcat(pathtoFile, FileName);
//	strcat(pathtoFile, ".mp3");
	printf("path: %s\n", pathtoFile);

	printf("create file\n");
	FileReceived = fopen(pathtoFile, "w");
	if (FileReceived == NULL) {
		perror("File Error");
		return ERROR;
	} else {
		printf("File was created successfully\n");
		return SUCCESS;
	}
	free(FileName);

}*/

#include "sock_infra.h"
#include "receive_file.h"
#include "logger.h"
#include <pthread.h>
#include <syslog.h>

#define APP_SUCCESS 1
#define APP_ERROR 0

int main(int argc, char *argv[]){

	pthread_t recv_thread;
	int ret;
	ret = pthread_create(&recv_thread, NULL, recvFileThread(), (void *)"receive file thread!");
	if(ret){
		perror("create thread:");
		syslog(LOG_ERR, "pthread_create %d (failed)\n", ret);
		exit(EXIT_FAILURE);
	}
	pthread_join(recv_thread, NULL);
	return APP_SUCCESS;
}