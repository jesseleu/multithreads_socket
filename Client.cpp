//
//  Client.cpp
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
#include <chrono>
#include <iostream>

const double to_Gbps = 0.008;
using namespace std;

// Usage ./client [port] [nreps] [nbufs] [bufsize] [ip/hostname] [type]
// Connects to a server process with the above parameters and transmits nbufs
// buffers of bufsize size nreps times, while timing the transmit and
// confirmation time. Outputs timing data to cout and raw data to cerr.
int main( int argc, char* argv[] )
{
    if (argc != 7)
    {
        cout << "Check the parameters" << endl;
        exit(1);
    }
    char* serverName = argv[1];
    char* port = argv[2];
    int reps = atoi(argv[3]);
    int nbufs = atoi(argv[4]);
    int bufsize = atoi(argv[5]);
    int type = atoi(argv[6]);
    
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int clientSD = -1;
    
    if(type != 1 && type != 2 && type != 3)
    {
        cout << "Type should be 1, 2 or 3" << endl;
        exit(1);
    }
    
    if (nbufs * bufsize != 1500)
    {
        cout << "nbufs * bufsize should equal 1500" << endl;
        exit(1);
    }
    
    char databuf[nbufs][bufsize];
    
    // Get socket file descriptor
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;
    int rc = getaddrinfo(serverName, port, &hints, &result);
    if (rc != 0)
    {
        cerr << "ERROR: " << gai_strerror(rc) << endl;
        exit(EXIT_FAILURE);
        
    }
    
    for (rp = result; rp != NULL; rp = rp->ai_next)
    {
        clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (clientSD == -1)
        {            continue;
            
        }
        rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
        if (rc < 0)
        {
            cerr << "Connection Failed" << endl;
            close(clientSD);
            return -1;
            
        }
        
        else
        {
            break;
        }
        
    }
    if (rp == NULL)
    {
        cerr << "No valid address" << endl;
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(result);
    
    int converted_number = htonl(reps);
    
    // Write int rep
    write(clientSD, &converted_number, sizeof(converted_number));
    auto start = std::chrono::high_resolution_clock::now(); //start time
    for (int i = 0; i < reps; i++)
    {
        if (type == 1)
        {
            for (int j = 0; j < nbufs; j++) // write buff size every time
            {
              write(clientSD, databuf[j], bufsize);
            }
        }
        else if (type == 2)
        {
            struct iovec vector[nbufs];
            for (int j = 0; j < nbufs; j++)
            {
                vector[j].iov_base = databuf[j];
                vector[j].iov_len = bufsize;
                
            }
            writev(clientSD, vector, nbufs);
        }
        else
        {
             write( clientSD, databuf, nbufs * bufsize );
        }
    }
    auto end = std::chrono::high_resolution_clock::now(); //end time
    auto int_usec = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    int count  = 0;
    int bytesRead = (int)read(clientSD, &count, sizeof(count));
    if(bytesRead < 0)
    {
        cout << "fail to get read time from customer" << endl;
    }

    count = ntohl(count);
    

    double throughput = (double)nbufs * (double)bufsize * (double)reps / (double)(int_usec.count()) * to_Gbps;
    //print result
    cout << "Test " << type << ": ";
    cout << "Time = " << int_usec.count() << " usec, ";
    cout << "#reads = " << count << "," ;
    cout << "throughput " << throughput << "Gbps " << endl;
    
    close(clientSD);
    return 0;
}
