#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define BOARD_SIZE 10
pthread_mutex_t lock;
int quantities[10];

void print_error(const char* msg) {
	perror(msg);
	pthread_exit(NULL);
}

int get_port(char* port_arg) {
	int port = atoi(port_arg);

    if ((port < 1) || (port > 65535)) {
        printf("ERROR: invalid port specified.\n");
        exit(EXIT_FAILURE);
    }
    else {
    	printf("Port no: %d\n", port);
    }
    return port;
}

int get_socket(int port) {
	int socket_fd;
	struct sockaddr_in server;

	if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		print_error("ERROR: ");
		exit(EXIT_FAILURE);
	}

	printf("Socket created\n");
	//isvaloma serverio struktura
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = INADDR_ANY;

	if (bind(socket_fd, (struct sockaddr *) &server, sizeof(server)) == -1) {
		print_error("ERROR: ");
		exit(EXIT_FAILURE);
	}
	printf("Bind successful.\n");
	return socket_fd;
}

void print_menu(char *menu[]) {
	int i;
	printf("________________Menu_______________\n");
	printf("|No_|Product__________ |Quantity__|\n");
	for (i = 0; i < 10; i++) {
		printf("|%2d.|%-18s| %-9d|\n", i+1, menu[i], quantities[i]);
	}
	printf("\n");
}

void load_menu(char *menu[10]) {
	menu[0] = "Cupcakes";
	menu[1] = "Muffins";
	menu[2] = "Ice Cream";
	menu[3] = "Cake slice";
	menu[4] = "Chocolate bar";
	menu[5] = "Candy Floss";
	menu[6] = "Cookies";
	menu[7] = "Apple pie";
	menu[8] = "Dohnuts";
	menu[9] = "Hot Chocolate";
}

void restock() {
	int i;
	for (i=0; i<10;i++) {
		quantities[i] = 10;
	}
}

int buy(int selection) {
	if (quantities[selection-1] != 0) {
		quantities[selection-1] -= 1;
		return 0;
	} 
	else {
		return -1;
	}
}

int recv_int(int client_sockfd) {
    int msg = 0;
    char buffer[1024];
    int n = recv(client_sockfd, buffer, 1024, 0);
    sscanf(buffer, "%d", &msg);
    if (n < 0) {
    	return -1;
    }
    return msg;
}

void *connection_handler(void *socket_fd) {
	int socket = *(int*) socket_fd;
	int purchase;
	int selection;
	int n;
	char *message;
	char *success_message;
	char *purchase_failure_message;
	char *wrong_selection_msg;
	char *purchase_process_msg;
	char *menu[10];
	load_menu(menu);
	success_message = "Your purchase was successful. Please enter the number of the next item you would like to buy.\nIf you would like to quit enter 0\n";
	purchase_failure_message = "We are sorry but this item is out of stock. Please choose a different item.\n";
	wrong_selection_msg = "Please make a valid selection\n";
	purchase_process_msg = "Your purchase is being processed..\n";
    message = "\nWelcome to our online desserts shop! :)\nChoose an item from the menu below and\nenter the number of your selection to make a purchase..\nIf you would like to quit enter 0\n";	

	if ((n = send(socket, message, strlen(message), 0)) == -1) {
			print_error("ERROR: ");
	}

	while((selection = recv_int(socket)) > 0 ) {
    	if (selection < 1 || selection > 10) {
    		send(socket, wrong_selection_msg, strlen(wrong_selection_msg), 0);
    	}
    	//cia turi vykt pirkimas, kad nepridirbtu nesamoniu
    	pthread_mutex_lock(&lock);
    	send(socket, purchase_process_msg, strlen(purchase_process_msg), 0);
    	printf("Selection: %d\n", selection);
    	purchase = buy(selection);
		pthread_mutex_unlock(&lock);
		if (purchase == -1) {
			send(socket, purchase_failure_message, strlen(purchase_failure_message), 0);
			printf("Selected item out of stock\n");
		}
		else {
			send(socket, success_message, strlen(success_message), 0);
		}
		print_menu(menu);
        
    }
    printf("Selection: %d\n", selection);
	if (selection == 0) {
		printf("Client disconnected.\n");
	} 
	else if (selection == -1) {
		print_error("ERROR: recv failed in thread.");
	}

	free(socket_fd);
	pthread_exit(NULL);
}

int main(int argc, char* argv[]) {
	int i;
	int client_length;
	int new_socket, *new_sock;
	struct sockaddr_in client;

	restock();

	if (argc != 2) {
		printf("ERROR: no port provided.\n");
		exit(EXIT_FAILURE);
	}

	int port = get_port(argv[1]);
	int socket_fd = get_socket(port);

	listen(socket_fd, 10);	
	printf("Waiting for connections..\n");

	client_length = sizeof(client);
	pthread_mutex_init(&lock, NULL);
	while (1) {
		new_socket = accept(socket_fd, (struct sockaddr *) &client, (socklen_t *) &client_length);
			if (new_socket == -1) {
			print_error("ERROR: EXITED THREAD CYCLE");
			continue;
		}

		printf("Connection accpeted.\n");

		pthread_t thread;
		new_sock = (int*) malloc(1);
		*new_sock = new_socket;

		if ((pthread_create(&thread, NULL, connection_handler, (void*) new_sock)) < 0) {
			print_error("ERROR: could not create thread. ");
			exit(EXIT_FAILURE);
		}
	}
	close(socket_fd);
	pthread_mutex_destroy(&lock);
	pthread_exit(NULL);
}