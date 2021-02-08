#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>


#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <strings.h>
#include <string.h>

#define BUFF_SIZE 1024
#define backlog 7
#define ser_port 11277
#define CLI_NUM 3

using namespace std;

int client_fds[CLI_NUM];

int main() {
	int ser_sock_fd;
	char input_message[BUFF_SIZE];
	char recv_message[BUFF_SIZE];
	
	struct sockaddr_in ser_addr;
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons(ser_port);
	ser_addr.sin_addr.s_addr = INADDR_ANY;

	ser_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (ser_sock_fd < 0) {
		cout << "create sock fd failed" << endl;
		return -1;
	}

	int ret = bind(ser_sock_fd, (const struct sockaddr *)&ser_addr, sizeof(ser_addr));
	if (ret < 0) {
		cout << "bind failure:" << ret << endl;
		return -1;
	}

	if (listen(ser_sock_fd, backlog) < 0) {
		cout << "listen failed" << endl;
		return -1;
	}

	fd_set ser_fdset;
	int max_fd = 1;
	struct timeval mytime;
	cout << "wait for client connect" << endl;

	while (true) {
		mytime.tv_sec = 27;
		mytime.tv_usec = 0;

		FD_ZERO(&ser_fdset);
		FD_SET(0, &ser_fdset);

		if (max_fd < 0) {
			max_fd = 0;
		}

		FD_SET(ser_sock_fd, &ser_fdset);
		if (max_fd < ser_sock_fd) {
			max_fd = ser_sock_fd;
		}
		
		for (int i=0;i<CLI_NUM;i++) {
			if (client_fds[i] != 0) {
				FD_SET(client_fds[i], &ser_fdset);
				if (max_fd < client_fds[i]) {
					max_fd = client_fds[i];
				}
			}
		}
		
		int ret = select(max_fd+1, &ser_fdset, NULL, NULL, &mytime);
		cout << "select triggered" << endl;
		if (ret < 0) {
			cout << "select failed" << endl;
			continue;
		} else if (ret == 0) {
			cout << "time out" << endl;
			continue;
		} else {
			if (FD_ISSET(0, &ser_fdset)) {
				cout << "send message" << endl;
				bzero(input_message, BUFF_SIZE);
				fgets(input_message, BUFF_SIZE, stdin);

				for (int i=0;i<CLI_NUM;i++) {
					if (client_fds[i] != 0) {
						cout << "client_fds [" << i << "]=" << client_fds[i] << endl;
						send(client_fds[i], input_message, BUFF_SIZE, 0);
					}
				}
			}
		}

		if (FD_ISSET(ser_sock_fd, &ser_fdset)) {
			struct sockaddr_in client_address;
			socklen_t address_len;
			int client_sock_fd = accept(ser_sock_fd, (struct sockaddr *)&client_address, &address_len);
			if (client_sock_fd > 0) {
				int flags = -1;
				for (int i=0;i<CLI_NUM;i++) {
					if (client_fds[i] == 0) {
						flags = i;
						client_fds[i] = client_sock_fd;
						break;
					}
				}
				if (flags >= 0) {
					cout << "new user client " << flags << endl << "add successfully" << endl;
				} else {
					char full_message[] = "the client is full! cant join \n";
					bzero(input_message, BUFF_SIZE);
					strncpy(input_message, full_message, 100);
					send(client_sock_fd, input_message, BUFF_SIZE, 0);
				}
			}
		}

		for (int i=0;i<CLI_NUM;i++) {
			if (client_fds[i] != 0) {
				if (FD_ISSET(client_fds[i], &ser_fdset)) {
					bzero(recv_message, BUFF_SIZE);
					int byte_num = read(client_fds[i], recv_message, BUFF_SIZE);
					if (byte_num > 0) {
						cout << "message form message[" << i << "]:" << recv_message << endl;
					} else if (byte_num < 0) {
						cout << "resessed error!" << endl;
					} else {
						cout << "cleint[" << i << "] exit" << endl;
						FD_CLR(client_fds[i], &ser_fdset);
						client_fds[i] = 0;
						continue;
					}
				}
			}
		}

	}

	return 0;
}
