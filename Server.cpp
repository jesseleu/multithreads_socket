//
//  Server.cpp
//  proj4
//
//  Created by Jesse Leu on 5/27/20.
//  Copyright © 2020 Jesse Leu. All rights reserved.
//
#include <sys/types.h>    // socket, bind
#include <sys/socket.h>   // socket, bind, listen, inet_ntoa
#include <netinet/in.h>   // htonl, htons, inet_ntoa
#include <arpa/inet.h>    // inet_ntoa
#include <netdb.h>        // gethostbyname
#include <unistd.h>       // read, write, close
#include <string.h>       // bzero
#include <netinet/tcp.h>  // SO_REUSEADDR
#include <sys/uio.h>      // writev
#include <pthread.h>
#include <iostream>

#define BUFFSIZE 1500
#define PENDING 5
using namespace std;

void* readClient(void* data)
{
    
    char databuf[BUFFSIZE]; //Allocate dataBuf[BUFSIZE],
    bzero(databuf, BUFFSIZE);
    
    int newSD; //cast descripter from param
    newSD = *(int*)data;
    
    int received_int = 0;
    //get rep time
    int readbytes = (int)read(newSD, &received_int, sizeof(received_int));
    if(readbytes < 0)
    {
        cout << "fail to read rep time" << endl;
    }
    int rep = ntohl(received_int);
    
    int total = 0;
    int count = 0;
    
    while(total != BUFFSIZE * rep) // make sure to read the correct amount of data
    {
        int bytesRead = (int) read(newSD, databuf, BUFFSIZE);
        if(bytesRead < 0)
        {
            cout << "fail to read from client" << endl;
        }
        total += bytesRead;
        count++; //count the read time
    }
    
    
    
    int converted_number = htonl(count);
    // Tell client how many read time
    int bytesWritten = (int)write(newSD, &converted_number, sizeof(converted_number));
    if(bytesWritten < 0)
    {
        cout << "fail to write time count" << endl;
    }
    
    close(newSD);
    
    
    pthread_exit(0);
}


int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        cout << "Check the parameters" << endl;
        exit(1);
    }
    
    int serverPort = atoi(argv[1]);
    
    
    
    /*
     * Build address
     */
    sockaddr_in acceptSocketAddress;
    bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
    acceptSocketAddress.sin_family = AF_INET;
    acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    acceptSocketAddress.sin_port = htons(serverPort);
    /*
     *  Open socket and bind
     */
    int serverSD = socket(AF_INET, SOCK_STREAM, 0);
    const int on = 1;
    setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
    
    int rc = ::bind(serverSD,(sockaddr*)&acceptSocketAddress,sizeof(acceptSocketAddress));
    if (rc < 0)
    {
        cerr << "Bind Failed" << endl;
    }
    /*
     *  listen and accept
     */
    listen(serverSD, PENDING);       //setting number of pending connections
    
    while(1)
    {
        sockaddr_in newSockAddr;
        socklen_t newSockAddrSize = sizeof(newSockAddr);
        int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);//wait and accept connection
        
        pthread_t new_thread;
        int threadResult = pthread_create(&new_thread , nullptr , readClient , (void *) &newSD);
        if(threadResult != 0)
        {
            cout << "thread fail" <<endl;
        }
        
    }
    
}


