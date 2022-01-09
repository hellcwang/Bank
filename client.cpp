#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <signal.h>

constexpr unsigned int SERVER_PORT = 50544;
constexpr unsigned int MAX_BUFFER = 128;

int sockfd = -1;
void handler_int(int);
int main(int argc, char *argv[])
{

	if (argc != 2)
    	{
        	std::cerr << "Error! usage: ./client localhost" << std::endl;
		return 6;
    	}

    	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    	if (sockfd < 0) 
    	{
        	std::cerr << "socket error" << std::endl;
        	return 1;
    	}
	signal(SIGINT, handler_int);

    	struct hostent* server = gethostbyname(argv[1]);
    	if (server == nullptr) 
    	{
        	std::cerr << "gethostbyname, no such host" << std::endl;
        	return 2;
    	}

    	struct sockaddr_in serv_addr;
    	bzero((char *) &serv_addr, sizeof(serv_addr));
    	serv_addr.sin_family = AF_INET;
    	bcopy((char *)server->h_addr, 
          (char *)&serv_addr.sin_addr.s_addr, 
          server->h_length);
    	serv_addr.sin_port = htons(SERVER_PORT);
    	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
    	{
        	std::cerr << "connect error: the server does not look running" << std::endl;
        	return 3;
    	}

    	std::string readBuffer (MAX_BUFFER, 0);
    	/*if (read(sockfd, &readBuffer[0], MAX_BUFFER-1) < 0)
    	{
     		std::cerr << "read from socket failed" << std::endl;
       		return 5;
    	}
    	std::cout << readBuffer << std::endl;
	*/
    	std::string writeBuffer (MAX_BUFFER, 0);
    	while(1){
		std::fill( std::begin(readBuffer), std::end(readBuffer ), 0 );
    		if (read(sockfd, &readBuffer[0], MAX_BUFFER-1) < 0)
    		{
        		std::cerr << "read from socket failed" << std::endl;
        		return 5;
    		}
    		std::cout << readBuffer << "\n>>" ;

		std::fill( std::begin(writeBuffer), 
				std::end(writeBuffer ), 0 );
    		getline(std::cin, writeBuffer);
    		if (write(sockfd, writeBuffer.c_str(),\
			       	strlen(writeBuffer.c_str())) < 0) 
    		{
        		std::cerr << "write to socket" << std::endl;
        		return 4;
    		}
    	}

    	close(sockfd);
    	return 0;
}

void handler_int(int sig){
	if(sockfd > 0){
		close(sockfd);
	}
	exit(0);
}
