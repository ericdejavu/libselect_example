import select
import socket
from Queue import Queue

server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_address = ("127.0.0.1", 11277)
server_sock.bind(server_address)
server_sock.listen(10)
server_sock.setblocking(False)

timeout = 5

epoll = select.epoll();
epoll.register(server_sock.fileno(), select.EPOLLIN)

mq = {}
fd_to_socket = {server_sock.fileno(): server_sock}

while True:
    print ('wait for client...')
    events = epoll.poll(timeout)
    if not events:
        print ('epoll timeout')
        continue
    print ('events len:', len(events))
    for fd,event in events:
        socket = fd_to_socket[fd]
        if socket == server_sock:
            print ('start new connection:', address)
            connection.setblocking(False)
            epoll.register(connection.fileno(), select.EPOLLIN)
            fd_to_socket[connection.fileno()] = connection
            mq[connection] = Queue()

        elif event & select.EPOLLHUP:
            print ('client close')
            epoll.unregister(fd)
            fd_to_socket[fd].close()
            del fd_to_socket[fd]
        elif 


