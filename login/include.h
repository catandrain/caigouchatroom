#ifndef DEBUG
#define DEBUG

#include<gtk/gtk.h>

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

#include "../cJSON/cJSON.h"

#define server_port 9999
#define UDP_PORT 1234

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
    OFFLINE = 0,
    ONLINE
};

enum{
    SEND_MSG = 0,
    SEND_FILE,
    UPDATE_STATE
};

typedef struct ui{
    GtkWidget *loginwindow;

    GtkWidget *signin_btn;
    GtkWidget *login_btn;
    GtkWidget *username_entry;
    GtkWidget *password_entry;

    //下面的是chatroom的，先不要动了
    GtkWidget *chatwindow;
    GtkWidget *vbox;
    GtkWidget *sw;

    GtkWidget *paned;   //分割面板
    GtkWidget *msgvbox;
    GtkWidget *msgvbox_tophbox;
    GtkWidget *msgstack;
    GtkWidget *msgtextview;
    GtkWidget *msgvbox_sendhbox;
    GtkWidget *filechoosebtn;
    GtkWidget *sendtext;
    //GtkWidget *sendbtn;
    GtkWidget *username;
    GtkWidget *list;

    GtkWidget * label;

    GtkTreeSelection *selection;
}UI;

typedef struct json_data{
    char msg[1024];
    char fromip[100];
    int port;
    char from_username[100];
}MSGDATA;

typedef struct user_list{
    char username[100];
    char ip[100];
    int state;
    int port;
    int friendsockfd;
    GtkTextBuffer *buffer;
}USER;

typedef struct m_runtime{
    USER userinfolist[100];
    int friendnum;
    int loginflag;

    char username[100];
    char ip[100];
    int port;
    int udpsockfd;

    int friendid;      //就是USER的id
}MRT;

MRT global_userinfolist;     //最多存储100个用户的数据，也就是最多同时在线100个人。暂时用


GtkWidget* login_or_signin(UI *ui);

GtkWidget* userui(UI *ui);
void add_to_list(GtkWidget *list, const gchar *str);
void recv_msg_thread_func(UI *ui);
void logout(GtkWidget *btn,UI *ui);
#endif // !DEBUG
