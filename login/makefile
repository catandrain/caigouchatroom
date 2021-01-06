client:login.o gtk_login.o ../cJSON/cJSON.o ../cJSON/cJSON_Utils.o
	gcc -g -o $@ $^ `pkg-config --cflags --libs gthread-2.0 gtk+-2.0`
login.o:login.c
	gcc -c -g $< `pkg-config --cflags --libs gthread-2.0 gtk+-2.0`

gtk_login.o:gtk_login.c
	gcc -c -g $< `pkg-config --cflags --libs gthread-2.0 gtk+-2.0`