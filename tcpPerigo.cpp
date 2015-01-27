#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

void error(string msg)
{
	cout << "ERROR! " << msg << endl;
	exit(0);
}

int main(int argc, char const *argv[])
{
	
	if (argc < 3)
		error("Have not passed all arguments!");

    int port_number = atoi(argv[2]);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0)
    	error("socket system call failed!");

    struct hostent *server;
    server = gethostbyname(argv[1]);

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(port_number);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("connection failed!");

    char buffer[1000000];
    bzero(buffer,1000000);
    int n = read(sockfd, buffer, 1000000-1);

    if (n < 0)
        error("reading from socket");

    string message = buffer;

    for (size_t i = 0; i < message.size(); ++i)
    	cout << (char)message[i] << " ";
    cout << endl;

    cout << "message: " << message.size() << endl;
    cout << "message: " << message.substr(0,4) << endl;
    close(sockfd);

	return 0;
}