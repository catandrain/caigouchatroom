#include"include.h"

//注册
void signin(GtkWidget *btn,UI *ui){
    const gchar *username , *password;
    username = gtk_entry_get_text(GTK_ENTRY(ui->username_entry));
    password = gtk_entry_get_text(GTK_ENTRY(ui->password_entry));
    //todo:数据打包
    cJSON *send_package = cJSON_CreateObject();
    cJSON_AddNumberToObject(send_package,"type",TYPE_SIGNIN);
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data,"username",username);
    cJSON_AddStringToObject(data,"password",password);
    cJSON_AddItemToObject(send_package,"data",data);
    char *send_str = cJSON_Print(send_package);
    printf("send json:%s\n",send_str);
    //todo:socket连接
    int serverfd;
    struct sockaddr_in sin_addr;
    serverfd = socket(AF_INET,SOCK_STREAM,0);
    if(serverfd<0){
        perror("socket build error");
        return;
    }

    char ip[100] = {"127.0.0.1"};
    sin_addr.sin_family=AF_INET;
    sin_addr.sin_port=htons(9999);
    sin_addr.sin_addr.s_addr=inet_addr(ip);
    //这里不对,改完了
    printf("succeed to set addr\n");

    if(connect(serverfd,(struct sockaddr*)&sin_addr,sizeof(sin_addr))){
        perror("connect");
        return;
    }
    //todo:发送数据

    if(send(serverfd,send_str,strlen(send_str),0)==-1){
        perror("send");
        return;
    }

    //todo:接收数据并根据结果进行处理，返回注册失败或者成功。
    char recv_str[1024];

    if(recv(serverfd,recv_str,sizeof(recv_str),0)==-1){
        perror("receive data");
        close(serverfd);
                //这里先不确定
    }else{
        cJSON *recv_json;
        if(recv_str!=NULL)recv_json = cJSON_Parse(recv_str);
        printf("%s\n",recv_str);

        gchar *message;
        GtkMessageType msgtype;
        cJSON *recv_data_type = cJSON_GetObjectItem(recv_json,"type");
        if(recv_data_type->valueint == TYPE_SIGNIN){
            cJSON *recv_data_data = cJSON_GetObjectItem(recv_json,"data");
            cJSON *recv_data_flag = cJSON_GetObjectItem(recv_data_data,"flag");
            msgtype = GTK_MESSAGE_INFO;
            switch(recv_data_flag->valueint){
                case SUCCESS:message = "注册成功";break;
                default:message = "注册失败";break;
            }
        }
        else{
            //TODO:弹出警告框，消息错误，建议联系管理员
            msgtype = GTK_MESSAGE_ERROR;
            message = "服务器出错，建议联系管理员";
        }
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, 
            msgtype , 
            GTK_BUTTONS_OK, 
            message); 
        gtk_dialog_run(GTK_DIALOG(dialog)); 
        gtk_widget_destroy(dialog); 

        close(serverfd);
    }
}

//登录
void login(GtkWidget *btn,UI *ui){
    const gchar *username , *password;
    username = gtk_entry_get_text(GTK_ENTRY(ui->username_entry));
    password = gtk_entry_get_text(GTK_ENTRY(ui->password_entry));
    //todo:数据打包
    cJSON *send_package = cJSON_CreateObject();
    printf("1\n");
    cJSON_AddNumberToObject(send_package,"type",TYPE_LOGIN);
    printf("2\n");
    cJSON *data = cJSON_CreateObject();
    cJSON_AddStringToObject(data,"username",username);
    cJSON_AddStringToObject(data,"password",password);
    cJSON_AddStringToObject(data,"ip","127.0.0.1");
    printf("3\n");
    char port2[6];
    sprintf(port2,"%d",global_userinfolist.port);
    cJSON_AddStringToObject(data,"port",port2);
    cJSON_AddItemToObject(send_package,"data",data);

    char *send_str = cJSON_Print(send_package);
    printf("send json:%s\n",send_str);
    //todo:socket连接
    int serverfd;
    struct sockaddr_in sin_addr;
    serverfd = socket(AF_INET,SOCK_STREAM,0);
    if(serverfd<0){
        perror("socket build error");
        return;
    }

    char ip[100] = {"127.0.0.1"};
    sin_addr.sin_family=AF_INET;
    sin_addr.sin_port=htons(9999);
    sin_addr.sin_addr.s_addr=inet_addr(ip);
    //这里不对,改完了
    printf("succeed to set addr\n");

    if(connect(serverfd,(struct sockaddr*)&sin_addr,sizeof(sin_addr))){
        perror("connect");
        return;
    }
    //todo:发送数据

    if(send(serverfd,send_str,strlen(send_str),0)==-1){
        perror("send");
        return;
    }

    //todo:接收数据并根据结果进行处理，返回注册失败或者成功。
    char recv_str[1024];

    if(recv(serverfd,recv_str,sizeof(recv_str),0)==-1){
        perror("receive data");
        close(serverfd);
                //这里先不确定
    }else{
        printf("%s\n",recv_str);
        cJSON *recv_json = cJSON_Parse(recv_str);
        if(recv_json==NULL){
            perror("analysis json");
            close(serverfd);
            return;
        }
        cJSON *recv_data = cJSON_GetObjectItem(recv_json,"data");
        cJSON *recv_flag = cJSON_GetObjectItem(recv_json,"flag");

        gchar *message;
        GtkMessageType msgtype;
        msgtype = GTK_MESSAGE_INFO;

        if(cJSON_GetNumberValue(recv_flag)==SUCCESS){
            strcpy(global_userinfolist.username,username);
            //创建聊天界面
            ui->chatwindow = userui(ui);
            //处理收到的好友信息
            cJSON *recv_list = cJSON_GetObjectItem(recv_data,"friendlist");
            message = "登录成功";
            int len = cJSON_GetArraySize(recv_list);

            global_userinfolist.friendnum = len;

            if(len == 0){
                printf("no one online?\n");
            }
            else{
                for(int i=0;i<len;i++){
                    cJSON *frienditem = cJSON_GetArrayItem(recv_list,i);
                    global_userinfolist.userinfolist[i].buffer = gtk_text_buffer_new(NULL);
                    strcpy(global_userinfolist.userinfolist[i].username,cJSON_GetObjectItem(frienditem,"username")->valuestring);
                    strcpy(global_userinfolist.userinfolist[i].ip,cJSON_GetObjectItem(frienditem,"ip")->valuestring);
                    global_userinfolist.userinfolist[i].state = atoi(cJSON_GetObjectItem(frienditem,"state")->valuestring);
                    global_userinfolist.userinfolist[i].port = atoi(cJSON_GetObjectItem(frienditem,"port")->valuestring);
                    add_to_list(ui->list,global_userinfolist.userinfolist[i].username);
                }
            }
            gtk_widget_show_all(GTK_WINDOW(ui->chatwindow));
            //gtk_window_deiconify(GTK_WINDOW(ui->loginwindow));
            global_userinfolist.loginflag = 1;
            gtk_widget_destroy(ui->loginwindow);
            g_thread_create(recv_msg_thread_func,ui,FALSE,NULL);

        }else{
            message = "登录失败";
        }
        GtkWidget *dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT, 
            msgtype , 
            GTK_BUTTONS_OK, 
            message); 
            gtk_dialog_run(GTK_DIALOG(dialog)); 
            gtk_widget_destroy(dialog); 
        close(serverfd);
    }
}

void logout2(GtkWidget *btn){
    if(global_userinfolist.loginflag == 1){
        printf("okokoko\n");
        return;
    }
    else {
        printf("falsefalse\n");
        gtk_main_quit();
    }
}

void logout(GtkWidget *btn,UI *ui){
        const gchar *username ;
        username = global_userinfolist.username;
        //上面以后要改
        cJSON *send_data = cJSON_CreateObject();
        cJSON *data = cJSON_CreateObject();
        cJSON_AddNumberToObject(send_data,"type",TYPE_LOGOUT);
        cJSON_AddItemToObject(send_data,"data",data);
        cJSON_AddStringToObject(data,"username",username);
        char *send_str = cJSON_Print(send_data);
        printf("logout:%s\n",send_str);

        //todo:socket连接
        int serverfd;
        struct sockaddr_in sin_addr;
        serverfd = socket(AF_INET,SOCK_STREAM,0);
        if(serverfd<0){
            perror("socket build error");
            gtk_main_quit();
            return;
        }

        char ip[100] = {"127.0.0.1"};
        sin_addr.sin_family=AF_INET;
        sin_addr.sin_port=htons(9999);
        sin_addr.sin_addr.s_addr=inet_addr(ip);
        //这里不对,改完了
        printf("succeed to set addr\n");

        if(connect(serverfd,(struct sockaddr*)&sin_addr,sizeof(sin_addr))){
            perror("connect");
            gtk_main_quit();
            return;
        }
        //todo:发送数据

        if(send(serverfd,send_str,strlen(send_str),0)==-1){
            perror("send");
            gtk_main_quit();
            return;
        }
    close(serverfd);
    gtk_main_quit();
}

GtkWidget* login_or_signin(UI *ui){
    // GtkWidget *window;

    GtkWidget *username_hbox = gtk_hbox_new(FALSE,0);
    GtkWidget *password_hbox = gtk_hbox_new(FALSE,15);
    GtkWidget *login_btn_hbox = gtk_hbox_new(TRUE,10);
    GtkWidget *login_window_box = gtk_vbox_new(FALSE,5);

    ui->loginwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    gtk_window_set_title(GTK_WINDOW(ui->loginwindow),"聊天室");
    gtk_window_set_default_size(GTK_WINDOW(ui->loginwindow),0,0);
    gtk_container_set_border_width(GTK_CONTAINER(ui->loginwindow),20);

    gtk_container_add(GTK_CONTAINER(login_window_box),username_hbox);
    gtk_container_add(GTK_CONTAINER(login_window_box),password_hbox);
    GtkWidget *sep = gtk_hseparator_new(); 
      gtk_container_add(GTK_CONTAINER(login_window_box),sep);
     gtk_container_add(GTK_CONTAINER(login_window_box),login_btn_hbox);
     gtk_container_add(GTK_CONTAINER(ui->loginwindow),login_window_box);

    ui->login_btn = gtk_button_new_with_label("登录");
    ui->signin_btn = gtk_button_new_with_label("注册");
    gtk_container_add(GTK_CONTAINER(login_btn_hbox),ui->login_btn);
    gtk_container_add(GTK_CONTAINER(login_btn_hbox),ui->signin_btn);

    GtkWidget * username_label, *password_label;
    username_label = gtk_label_new("用户名：");
    ui->username_entry = gtk_entry_new();
    gtk_container_add(GTK_CONTAINER(username_hbox),username_label);
    gtk_container_add(GTK_CONTAINER(username_hbox),ui->username_entry);  

     password_label = gtk_label_new("密码：");
    ui->password_entry = gtk_entry_new();

    gtk_entry_set_visibility(GTK_ENTRY(ui->password_entry),FALSE);
    gtk_container_add(GTK_CONTAINER(password_hbox),password_label);
    gtk_container_add(GTK_CONTAINER(password_hbox),ui->password_entry);  

    g_signal_connect(G_OBJECT(ui->loginwindow),"destroy",G_CALLBACK(logout2),NULL);
    g_signal_connect(G_OBJECT(ui->signin_btn),"clicked",G_CALLBACK(signin),ui);
    g_signal_connect(G_OBJECT(ui->login_btn),"clicked",G_CALLBACK(login),ui);
    return ui->loginwindow;
}