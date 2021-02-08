import select, sys
import socket
import Queue

server_sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server_address = ("127.0.0.1", 11277)
server_sock.bind(server_address)
server_sock.listen(10)
server_sock.setblocking(False)

timeout = 5

epoll = select.epoll()
epoll.register(server_sock.fileno(), select.EPOLLIN)
epoll.register(0, select.EPOLLIN)

mq = {}
fd_to_socket = {server_sock.fileno(): server_sock}
fd_to_socket[0] = 0

while True:
    print ('wait for client...')
    events = epoll.poll(timeout)
    if not events:
        print ('epoll timeout')
        continue
    print ('events len:', len(events))
    for fd,event in events:
        print ('event', bin(event))

        socket = fd_to_socket[fd]
	if fd == 0:
		word = sys.stdin.readline()
        	for k in fd_to_socket.keys():
			if k == 0 or fd_to_socket[k] == server_sock:
				continue
			fd_to_socket[k].send(word)
	elif socket == server_sock:
            connection, address = server_sock.accept()
            print ('start new connection:', address)
            connection.setblocking(False)
            epoll.register(connection.fileno(), select.EPOLLIN)
            fd_to_socket[connection.fileno()] = connection
            mq[connection] = Queue.Queue()

        elif event & select.EPOLLHUP:
            print ('client close')
            epoll.unregister(fd)
            fd_to_socket[fd].close()
            del fd_to_socket[fd]
            
        elif event & select.EPOLLIN:
            data = socket.recv(1024)
            if data:
                print ('recv data:', data, 'client side:', socket.getpeername())
                mq[socket].put(data)
                epoll.modify(fd, select.EPOLLOUT)
                
        elif event & select.EPOLLOUT:
            try:
                msg = mq[socket].get_nowait()
            except Queue.Empty:
                print (socket.getpeername(), ' query empty')
                epoll.modify(fd, select.EPOLLIN)
            else:
                print ('send data: ', data, 'client side:', socket.getpeername())
                socket.send(msg)

epoll.unregister(server_sock.fileno())
epoll.close()
server_sock.close()
