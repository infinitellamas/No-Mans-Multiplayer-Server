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
#include "Communication.h"

#define DEFAULT_PORT 5258
#define SECONDS(s) std::chrono::seconds(s)
#define MILLIS(s) std::chrono::milliseconds(s)

bool keep_open = true;

PlayerData* client_data[MAX_CLIENTS] = { nullptr };
std::mutex data_lock[MAX_CLIENTS];
std::mutex lock_lock_all;
std::vector<std::thread> client_connection_threads;

struct sockaddr_in server_socket_address, next_client_socket_address;
SOCKET server_socket, next_client;

std::mutex print_lock;
void thread_print(const char* msg) {
	print_lock.lock();
	std::cout << msg << std::endl;
	print_lock.unlock();
}

std::string strcat_player_data(PlayerData* me) {
	
	Json::Reader reader;
	Json::Value jResponse;

	// Put the current player into json object
	Json::Value you;
	reader.parse(me->getJSONString(), you);
	jResponse["you"] = you;

	// For each player that is not the current player
	jResponse["others"] = Json::Value(Json::arrayValue);

	for (int i = 0; i < MAX_CLIENTS; i++) {
		data_lock[i].lock();
		if (client_data[i] != nullptr) { // If valid
			if(client_data[i]->getId() != -1 && client_data[i]->getId() != me->getId()){
				Json::Value other_player;
				reader.parse(client_data[i]->getJSONString(), other_player);
				jResponse["others"].append(other_player);
			}
		}
		data_lock[i].unlock();
	}
	
	return jResponse.toStyledString();
}

/*
*	Finds a free slot for a new client. If there are none, it returns -1
*/
int find_free_slot() {
	for (int i = 0; i < MAX_CLIENTS; i++) {
		data_lock[i].lock();
		if (client_data[i] == nullptr) {
			data_lock[i].unlock();
			return i;
		}
		data_lock[i].unlock();
	}
	return -1;
}

void handle_client(SOCKET client_socket, sockaddr_in client_address, int client_addr_len) {

	// Create cout stream buffer
	std::stringstream info;

	// Find a slot for our client
	int my_client_offset = find_free_slot();

	// Player cannot join. We are full!
	if (my_client_offset == -1) { 

		// Notify client that message is of length
		std::string client_error_msg = "code:1,msg: \"Server Full!\"";
		try_send(client_socket, client_error_msg);
		info << "Client Disconnected! Server Full";
		thread_print(info.str().c_str());

		// Kill the socket
		closesocket(client_socket);

		// Then kill the thread
		return;
	}

	// Update player buffer with my player data.
	PlayerData my_player_data;
	data_lock[my_client_offset].lock();
	{
		my_player_data.setId(my_client_offset);
		client_data[my_client_offset] = &my_player_data;
	}
	data_lock[my_client_offset].unlock();

	info.str("");
	info.flush();
	info << "Client connected:" << my_client_offset;
	thread_print(info.str().c_str());

	// While the server is open
	while (keep_open) {
		std::this_thread::sleep_for(MILLIS(10));

		std::string send_buffer = strcat_player_data(&my_player_data);
		info.flush();
		info.str("");
		info << send_buffer.length();
		int send_result = try_send(client_socket, send_buffer);

		if (send_result != 0) {
			info.str("");
			info.flush();
			info << "Network error on sending data. Disconnecting client[" << my_client_offset << "]" << std::endl; //Print data
			thread_print(info.str().c_str());
			break;
		}
		
		std::string recv_buffer;
		// Receive updated client data
		int recv_result = try_receive(client_socket, &recv_buffer);
		if (recv_result != 0) {
			info.str("");
			info.flush();
			info << "Network error on sending data. Disconnecting client[" << my_client_offset << "]" << std::endl; //Print data
			thread_print(info.str().c_str());
			break;
		}

		//Update this clients data
		PlayerData temp_pdata;
		bool parse_result = PlayerData::fromJSONString(&temp_pdata, recv_buffer);
		
		// If the JSON was valid
		if (parse_result) {
			// Update our player data
			data_lock[my_client_offset].lock();
			{
				my_player_data = temp_pdata;
				my_player_data.setId(my_client_offset);
			}
			data_lock[my_client_offset].unlock();
		}


	}

	info.str("");
	info.flush();
	info << "Client disconnected:" << my_client_offset;
	thread_print(info.str().c_str());
	closesocket(client_socket);
	client_data[my_client_offset] = nullptr; //Disconnected
	return;

}

bool start_server(SOCKET* server_socket) {
	WSADATA wsa;
	SOCKET server_socket_temp;
	std::cout << "Initializing Winsock..." << std::endl;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Failed. Error Code: " << WSAGetLastError() << std::endl;
		WSACleanup();
		return false;
	}
	std::cout << "Initialized." << std::endl;

	if ((server_socket_temp = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		std::cout << "Could not create socket: " << WSAGetLastError() << std::endl;
		closesocket(server_socket_temp);
		WSACleanup();
		return false;
	}
	std::cout << "Socket created." << std::endl;
	server_socket_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_socket_address.sin_family = AF_INET;
	server_socket_address.sin_port = htons(DEFAULT_PORT);

	if (bind(server_socket_temp, (struct sockaddr *)&server_socket_address, sizeof(server_socket_address)) == SOCKET_ERROR)
	{
		std::cout << "Socket bind failed!" << std::endl;
		closesocket(server_socket_temp);
		WSACleanup();
		return false;
	}
	memcpy(server_socket, &server_socket_temp, sizeof(server_socket_temp));
	std::cout << "Socket bound! Ready to accept connections." << std::endl;
	return true;
}

bool check_open() {
	return  GetAsyncKeyState(VK_F1) == 0 || GetAsyncKeyState(VK_F2) == 0;
}

/*###################################################################################################*/
//											MAIN FUNC HERE
/*###################################################################################################*/
int main(int argc, char *argv[]) {


	std::cout << "---------------------------------" << std::endl;
	std::cout << "-- No Man's Multiplayer Server --" << std::endl;
	std::cout << "------- By InfiniteLlamas -------" << std::endl;
	std::cout << "---------------------------------" << std::endl;

	bool start_res = start_server(&server_socket);
	if (start_res) {
		listen(server_socket, 3);
		std::cout << "Listening on socket..." << std::endl;
		while (keep_open = check_open()) {
			int c = sizeof(struct sockaddr_in);
			next_client = accept(server_socket, (struct sockaddr*)&next_client_socket_address, &c); // Wait for next client
			client_connection_threads.push_back(std::thread(handle_client, next_client, next_client_socket_address, c)); // Pass to thread to handle
		}
		closesocket(server_socket); // Close the server socket, disconnecting all clients.
	}
	system("PAUSE");
	return 0;




	return 0;
}

