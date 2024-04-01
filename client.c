#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ncurses.h>

#define handle_error_en(err, msg) \
	do {errno=err; perror(msg); exit(EXIT_FAILURE);} while(0)

void *receive(void *arg) {
	int *server = (int*)arg;
	char msg[256];

	while (1) {
		int err = recv(*server, msg, 255, 0);

		if (err > 0) {
			msg[256] = '\0';
			printf("%s", msg);
		}

		if (err == 0) {
			break;
		}

		if (err == -1)
			handle_error_en(-1, "recv");
	}

	pthread_exit(0);
}	

int main(int argc, char **argv) {
	

	system("clear");
	pthread_attr_t attr;
	int err = pthread_attr_init(&attr);

	if (err != 0)
		handle_error_en(err, "pthread_attr_init");


	int socket_descr = socket(AF_INET, SOCK_STREAM, 0);

	if (socket_descr == -1) 
		handle_error_en(-1, "socket");

	struct sockaddr_in server_sa;
	server_sa.sin_family = AF_INET;
	server_sa.sin_port = htons(8000);
	server_sa.sin_addr.s_addr = inet_addr("127.0.0.1");
	char user_name[14];
	char txt[240], msg[256];

	printf("Username: ");
	scanf("%s", user_name);
	while (1)
	if (connect(socket_descr,  (struct sockaddr *)&server_sa, sizeof(server_sa)) == -1)
		handle_error_en(-1, "connect");

	system("clear");
	
	pthread_t thread;
	err = pthread_create(&thread, &attr, receive, &socket_descr);
	
	if (err != 0) 
		handle_error_en(err, "pthread_create");

	while (1) {
		fgets(txt, 240, stdin);
		if (strcmp(txt, "/exit") == 0)
			break;

		if (txt[0] != '\n') {
			sprintf(msg, "%s: %s", user_name, txt);
			send(socket_descr, msg, strlen(msg), 0);
		}
	}

	close(socket_descr);

}
