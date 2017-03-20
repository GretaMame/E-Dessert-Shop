#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <stdint.h>

#define BUFFLEN 1024
#define NUMBER_OF_ITEMS 10

void error(const char *msg) {
    printf("%s", msg);
    exit(0);
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

void write_server_int(int sockfd, int msg) {
    char buffer[1024];
    memset(&buffer[0], 0, sizeof(buffer));
    sprintf(buffer,"%d", msg);
    int n = send(sockfd, buffer, 1024, 0);
    if (n < 0) {
        error("ERROR writing int to server socket");
    }
}

void recv_msg(int socket_fd) {
	char buffer[BUFFLEN];
	memset(&buffer[0], 0, sizeof(buffer));
	recv(socket_fd, &buffer, BUFFLEN, 0);
    printf("%s\n", buffer);
}

int get_selection(int socket_fd) {
	int selection;
	while(1) {
		printf("Enter your selection: ");
		scanf("%d", &selection);
		if (selection == 0) {
			write_server_int(socket_fd, selection);
			return selection;
		}
		if (selection > 0 && selection < NUMBER_OF_ITEMS+1) {
			printf("\n");
         	write_server_int(socket_fd, selection);
         	return selection;
		}
		else {
			printf("Wrong selection. Please try again..\n");
		}
	}
}

void print_menu(char *menu[]) {
	int i;
	printf("_____________Menu_____________\n");
	printf("|No_|Product_________________|\n");
	for (i = 0; i < NUMBER_OF_ITEMS; i++) {
		printf("|%2d.|%-24s|\n", i+1, menu[i]);
	}
	printf("\n");
}

void load_menu(char *menu[NUMBER_OF_ITEMS]) {
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

int connect_to_server(char *host_name, int port) {
    struct sockaddr_in server_adr;
    struct hostent *server; //This data type is used to represent an entry in the hosts database.
    int socket_fd;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        error("ERROR opening socket for server.");
    }

    memset(&server_adr, 0, sizeof(server_adr));
    server_adr.sin_family = AF_INET;
    server_adr.sin_port = htons(port);
    server = gethostbyname(host_name);//The gethostbyname function returns information about the host named name. If the lookup fails, it returns a null pointer.
    if (server == NULL) {
        printf("ERROR, no such host\n");
        exit(0);
    }

    memmove(server->h_addr, &server_adr.sin_addr.s_addr, server->h_length); //is pirmos i antra tiek kiek nurodyta treciam

    if (connect(socket_fd, (struct sockaddr *) &server_adr, sizeof(server_adr)) < 0) {
        error("ERROR connecting to server\n");
    }
    return socket_fd;
}

int main(int argc, char *argv[]) {
	char *menu[NUMBER_OF_ITEMS];
	int selection;

	if (argc != 3) {
        printf("USAGE: %s <ip> <port>\n",argv[0]);
        exit(1);
    }

    int port = get_port(argv[2]);
    int socket_fd = connect_to_server(argv[1], port);
    recv_msg(socket_fd);
    load_menu(menu);
    print_menu(menu);
    while (1) {
    	if ((get_selection(socket_fd)) == 0)
    	{
    		break;
    	}
    	//printf("I am trying to receive a msg.\n");
    	recv_msg(socket_fd);
    }
    
    close(socket_fd);
	return 0;
}