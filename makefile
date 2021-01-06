server:control.o module.o ./cJSON/cJSON.o ./cJSON/cJSON_Utils.o
	gcc -g -o $@ $^ `mysql_config --cflags --libs`
control.o:control.c
	gcc -g -c $< 
module.o:module.c
	gcc -g -c $<