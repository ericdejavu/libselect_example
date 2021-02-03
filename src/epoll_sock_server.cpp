#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>

using namespace std;

#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1

#define SERVER_PORT	11277
#define EPOLL_EVENTS	100
#define FDSIZE		1000
#define BUFF_SIZE	4096

static void _event(int epollfd, int fd, int state, int action) {
	struct epoll_event ev;
	ev.events = state;
	ev.data.fd = fd;
	epoll_ctl(epollfd, action, fd, &ev);
}

int main() {
	int serverfd;
	if ((serverfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
		cout << "create socket error" << endl;
		return -1;
	}

	sockaddr_in server_addr;
	memset(&server_addr, 0, sizeof(sockaddr_in));	
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(SERVER_PORT);

	if (bind(serverfd, (sockaddr *)&server_addr, sizeof(sockaddr)) == SOCKET_ERROR) {
		cout << "socket bind failed" << endl;
		close(serverfd);
		return -1;
	}

	if (listen(serverfd, 5) < 0) {
		cout << "listen error" << endl;
		return -1;
	}

	cout << "start listen .." << endl;
	int epollfd;
	struct epoll_event events[EPOLL_EVENTS];
	int fd_num;
	epollfd = epoll_create(FDSIZE);

	_event(epollfd, serverfd, EPOLLIN, EPOLL_CTL_ADD);

	char buffer[BUFF_SIZE + 1];

	while (true) {
		fd_num = epoll_wait(epollfd, events, EPOLL_EVENTS, -1);

		int fd;
		for (int i=0;i<fd_num;i++) {
			fd = events[i].data.fd;
			if (fd == serverfd && (events[i].events & EPOLLIN)) {
				int clientfd;
				struct sockaddr_in client_addr;
				socklen_t client_addrlen;
				
				if ((clientfd = accept(serverfd, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) {
					cout << "accept error" << endl;
				} else {
					cout << "accept a new client:" << inet_ntoa(client_addr.sin_addr) << ":" << client_addr.sin_port << endl;
					_event(epollfd, serverfd, EPOLLIN, EPOLL_CTL_ADD);
				}
			} else if (events[i].events & EPOLLIN) {
				int recv_args = recv(fd, (char*)buffer, BUFF_SIZE, 0);
				if (recv_args < 0) {
					if (errno == EWOULDBLOCK) {
						cout << "send cache full!" << endl;
						continue;
					}

					cout << "recv error" << endl;
					close(fd);
					_event(epollfd, fd, EPOLLIN, EPOLL_CTL_DEL);
				} else if (recv_args == 0) {
					cout << "client close" << endl;
					close(fd);
					_event(epollfd, fd, EPOLLIN, EPOLL_CTL_DEL);
				} else {
					cout << "recv:" << buffer << endl;
				}
			} else if (events[i].events & EPOLLOUT) {
				cout << "available to send!" << endl;
			}

		}
	}
	close(epollfd);

	return 0;
}
