#include"include.h"

int login(cJSON *recv_json,int sockfd,MYSQL *conn){
    cJSON *data = NULL;
    data = cJSON_GetObjectItem(recv_json,"data");
    cJSON *username=NULL,*password = NULL,*ip = NULL,*port = NULL;
    username = cJSON_GetObjectItem(data,"username");
    password = cJSON_GetObjectItem(data,"password");
    ip = cJSON_GetObjectItem(data,"ip");
    port = cJSON_GetObjectItem(data,"port");
    //这个可能是在inaddr中获得的

    USERDATA datalist [100];
    int flag = user_login(conn,datalist,username->valuestring,password->valuestring,ip->valuestring,port->valuestring);
    printf("flag:%d\n",flag);
    cJSON *response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response,"type",TYPE_LOGIN);
    if(flag == 0){
        //登录失败
        cJSON_AddNumberToObject(response,"flag",FAIL);
        cJSON_AddItemToObject(response,"data",NULL);
    }else{
        cJSON_AddNumberToObject(response,"flag",SUCCESS);
        cJSON *friendlist = cJSON_CreateArray();
         cJSON *newdata = cJSON_CreateObject();
        cJSON_AddItemToObject(newdata,"friendlist",friendlist);
        cJSON_AddItemToObject(response,"data",newdata);
        for(int i=0;i<flag;i++){
            cJSON *listitem = cJSON_CreateObject();
            printf("add %d\n",i);
            cJSON_AddStringToObject(listitem,"username",datalist[i].username);
            cJSON_AddStringToObject(listitem,"ip",datalist[i].ip);
            cJSON_AddStringToObject(listitem,"port",datalist[i].port);
            cJSON_AddStringToObject(listitem,"state",datalist[i].state);

            cJSON_AddItemToArray(friendlist,listitem);
            printf("add %d ok\n",i);
            //cJSON_Delete(listitem);     //??可以吗
        }
    }

    char *response_str = cJSON_Print(response);
    printf("%s\n",response_str);
    cJSON_Delete(response);
    if(send(sockfd,response_str,strlen(response_str),0)<0){
        perror("send back signin msg");
        //这里不能用void*，只会发送八位数据，要用char*
    };

    update_state(1,datalist,flag,username->valuestring,ip->valuestring,port->valuestring);

    return 0;
}
/*
*登录函数
*接受数据后，判断是否存在id，如果存在判断密码是否正确。
*如果不正确或者不存在，则返回登录失败，用户不存在或密码错误。（一条就够了）
*如果成功，修改数据库在线状态为在线，并将所有好友信息和个人信息打包发送给用户（包括好友的ip和tcp端口号），用户登录成功。
*然后向所有在线的好友，发送xxx登陆的消息以及他的ip tcpport
*/

int logout(cJSON *recv_json,MYSQL *conn){
    cJSON *data = NULL;
    data = cJSON_GetObjectItem(recv_json,"data");
    cJSON *username=NULL;
    username = cJSON_GetObjectItem(data,"username");
    USERDATA *datalist = (USERDATA*)malloc((100+10)*sizeof(USERDATA));
    int flag = user_logout(conn,datalist,username->valuestring);
    
    update_state(0,datalist,flag,username->valuestring,NULL,NULL);
    return 0;
}
/*
*退出函数，需要的输入有userid,ip等
*接受数据后，修改数据库在线状态为离线
*从数据库中找到所有在线的好友，发送xxx离线的消息
*发送完成后关闭连接
*/

void update_state(int state,USERDATA *datalist,int flag,char *username,char *ip,char *port){
    cJSON *logmsg = cJSON_CreateObject();
    cJSON_AddNumberToObject(logmsg,"type",UPDATE_STATE); 
    cJSON *logdata = cJSON_CreateObject();
    cJSON_AddStringToObject(logdata,"username",username);
    cJSON_AddStringToObject(logdata,"ip",ip);
    cJSON_AddStringToObject(logdata,"port",port);
    cJSON_AddNumberToObject(logdata,"state",state);        //logout
    cJSON_AddItemToObject(logmsg,"data",logdata);
    //告诉好友自己退出的消息
    char *logmsg_str = cJSON_Print(logmsg);
    printf("%s",logmsg_str);

    int sockfd2;
    struct sockaddr_in _addr;
    //建立socket
    sockfd2 = socket(AF_INET,SOCK_DGRAM,0);//UDP协议 
    if(sockfd2<0){
        perror("socket");
        exit(1);
    }
    printf("succeed to build socket2\n");
    //初始化地址
    _addr.sin_family=AF_INET;

    for(int i =0;i<flag;i++){
        if(atoi(datalist[i].state) == 1){
            //好友此时在线
            in_addr_t serv;
            serv = inet_addr(datalist[i].ip);
            _addr.sin_addr.s_addr = serv;
            _addr.sin_port=htons(atoi(datalist[i].port));   //为啥之前port用char*定义的，离谱
            
            printf("ready to send\n");
            if(sendto(sockfd2,logmsg_str,strlen(logmsg_str),0,(struct sockaddr*)&_addr,sizeof(_addr))==-1){
                perror("send logout msg");
                printf("port:%s\tip:%s",datalist[i].port,datalist[i].ip);
            }
            printf("send over\n");
        }
    }
    close(sockfd2);
    return;
}

int signin(cJSON *recv_json,int sockfd,MYSQL *conn){
    cJSON *data = NULL;
    data = cJSON_GetObjectItem(recv_json,"data");

    cJSON *userid,*username,*password;
    // userid = cJSON_GetObjectItem(data,"userid");
    username = cJSON_GetObjectItem(data,"username");
    password = cJSON_GetObjectItem(data,"password");
    printf("data:%s\n",username->valuestring);
    int flag = FAIL;
    flag = user_signin(conn,username->valuestring,password->valuestring);
    //查询数据库，返回结果

    printf("%d\n",flag);

    cJSON *response = cJSON_CreateObject();
    cJSON_AddNumberToObject(response,"type",TYPE_SIGNIN);
    //瞎几把搞的数字
    cJSON *newdata = cJSON_CreateObject();
    //cJSON_AddStringToObject(newdata,"res","注册成功");
     cJSON_AddNumberToObject(newdata,"flag",flag);

     cJSON_AddItemToObject(response,"data",newdata);

     char *response_str = cJSON_Print(response);
     printf("%s\n",response_str);
    if(send(sockfd,response_str,strlen(response_str),0)<0){
        perror("send back signin msg");
    };
    cJSON_Delete(response);
    //cJSON_Delete(newdata);
     return 0;
}
/*
*注册账号函数，需要的输入有userid,password
*首先接受数据后，判断数据库是否有重名id，如果是，返回用户名已存在。
*如果否，写入数据库，返回注册成功。
*/

int main(int argc,char **argv){

    MYSQL conn;
    mysql_init(&conn);
    mysql_real_connect(&conn,"localhost","root","xxxx","xxxx",0,NULL,CLIENT_FOUND_ROWS);//数据库的密码和数据库名

    //服务器main函数
    int sockfd;
    struct sockaddr_in sin_addr,pin_addr;
    //建立socket
    sockfd = socket(AF_INET,SOCK_STREAM,0);
    if(sockfd<0){
        perror("socket");
        exit(1);
    }
    printf("succeed to build socket\n");
    //初始化地址
    sin_addr.sin_family=AF_INET;
    sin_addr.sin_port=htons(9999);
    sin_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    printf("succeed to set addr\n");

    if(bind(sockfd,(struct sockaddr*)&sin_addr,sizeof(sin_addr))){
        perror("bind");
        exit(1);
    }
    printf("bind succeed.\n");

    if(listen(sockfd,10)<0){
        perror("listen error");
        exit(1);
    }
    printf("start to listen client\n");

    int listen_sockfd;
    int addr_size;
    while(1){
        listen_sockfd=accept(sockfd,(struct sockaddr*)&pin_addr,&addr_size);
        if(listen_sockfd<0){
            perror("accept");
            continue;
        }
        else{
            char recv_data[1024];
            memset(&recv_data,0,sizeof(recv_data));
            if(recv(listen_sockfd,&recv_data,sizeof(recv_data),0)==-1){
                perror("receive data");
                close(listen_sockfd);
                //这里先不确定
            }
            else{
                cJSON *recv_json = NULL;
                recv_json = cJSON_Parse(recv_data);
                printf("%s\n",recv_data);
                if(recv_json == NULL){
                    perror("json error");
                    continue;
                }
                cJSON *data_type = cJSON_GetObjectItem(recv_json,"type");
                int type = data_type->valueint;
                printf("type:%d\n",type);
                //假设是从数据中读出来的。T
                switch(type){
                    case TYPE_SIGNIN:signin(recv_json,listen_sockfd,&conn);break;
                    case TYPE_LOGIN:login(recv_json,listen_sockfd,&conn);break;
                    case TYPE_LOGOUT:logout(recv_json,&conn);break;
                    //这里还差一个其他情况，忘了叫啥了
                }
                printf("over\n");
               // cJSON_Delete(data_type);
                cJSON_Delete(recv_json);
                //这个东西会同时删除子项目
                close(listen_sockfd);
                //这个别忘了，别的地儿还有不少错的
            }
        }
    }
    return 0;
}
