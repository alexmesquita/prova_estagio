#include <iostream>
#include <algorithm>
#include <vector>
#include <sstream>
#include "md5.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define BUFFER_SIZE 1000000

using namespace std;

string calculate_parity(long parity){
    int i = 0;
    do
    {
        if( parity & 0x01 )
        {
            i++;
        }
        parity >>= 1;
    } while( parity );

    if((i & 0x01)){
        return "01";
    }
    else {
        return "00";
    }
}

long parity_md5(string hex, long parity){
    int size = hex.size();

    for (int i = 0; i < size; i+=2)
    {
        parity = parity ^ strtol(hex.substr(i, 2).c_str(), NULL, 16);
    }

    return parity;
}

void error(string msg){
    cout << "ERROR! " << msg << endl;
    exit(0);
}


int main(){

    // ----------------SOCKET---------------------------
    int port_number = 64013;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if(sockfd < 0)
        error("socket system call failed!");

    struct hostent *server;
    server = gethostbyname("191.237.249.140");

    struct sockaddr_in serv_addr;
    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);
    serv_addr.sin_port = htons(port_number);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("connection failed!");

    char buffer[BUFFER_SIZE];
    bzero(buffer,BUFFER_SIZE);
    int n = read(sockfd, buffer, BUFFER_SIZE-1);

    if (n < 0)
        error("reading from socket");

    char aux[BUFFER_SIZE];
    string formatted_text = "";

    for(int i = 0; i<n; i++){
    sprintf(aux,"%02X", (unsigned char)buffer[i]);
    formatted_text += aux; 
    }
    // cout << "SEM FORMATAR: " << formatted_text << endl;
    // -------------------------------------------


    vector<string> v, p;

    string length = formatted_text.substr(0,4);

    formatted_text.erase(formatted_text.begin(), formatted_text.begin()+36);
    formatted_text.erase(formatted_text.end()-2, formatted_text.end());

    // cout << "FORMATADO: " << formatted_text << endl;

    // populating 2 hexadecimal vectors
    int size = formatted_text.size();
    for (int i = 0; i < size; i+=2){
        v.push_back(formatted_text.substr(i,2));
        p.push_back(formatted_text.substr(i,2));
        // cout << formatted_text.substr(i,2) << endl;
    }

    // order to calculate the most common
    sort(v.begin(),v.end());

    int max = 1, c = 1;

    string temp = v[0];
    int r = strtol(temp.c_str(), NULL, 16);
    size = v.size();
    for (int i = 1; i < size; i++)
    {
        if(v[i] != temp){
            if(c > max){
                max = c;
                r=strtol(temp.c_str(), NULL, 16);
                cout << "mais comum: " << temp << " -- vezes: " << max << endl;

            }
            c = 0;
            temp = v[i];
        }
        c++;
    }

    long parity = 0;

    string deciphered = "";

    std::stringstream sstream;

    for (int i = 0; i < size; i++){
        long result = strtol(p[i].c_str(),NULL,16)^(r^32); 
        deciphered += char(result);
        sstream << std::hex << result;

        parity = parity ^ result;
    }

    cout << deciphered << endl;

    string hex = sstream.str();

    parity = parity_md5(hex, parity);

    parity = parity ^ strtol(length.substr(0,2).c_str(), NULL, 16);
    parity = parity ^ strtol(length.substr(2,2).c_str(), NULL, 16);

    string byte = length + md5(deciphered) + hex + calculate_parity(parity);

    const char *pos;
    unsigned char val[BUFFER_SIZE];
    int count = 0;

    pos =  byte.c_str();
    size = byte.size()/2;

    for(count = 0; count < size; count++) {
        sscanf(pos, "%2hhx", &val[count]);
        pos += 2 * sizeof(char);
        cout << val[count] << endl;
    }

    n = write(sockfd,val,size);

    if (n < 0) 
        error("ERROR writing to socket");

    close(sockfd);
}
  