#include <iostream>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

int main() {
	int keyboard;
	int ret;
	fd_set readfds;
	char key;
	struct timeval timeout;

	char *path = "/dev/tty";
	keyboard = open(path, O_RDONLY, O_NONBLOCK);
	if (keyboard < 0) {
		cout << "open error" << endl;
		return -1;
	}
	cout << "keyboard is " << keyboard << endl;
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(keyboard, &readfds);
		
		ret = select (keyboard+1, &readfds, NULL, NULL, &timeout);
		if (ret < 0) {
			cout << "select error" << endl;
			return -1;
		}
		ret = FD_ISSET(keyboard, &readfds);
		if (ret > 0) {
			read(keyboard, &key, 1);
			if ('\n' == key) {
				continue;
			}
			cout << "the input is" << key << endl;
			if ('q' == key) {
				break;
			}
		}
	}
	return 0;

}
