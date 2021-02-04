import socket

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = ('127.0.0.1', 11277)
client.connect(server_address)

data = raw_input('input:')
client.sendall(data)
server_data = client.recv(1024)
print ('server send:', server_data)
client.close()
