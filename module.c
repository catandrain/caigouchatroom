// #include<mysql/mysql.h>
// #include<stdio.h>
// #include<string.h>

#include"include.h"

int user_signin(MYSQL *conn,char *username,char *password){
    MYSQL_RES *res;
    int len;
    char command[200];
    memset(command,0,sizeof(command));
    sprintf(command,"select user_password from user_basic_information where user_id = \"%s\"", username);
    printf("%s\n",command);
    mysql_query(conn, command);

    res = mysql_store_result(conn);
    len = mysql_num_rows(res);

    if(len != 0){
        //strcpy(s,"用户名已存在");
        printf("注册：用户名已存在\n");
        mysql_free_result(res);     //释放空间
        return FAIL;       //用户名已经存在
    }
    else{
        mysql_free_result(res);     //释放空间
        memset(command,0,sizeof(command));
        sprintf(command, "insert into user_basic_information ( user_id, user_password, user_state) values (\"%s\",\"%s\",\"%s\");", username,password, "0");
        printf("%s\n",command);
        if(mysql_query(conn,command)){
            //strcpy(s,"注册失败");
            printf("注册失败\n");
            return FAIL;
        }      
        else{
            //strcpy(s,"注册成功");
            printf("注册成功\n");
            return SUCCESS;
        }               //注册成功是1，失败是0
    }
}

int user_login(MYSQL *conn,USERDATA *datalist,char *username,char *password,char *ip,char *port){
    MYSQL_RES *res;
    int len;
    char command[200];
    memset(command,0,sizeof(command));
    sprintf(command,"select user_password from user_basic_information where user_id = \"%s\"",username);
    mysql_query(conn,command);

    res = mysql_store_result(conn);
    len = mysql_num_rows(res);

    if(len == 0){
        printf("user not exist\n");
        mysql_free_result(res);
        return 0;
    }else{
        MYSQL_ROW row;
        row = mysql_fetch_row(res);
        if(strcmp(row[0],password)!=0){
            printf("%s",row[0]);
            printf("wrong password\n");
            mysql_free_result(res);
            return 0;
        }else{
            //密码正确，准备登录。
            mysql_free_result(res);
            printf("right password,ready to login\n");
            memset(command,'\0',sizeof(command));
            sprintf(command,"update user_basic_information set user_ip = \"%s\",user_port = %s,user_state = 1 where user_id = \"%s\"",ip,port,username);
            //mysql_query(conn,command);
            if(mysql_query(conn,command)){
                printf("update error");
                return 0;
            }
            printf("update ok");
            //mysql_free_result(res);
            memset(command,0,sizeof(command));
            sprintf(command,"select * from user_basic_information");
            //where user_id = (select friendid from relationtable where userid = \"%s\")",userid);
            mysql_query(conn,command);

            res = mysql_store_result(conn);
            len = mysql_num_rows(res);

            //datalist = (USERDATA*)malloc((len+10)*sizeof(USERDATA));

            MYSQL_ROW friend_row;
            int count = 0;
            while((friend_row = mysql_fetch_row(res))!=NULL){
                //strcpy(datalist[count].userid,friend_row[0]);
                if(friend_row[0]!=NULL)strcpy(datalist[count].username,friend_row[0]);
                if(friend_row[2]!=NULL)strcpy(datalist[count].ip,friend_row[2]);
                if(friend_row[4]!=NULL)strcpy(datalist[count].port,friend_row[4]);
                if(friend_row[3]!=NULL)strcpy(datalist[count].state,friend_row[3]);
                count++;
            }
            mysql_free_result(res);
            return count;   //返回好友列表长度
        }
    }
}


int user_logout(MYSQL *conn,USERDATA *datalist,char *username){
    MYSQL_RES *res;
    int len;
    char command[200];
    memset(command,0,sizeof(command));
    
    sprintf(command,"update user_basic_information set user_state = 0 where user_id = \"%s\"",username);
    mysql_query(conn,command);
    memset(command,0,sizeof(command));

    sprintf(command,"select * from user_basic_information where user_state = 1");
    // where user_id = (select friendid from relationtable where userid = \"%s\"),state = 1",username);
    mysql_query(conn,command);

    res = mysql_store_result(conn);
    len = mysql_num_rows(res);

    MYSQL_ROW friend_row;
    int count = 0;
    while(friend_row = mysql_fetch_row(res)){
        if(friend_row[0]!=NULL)strcpy(datalist[count].username,friend_row[0]);
        if(friend_row[2]!=NULL)strcpy(datalist[count].ip,friend_row[2]);
        if(friend_row[4]!=NULL)strcpy(datalist[count].port,friend_row[4]);
        if(friend_row[3]!=NULL)strcpy(datalist[count].state,friend_row[3]);
        count++;
    }
    mysql_free_result(res);
    return count;   //返回好友列表长度
}