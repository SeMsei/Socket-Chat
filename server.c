#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define LISTEN_BACKLOG 50

#define handle_error_en(en, msg) \
	do {perror(msg); exit(EXIT_FAILURE);} while(0)

struct client_info {
	int *sock_client;
	int *count;
	int client;
};

void *server_receive(void *arg) {
	struct client_info *cinfo = ( struct client_info*)arg;

	char msg[256];

	while (1) {
		ssize_t err = recv(cinfo->client, msg, 255, 0);

		if (err > 0) {
			msg[255] = '\0';

			for (int i = 0; i < *cinfo->count; i++) {
				//if (cinfo->sock_client[i] != cinfo->client)
					err = send(cinfo->sock_client[i], msg, 256, 0);

				if (err == -1) 
					handle_error_en(-1, "send");
			}
		}

		if (err = 0) {
			break;
		}

		if (err == -1) {
			handle_error_en(err, "recv");
		}
	}

	pthread_exit(0);
}

int main(int argc, char **argv) {
	system("clear");
	pthread_attr_t attr;
	int err = pthread_attr_init(&attr);

	if (err != 0)
		handle_error_en(err, "pthread_attr_init");

	int socket_descr = socket(AF_INET, SOCK_STREAM, 0); //IPv4, TCP
	//int socket_descr = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);

	if (socket_descr == -1)
		handle_error_en(socket_descr, "socket(2)");

	struct sockaddr_in server_sa;
	server_sa.sin_family = AF_INET;
	server_sa.sin_port = htons(8000);
	server_sa.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (bind(socket_descr, (struct sockadrr*)&server_sa, sizeof(server_sa)) == -1) {
		handle_error_en(-1, "bind");
		return 1;
	}

	if (listen(socket_descr, LISTEN_BACKLOG) == -1) {
		handle_error_en(-1, "listen");
		return 1;
	}

	int *client = (int*)malloc(sizeof(int));
	int client_count = 0;
	struct client_info **cinfo_arr = (struct client_info**)malloc(sizeof(struct client_info*)*100);
	int client_info_count = 0;

	char *server_addr = inet_ntoa(server_sa.sin_addr);
	in_port_t server_port = server_sa.sin_port;
	printf("Server is ready.\n ip: %s:%d\n", server_addr, server_port);

	while (1) {
		struct sockaddr_in client_sa;
		socklen_t peer_addr_size = sizeof(struct sockaddr_in);
		client[client_count] = accept(socket_descr, (struct sockaddr*)&client_sa, (socklen_t *)&peer_addr_size);
		char * client_addr = inet_ntoa(client_sa.sin_addr);
		in_port_t client_port = client_sa.sin_port;

		if (client[client_count] != -1) {
			printf("New connection: %s:%d\n", client_addr, client_port);
			struct client_info *cinfo = (struct client_info*)malloc(sizeof(struct client_info));
			cinfo->sock_client = client;
			cinfo->count = &client_count;
			cinfo->client = client[client_count];
			cinfo_arr[client_info_count] = cinfo;
			client_info_count++;
			pthread_t thread;
			pthread_create(&thread, &attr, server_receive, cinfo);

			//client_count++;
			client = (int*)realloc(client, sizeof(int) * (++client_count + 1));
		}
	}

	err = pthread_attr_destroy(&attr);

	if (err != 0)
		handle_error_en(err, "pthread_attr_destroy");

	free(client);

	for (int i = 0; i < client_info_count; i++) {
		free(cinfo_arr[i]);
	}

	free(cinfo_arr);

	if (close(socket_descr) == -1)
			handle_error_en(-1, "close");

	return 0;
}
