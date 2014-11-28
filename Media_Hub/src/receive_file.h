/*
 * receiveFile.h
 *
 *  Created on: Jul 11, 2014
 *      Author: d-dragon
 */

#ifndef RECEIVE_FILE_H_
#define RECEIVE_FILE_H_



//#define DEFAULT_PATH "/home/pi/MEDIA_HUB/"
#define DEFAULT_PATH "/home/duyphan/git/Smart_Classroom/Media_Hub/List_File/"
#define FILE_PATH_LEN_MAX 200
#define LIST_FILE_MAX 1000
#define FILE_ERROR -1
#define FILE_SUCCESS 0
#define FILE_UNKNOW 2

FILE *g_file_stream;
int g_file_size;
pthread_t g_File_Handler_Thd;

extern pthread_mutex_t g_file_buff_mutex;

typedef struct file{
	int index;
	char *filename;
}FileInfo;


FILE *createFileStream(char *);
void *FileStreamHandlerThread();
void closeFileStream();
int getListFile(char *, char *);

#endif /* RECEIVE_FILE_H_ */
