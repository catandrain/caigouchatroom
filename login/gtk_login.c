#include"include.h"


//接受消息线程
void recv_msg_thread_func(UI *ui){
    int socketfd;
    socketfd = socket(AF_INET,SOCK_DGRAM,0);
    if(socketfd<0){
        perror("build socket");
        return 0;
    }
    struct sockaddr_in sin_addr;

    sin_addr.sin_family = AF_INET;
    sin_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sin_addr.sin_port = htons(global_userinfolist.port);

    if(bind(socketfd,(struct sockaddr *)&sin_addr,sizeof(sin_addr))==-1){
        perror("bind");
        return 0;
    }

    char buf[1024];
    memset(buf,0,sizeof(buf));

    int len;

    while(1){
        if(recvfrom(socketfd,buf,sizeof(buf),0,(struct sockaddr*)&sin_addr,&len)!=-1){
            gdk_threads_enter();
            printf("recv udp data%s\n",buf);
            msg_classfy(ui,buf);
            gdk_threads_leave();
            memset(buf,0,sizeof(buf));
            continue;
        }else perror("recv fail");
    }
    close(socketfd);
}

void msg_classfy(UI *ui,char *msg){
    cJSON *recv_data = cJSON_Parse(msg);
    cJSON *data = cJSON_GetObjectItem(recv_data,"data");
    cJSON *type = cJSON_GetObjectItem(recv_data,"type");
    int _type = cJSON_GetNumberValue(type);
    switch(_type){
        case SEND_MSG:insert_into_textview(ui,data);break;
        case SEND_FILE:break;
        case UPDATE_STATE:update_state(data);break;
        default:break;
    }
    cJSON_Delete(recv_data);
}

void update_state(cJSON *data){
    cJSON *state = cJSON_GetObjectItem(data,"state");
    int _state = cJSON_GetNumberValue(state);
    printf("state:%d\n",_state);
    cJSON *username = cJSON_GetObjectItem(data,"username");
    gchar *ip;
    int port;
    if(_state == 1){

        ip = cJSON_GetObjectItem(data,"ip")->valuestring;
        port = atoi(cJSON_GetObjectItem(data,"port")->valuestring);
    }else{
        ip = "";
        port = 0;
    }
    printf("user online:%s+%s+%d\n",username->valuestring,ip,port);
    int len = global_userinfolist.friendnum;
    for(int i=0;i<len;i++){
        if(strcmp(username->valuestring,global_userinfolist.userinfolist[i].username)==0){
            global_userinfolist.userinfolist[i].state = _state;
            global_userinfolist.userinfolist[i].port = port;
            strcpy(global_userinfolist.userinfolist[i].ip,ip);
            break;
        }
    }
    return;
}

void insert_into_textview(UI *ui,cJSON *json_data){
    GtkTextIter start,end;
    cJSON *username = cJSON_GetObjectItem(json_data,"username");
    cJSON *msg = cJSON_GetObjectItem(json_data,"msg");
    char s[1024],name[100];

    sprintf(s,"%s\n\n",msg->valuestring);
    g_print("recv_msg:%s",s);

    GtkTextBuffer *buffer;
    for(int i=0;i<global_userinfolist.friendnum;i++){
        if(strcmp(username->valuestring,global_userinfolist.userinfolist[i].username)==0){
            buffer = global_userinfolist.userinfolist[i].buffer;
            break;
        }
    }

    sprintf(name,"%s:\n",username->valuestring);

    gtk_text_buffer_get_end_iter(buffer,&end); 

    gtk_text_buffer_insert(buffer,&end,name,strlen(name));
    gtk_text_buffer_get_end_iter(buffer,&end); 
    gtk_text_buffer_insert(buffer,&end,s,strlen(s));
    
}



void msend_msg(GtkWidget *btn,UI* ui){
    const gchar* s;
    s = gtk_entry_get_text(GTK_ENTRY(ui->sendtext));

    printf("长度：%d",strlen(s));
    cJSON *send_data = cJSON_CreateObject();
    cJSON_AddNumberToObject(send_data,"type",SEND_MSG);
    cJSON *data = cJSON_CreateObject();
    cJSON_AddItemToObject(send_data,"data",data);
    cJSON_AddStringToObject(data,"username",global_userinfolist.username);
    cJSON_AddStringToObject(data,"ip",global_userinfolist.ip);
    cJSON_AddStringToObject(data,"msg",s);
    cJSON_AddNumberToObject(data,"port",global_userinfolist.port);
    char *send_str = cJSON_Print(send_data);
    cJSON_Delete(send_data);
    g_print("send_data:%s\n",send_str);

    int sendsocket;
    struct sockaddr_in sin_addr;

    int i =global_userinfolist.friendid;

    sendsocket = socket(AF_INET,SOCK_DGRAM,0);
    if(sendsocket<0){
        perror("sendsocket");
        exit(1);
    }

    printf("ip+port：%s+%d\n",global_userinfolist.userinfolist[i].ip,global_userinfolist.userinfolist[i].port);
    sin_addr.sin_family = AF_INET;
    sin_addr.sin_addr.s_addr = inet_addr(global_userinfolist.userinfolist[i].ip);//htonl(INADDR_ANY);
    sin_addr.sin_port = htons(global_userinfolist.userinfolist[i].port);
    if(sendto(sendsocket,send_str,strlen(send_str),0,(struct sockaddr*)&sin_addr,sizeof(sin_addr))==-1){
        perror("error send socket");
    }
    close(sendsocket);
}

void on_change(GtkWidget *widget,UI *ui){
    GtkTreeIter iter;
    GtkTreeModel *model;
    char *value;

    if(gtk_tree_selection_get_selected(
        GTK_TREE_SELECTION(widget),&model,&iter)){
            gtk_tree_model_get(model, &iter, 0, &value,  -1);
            
            gchar *statestr;
            int i;
            for(i=0;i<global_userinfolist.friendnum;i++){
                if(strcmp(global_userinfolist.userinfolist[i].username,value)==0){
                    printf("username:%s\n",value);
                    global_userinfolist.friendid = i;
                    // gtk_text_view_set_buffer()
                    statestr = (global_userinfolist.userinfolist[i].state==0)?"(离线)":"(在线)";
                     printf("state:%s %d\n",statestr,global_userinfolist.userinfolist[i].state);
                    gtk_label_set_text(GTK_LABEL(ui->label),strcat(value,statestr));
                    g_print(global_userinfolist.userinfolist[i].username);
                    g_free(value);

                    gtk_text_view_set_buffer(ui->msgtextview,global_userinfolist.userinfolist[i].buffer);
                    //设置新的buffer

                    return;
                }
            }
            g_free(value);
        }
}

void init_list(GtkWidget *list){
    GtkCellRenderer *render;
    GtkTreeViewColumn *column;
    GtkListStore *store;

    render = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("List Items",render,"text",0,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(list),column);
    store = gtk_list_store_new(1, G_TYPE_STRING);
    
    gtk_tree_view_set_model(GTK_TREE_VIEW(list), 
    GTK_TREE_MODEL(store));

    g_object_unref(store);
}

//向列表中添加新项目
void add_to_list(GtkWidget *list, const gchar *str)
{
  GtkListStore *store;
  GtkTreeIter iter;

  store = GTK_LIST_STORE(gtk_tree_view_get_model
      (GTK_TREE_VIEW(list)));

  gtk_list_store_append(store, &iter);
  gtk_list_store_set(store, &iter, 0, str, -1);
}

GtkWidget* userui(UI *ui){

    //这个是不是要放到别的地方，比如创建窗口的地方

    GtkWidget *window;
    GtkWidget *sendbtn;
    int n=0;
    printf("%d",n++);
    
    //主窗口设置
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window),"chatroomlist");
    gtk_window_set_default_size(GTK_WINDOW(window),1000,700);
    gtk_container_set_border_width(GTK_CONTAINER(window),0);


    ui->vbox = gtk_vbox_new(FALSE,0);

    //状态栏设置
    GtkWidget *headpaned = gtk_hpaned_new();
    GtkWidget *userheadbar, *chatroomheadbar;

    gtk_paned_set_position(GTK_PANED(headpaned),300);

    //分割窗口，左侧
    ui->paned = gtk_hpaned_new();
    gtk_paned_set_position(GTK_PANED(ui->paned),300);
    gtk_paned_add1(GTK_PANED(ui->paned),ui->vbox);

    gtk_container_add(GTK_CONTAINER(window),ui->paned);


    //label设置
    ui->label = gtk_label_new("friendname");

    //左侧分割线和用户名
    ui->username = gtk_label_new(global_userinfolist.username);
    GtkWidget *sp1 = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(ui->vbox),ui->username,0,0,0);
    
    gtk_box_pack_start(GTK_BOX(ui->vbox),sp1,0,0,0);

    //滑动窗口设置
    ui->sw = gtk_scrolled_window_new(NULL,NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(ui->sw),
        GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW(ui->sw),
        GTK_SHADOW_ETCHED_IN);
    gtk_container_add(GTK_CONTAINER(ui->vbox),ui->sw);

    //list设置
    ui->list = gtk_tree_view_new();
    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(ui->list),FALSE);

    init_list(ui->list);

    ui->selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(ui->list));

   gtk_container_add(GTK_CONTAINER(ui->sw),ui->list);

    
    //分割窗口右侧
    ui->msgvbox = gtk_vbox_new(FALSE,0);
    gtk_paned_add2(GTK_PANED(ui->paned),ui->msgvbox);
    
    //msgbox顶层box
    ui->msgvbox_tophbox = gtk_hbox_new(FALSE,0);
    
    gtk_box_pack_start(GTK_BOX(ui->msgvbox_tophbox),ui->label,1,0,0);

    
    //中间消息窗口部分
    ui->msgtextview = gtk_text_view_new();
    //todo：完善啊

    //发送部分
    ui->msgvbox_sendhbox = gtk_hbox_new(FALSE,0);
    
     ui->filechoosebtn = gtk_file_chooser_button_new("发送文件",GTK_FILE_CHOOSER_ACTION_OPEN);
    ui->sendtext = gtk_entry_new();
    sendbtn = gtk_button_new_with_label("发送消息");
    gtk_box_pack_start(GTK_BOX(ui->msgvbox_sendhbox),ui->filechoosebtn,0,0,0);
    gtk_box_pack_start(GTK_BOX(ui->msgvbox_sendhbox),ui->sendtext,1,1,0);
    gtk_box_pack_start(GTK_BOX(ui->msgvbox_sendhbox),sendbtn,0,0,0);

    gtk_text_view_set_editable(GTK_TEXT_VIEW(ui->msgtextview),FALSE);
    gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(ui->msgtextview),FALSE);

    GtkWidget *sp2 = gtk_hseparator_new();
    GtkWidget *sp3 = gtk_hseparator_new();
    
    gtk_box_pack_start(GTK_BOX(ui->msgvbox),ui->msgvbox_tophbox,0,0,0);
    gtk_box_pack_start(GTK_BOX(ui->msgvbox),sp2,0,0,0);
    gtk_box_pack_start(GTK_BOX(ui->msgvbox),ui->msgtextview,1,1,0);
    gtk_box_pack_start(GTK_BOX(ui->msgvbox),sp3,0,0,0);
    gtk_box_pack_start(GTK_BOX(ui->msgvbox),ui->msgvbox_sendhbox,0,0,0);

    //信号设置
    g_signal_connect(G_OBJECT (window), "destroy",
    G_CALLBACK(logout), ui);
    g_signal_connect(ui->selection,"changed",G_CALLBACK(on_change),ui);
    //发送消息
    g_signal_connect(G_OBJECT(sendbtn),"clicked",G_CALLBACK(msend_msg),ui);

    return window;
}

int main(int argc,char **argv){
    GtkWidget *loginwindow;

    UI ui;

    g_print("my port：\n");
    scanf("%d",&global_userinfolist.port);

    if(!g_thread_supported()){
        g_thread_init(NULL);
    }

    gtk_init(&argc,&argv);

    loginwindow = login_or_signin(&ui);

    int res;

    gtk_widget_show_all(loginwindow);

    gdk_threads_enter();
    gtk_main();
    gdk_threads_leave();

    return 0;
}