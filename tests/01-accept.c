#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <err.h>

#include <pcm.h>

int server_run()
{
	int fd;
	struct sockaddr_in sin;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(! fd) {
		err(EXIT_FAILURE, "Unable to socket()");
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(65432); // hopefully unused
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(bind(fd, &sin, sizeof(struct sockaddr_in)) == -1) {
		err(EXIT_FAILURE, "bind");
	}

	if(listen(fd, 10) == -1) {
		err(EXIT_FAILURE, "listen");
	}

	return fd;
}

int client_connect()
{
	int fd;
	struct sockaddr_in sin;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if(! fd) {
		err(EXIT_FAILURE, "socket");
	}

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_port = htons(65432);
	sin.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(fd, &sin, sizeof(struct sockaddr_in)) == -1) {
		err(EXIT_FAILURE, "connect");
	}
}

void accept_connection(int sfd)
{
	int acfd;
	char *msg = "testing...";

	acfd = accept(sfd, NULL, NULL);
	if(! acfd) {
		err(EXIT_FAILURE, "accept");
	}

	if(write(acfd, msg, strlen(msg)) == -1) {
		err(EXIT_FAILURE, "write");
	}

	close(acfd);
}

int main(int argc, char **argv)
{
	int sfd;
	int cfd;

	setenv("PCM_HOOK", "accept", 1);
	setenv("PCM_POLICY_FILE", "../tests/01-accept.json", 1);
	pcm_initialize();

	sfd = server_run();	
	cfd = client_connect();
	accept_connection(sfd);

	return 0;
}
