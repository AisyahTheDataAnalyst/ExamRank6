#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

enum {
	BUFFER_SIZE = 100000,
	SEND_BUFFER_SIZE = 100050,
	MAX_CLIENT = 1024
};

typedef struct s_client
{
	int id;
	char msg[BUFFER_SIZE];
}	t_client;

struct pollfd fds[MAX_CLIENT + 1];
t_client client[MAX_CLIENT + 1];

int fdcount = 0;
int currentid = 0;

char sendbuffer[SEND_BUFFER_SIZE];
char recvbuffer[BUFFER_SIZE];

void errMsg(char *str) {
	if (str)
		write(2, str, strlen(str));
	else
		write(2, "Fatal error\n", 12);
	exit(1);
}

void boardCastMsg(int senderfd) {
	int i = 1;
	while (i < fdcount)
	{
		if (fds[i].fd != senderfd && fds[i].fd != -1)
			send(fds[i].fd, sendbuffer, strlen(sendbuffer), 0);
		i++;
	}
}

void newConnection(int serverfd) {
	struct sockaddr_in cli;
	socklen_t len = sizeof(cli);
	int newfd = accept(serverfd, (struct sockaddr *)&cli, &len);
	if (newfd < 0)
		errMsg(NULL);
	if (fdcount > MAX_CLIENT) {
		close(newfd);
		return ;
	}

	fds[fdcount].fd = newfd;
	fds[fdcount].events = POLLIN;

	client[fdcount].id = currentid;
	currentid++;
	bzero(client[fdcount].msg, BUFFER_SIZE);
	sprintf(sendbuffer,  "server: client %d just arrived\n", client[fdcount].id);
	boardCastMsg(newfd);
	fdcount++;
}

void disconnection(int fd, int idx) {
	sprintf(sendbuffer, "server: client %d just left\n", client[idx].id);
	boardCastMsg(fd);
	close (fd);

	int i = idx;
	while (i < fdcount - 1)
	{
		fds[i] = fds[i + 1];
		client[i] = client[i + 1];
		i++;
	}
	fdcount--;
}

void processMsg(int idx, int ret) {
	int j = strlen(client[idx].msg);
	int i = 0;

	while (i < ret)
	{
		if (j > BUFFER_SIZE)
			j = 0;

		client[idx].msg[j] = recvbuffer[i];
		if (client[idx].msg[j] == '\n') {
			client[idx].msg[j] = '\0';
			sprintf(sendbuffer, "client %d: %s\n", client[idx].id, client[idx].msg);
			boardCastMsg(fds[idx].fd);
			bzero(client[idx].msg, BUFFER_SIZE);
			j = -1;
		}
		i++;
		j++;
	}
}

void handleClient(int fd, int idx) {
	int ret = recv(fd, recvbuffer, BUFFER_SIZE, 0);

	if (ret <= 0)
		disconnection(fd, idx);
	else
		processMsg(idx, ret);
}

int main(int argc, char *argv[]) {
	if (argc != 2)
		errMsg("Wrong number of arguments");

	int severfd = socket(AF_INET, SOCK_STREAM, 0);
	if (severfd < 0)
		errMsg(NULL);

	fds[0].fd = severfd;
	fds[0].events = POLLIN;
	fdcount++;

	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	addr.sin_port = htons(atoi(argv[1]));

	if ((bind(severfd, (const struct sockaddr *)&addr, sizeof(addr))) != 0)
		errMsg(NULL);
	if (listen(severfd, 10) != 0)
		errMsg(NULL);

	while (1)
	{
		if (poll(fds, fdcount, -1) == -1)
			errMsg(NULL);
		if (fds[0].revents & POLLIN)
			newConnection(severfd);
		int i = 1;
		while (i < fdcount)
		{
			if (fds[i].revents & POLLIN)
				handleClient(fds[i].fd, i);
			i++;
		}
	}
	return 0;
}
