#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <mutex>
#include <json/json.h>
#pragma comment(lib, "ws2_32.lib")

#include "PlayerData.h"

#define MAX_HEADER_LEN 20
#define MAX_CLIENTS 2
/*
	If the header or data send/recv failed just bail the connection and try to reconnect.
	It shouldn't though under normal working conditions.
*/

int try_send(SOCKET s, std::string msg) {
	std::stringstream msg_len;
	msg_len << msg.length();
	// Tell the recipient the length of the next message
	int msg_res = send(s, msg_len.str().c_str(), MAX_HEADER_LEN, 0);
	if (msg_res == SOCKET_ERROR) {
		return 1;
	}
	msg_res = send(s, msg.c_str(), msg.length(), 0);
	if (msg_res == SOCKET_ERROR) {
		return 1;
	}
	return 0;
}

int try_receive(SOCKET s, std::string* data) {
	std::string recv_buf;

	// Create a message buffer
	char msg_header[MAX_HEADER_LEN];
	memset(msg_header, '\0', MAX_HEADER_LEN);

	// try to receive the header
	int msg_res = recv(s, msg_header, MAX_HEADER_LEN, 0);
	if (msg_res == SOCKET_ERROR) {
		return 1;
	}
	else if (msg_res == 0) {
		return 2;
	}

	// get msg length
	int msg_len = atoi(msg_header);
	if (msg_len < 1) {
		return 3;
	}

	// create buffer for msg
	char* recv_msg;
	recv_msg = (char*)malloc((msg_len + 1) * sizeof(char));
	memset(recv_msg, '\0', (msg_len + 1));

	// try to receive message
	msg_res = recv(s, recv_msg, msg_len, 0);
	if (msg_res == SOCKET_ERROR) {
		free(recv_msg);
		return msg_res;
	}
	else if (msg_res == 0) {
		free(recv_msg);
		return msg_res;
	}

	// Force terminator at end of string
	recv_msg[msg_len] = '\0'; 

	// Copy char* into specified string
	(*data) = std::string(recv_msg);
	return 0;
}