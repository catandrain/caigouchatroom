#include<mysql/mysql.h>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include<fcntl.h> 
#include<memory.h>
#define myport 9999
#define sendport 10000

#include "./cJSON/cJSON.h"

typedef struct userdata
{
    char state[100];
    //char userid[100];
    char username[100];
    char password[100];
    char ip[100];
    char port[100];
}USERDATA;

enum{
    FAIL,
    SUCCESS
};

enum{
    TYPE_SIGNIN,
    TYPE_LOGIN,
    TYPE_LOGOUT
};
enum{
    SEND_MSG = 0,
    SEND_FILE,
    UPDATE_STATE
};


int user_signin(MYSQL *conn,char *username,char *password);
int user_login(MYSQL *conn,USERDATA *datalist,char *username,char *password,char *ip,char* port);

int user_logout(MYSQL *conn,USERDATA *datalist,char *userid);
int login(cJSON *recv_json,int sockfd,MYSQL *conn);
int logout(cJSON *recv_json,MYSQL *conn);
int signin(cJSON *recv_json,int sockfd,MYSQL *conn);
void update_state(int state,USERDATA *datalist,int flag,char *username,char *ip,char *port);