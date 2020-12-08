#!/bin/sh
g++ -pthread -std=c++11 Server.cpp -o Server
g++ -std=c++11  Client.cpp -o Client

chmod 700 Server
chmod 700 Client


