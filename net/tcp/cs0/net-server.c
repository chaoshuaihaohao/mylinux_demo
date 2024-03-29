#include <stdio.h>
#include <string.h>		//bzero
#include <stdlib.h>		//exit

#include <unistd.h>		//read,close
#include <arpa/inet.h>		//htons,htonl
#include <sys/types.h>
#include <sys/socket.h>		//socket,bind

//#include <netinet/in.h>

int main(int argc, char *argv[])
{
	unsigned short port = 8000;

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("socket");
		exit(-1);
	}

	struct sockaddr_in my_addr;
	bzero(&my_addr, sizeof(my_addr));
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(port);
	my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int err_log =
	    bind(sockfd, (struct sockaddr *)&my_addr, sizeof(my_addr));
	if (err_log != 0) {
		perror("binding");
		close(sockfd);
		exit(-1);
	}

	err_log = listen(sockfd, 0);	// 等待队列为1
	if (err_log != 0) {
		perror("listen");
		close(sockfd);
		exit(-1);
	}
	printf("listen client @port=%d...\n", port);

	int i = 0;

	while (1) {
		struct sockaddr_in client_addr;
		char client_ip[INET_ADDRSTRLEN] = { 0 };
		socklen_t cliaddr_len = sizeof(client_addr);

		int connfd;
		connfd =
		    accept(sockfd, (struct sockaddr *)&client_addr,
			   &cliaddr_len);
		if (connfd < 0) {
			perror("accept");
			continue;
		}

		inet_ntop(AF_INET, &client_addr.sin_addr, client_ip,
			  INET_ADDRSTRLEN);
		printf("-----------%d------\n", ++i);
		printf("client ip=%s, port=%d\n", client_ip,
		       ntohs(client_addr.sin_port));

#if 0
		char recv_buf[512] = { 0 };
		while (recv(connfd, recv_buf, sizeof(recv_buf), 0) > 0) {
			printf("recv data: %s\n", recv_buf);
			break;
		}
#else
		int recv_buf, send_buf;
		while (recv(connfd, &recv_buf, sizeof(recv_buf), 0) > 0) {
			printf("recv data: %d\n", recv_buf);
			send_buf = recv_buf;
			send(connfd, &send_buf, sizeof(send_buf), 0);
//			break;
		}


#endif

		close(connfd);	//关闭已连接套接字
		printf("client closed!\n");

	}
	close(sockfd);		//关闭监听套接字
	return 0;
}
