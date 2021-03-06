#pragma once
#include <string>

	//struct
	//{
	//	//char doodleversion[sizeof(DOODLE_PROTOCOL_VERSION)-1];
	//	uint16_t length;
	//}  __attribute__((packed)) header;

int open_server_socket(std::string& socket_path); // -> <0: FD, -1: socket() error, -2: path too long, -3: path taken

int open_client_socket(std::string& socket_path);

void write_n(int fd, const char buffer[], int size);	// Write exactly size bytes

bool read_n(int fd, char buffer[], int size);	// Read exactly size bytes

std::string read_socket(int fd);
