/*
 * Copyright (C) 2018 David C. Harrison. All right reserved.
 *
 * You may not use, distribute, publish, or modify this code without 
 * the express written permission of the copyright holder.
 https://classes.soe.ucsc.edu/cmps109/Spring18/SECURE/12.Distributed2.pdf
 https://classes.soe.ucsc.edu/cmps109/Spring18/SECURE/13.Distributed3.pdf
 https://classes.soe.ucsc.edu/cmps109/Spring18/SECURE/14.Distributed4.pdf
 */

#include "radix.h"
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <thread>
#include <sys/socket.h>
#include <sys/types.h> 
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>


const int R = 2 << 8;

static int charAt(std::string s, int i){
	if ((unsigned)i<s.length()) 
		return s.at(i);
    return -1;
}

static void sort(std::vector<unsigned int> (&s), unsigned int aux[], int lo, int hi, int at){
	if (hi<=lo) return;

	int count[R+2];

	for (int i = 0; i < R+2; i++) 
		count[i] = 0;
	for (int i = lo; i<=hi; ++i)
	    count[charAt(std::to_string(s[i]), at)+2]++;
	for (int i = 0; i<R+1; ++i) 
		count[i+1] += count[i];
	for (int i = lo; i<=hi; ++i)
		aux[count[charAt(std::to_string(s[i]), at)+1]++] = s[i];
	for (int i = lo; i<=hi; ++i) 
		s[i] = aux[i-lo];
	for (int r = 0; r<R; ++r) 
		sort(s, aux, lo+count[r], lo+count[r+1]-1, at+1);
}

static void sort2(std::vector<unsigned int> (&s), int len){
	unsigned int aux[len];
	int lo = 0;
	int hi = len-1;
	int at = 0;
	sort(s, aux, lo, hi, at);
}

void ParallelRadixSort::msd(std::vector<std::reference_wrapper<std::vector<unsigned int>>> &lists, const unsigned int cores) { 
	std::vector<std::thread*> threads;
	unsigned int threadTotal = 0;

	for (std::vector<unsigned int> &list : lists){
	threads.push_back(new std::thread{[&list]{
		sort2(list, list.size());
	}});
	threadTotal++;
	if (threads.size() == cores || threadTotal == lists.size()){
		for (std::thread *thread : threads)
			thread->join();
		threads.clear();
	}
	}
}

RadixServer::RadixServer(const int port, const unsigned int cores) {
	std::vector<unsigned int> myList;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) exit(-1);

	struct sockaddr_in server_addr;
	bzero((char*) &server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);

	if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) exit (-1);

	listen(sockfd, 5);
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	int newsockfd = accept(sockfd, (struct sockaddr *) &client_addr, &len);
	if (newsockfd < 0) exit (-1);

	for (;;){
		unsigned int test = 0;
		int rc = recv(newsockfd, (void*)&test, sizeof(unsigned int), 0);
		unsigned int local = ntohl(test);
		unsigned int zero = htonl(0);

		if (rc < 0){
			printf("What the");
		}
		else if (rc == 0){
			close(newsockfd);
			close(sockfd);
			break;
		}
		if (local != 0){
			myList.push_back(local);
		}
		if (local == 0){ 
			sort2(myList, myList.size());
			for (unsigned int i = 0; i<myList.size(); i++){
			unsigned int bruh = htonl(myList[i]);
			send(newsockfd, (void*)&bruh, sizeof(unsigned int), 0);
			}
			send(newsockfd, (void*)&zero, sizeof(unsigned int), 0);
			myList.clear();
		}
	}
}

void RadixClient::msd(const char *hostname, const int port, std::vector<std::reference_wrapper<std::vector<unsigned int>>> &lists) { 
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	std::vector<unsigned int> myList;

	struct hostent *server = gethostbyname(hostname);
	if (server == NULL) exit (-1);

	struct sockaddr_in serv_addr;
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

	serv_addr.sin_port = htons(port);

	if (connect(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) exit (-1);

	for (unsigned int i = 0; i<lists.size(); i++){
		for (unsigned int j = 0; j < lists[i].get().size(); j++){
			unsigned int sentNum = htonl(lists[i].get()[j]);
			send(sockfd, (void*)&sentNum, sizeof(unsigned int), 0);
		}
		unsigned int zero = htonl(0);
		send(sockfd, (void*)&zero, sizeof(unsigned int), 0);
		for (;;){
		unsigned int test = 0;
		int rc = recv(sockfd, (void*)&test, sizeof(unsigned int), 0);
			if (rc < 0 || test == 0){
				break;
			}
		unsigned int local = ntohl(test);
		myList.push_back(local);
		}
		for (unsigned int k = 0; k < lists[i].get().size(); k++){
			lists[i].get()[k] = myList[k];
		}
		myList.clear();
	}
	close(sockfd);
}
