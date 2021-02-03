#include <iostream>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>

#define BUFF_SIZE 1024

using namespace std;

int main() {
	int connfd, sockfd, n;
	struct epoll_event ev, events[20];
	int epfd = epoll_create(256);

	struct sockaddr_in client_addr;
	struct sockaddr_in server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(11277);
	server_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server_addr.sin_zero), 8);

	int server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	ev.data.fd = server_sock_fd;
	ev.events = EPOLLIN|EPOLLET;
	epoll_ctl(epfd, EPOLL_CTL_ADD, server_sock_fd, &ev);

	if (server_sock_fd == -1) {
		cout << "socket error" << endl;
		return -1;
	}

	char recv_msg[BUFF_SIZE];
	char input_msg[BUFF_SIZE];

	if (connect(server_sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) == 0) {
		while (true) {
			int nfds = epoll_wait(epfd, events, 20, 500);
			for (int i=0;i<nfds;++i) {
				cout << "nfds:" << events[i].events << endl;
				if (events[i].events & EPOLLOUT) {
					bzero(input_msg, BUFF_SIZE);
					fgets(input_msg, BUFF_SIZE, stdin);

					cout << input_msg << endl;

					sockfd = events[i].data.fd;
					write(sockfd, input_msg, BUFF_SIZE);

					ev.data.fd = sockfd;
					ev.events = EPOLLIN|EPOLLET;
					epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
				} else if (events[i].events & EPOLLIN) {
					bzero(recv_msg, BUFF_SIZE);
					if ( (n = read(server_sock_fd, recv_msg, BUFF_SIZE)) < 0) {
						cout << "read error" << endl;
					}
					ev.data.fd = server_sock_fd;
					ev.events = EPOLLOUT|EPOLLET;
					cout << "recv_msg:" << recv_msg << endl;
				}
			}
		}
	}

	return 0;

}
