#pragma once



int open_server_socket(std::string& socket_path); // -> <0: FD, -1: socket() error, -2: path too long, -3: path taken

int open_client_socket(std::string& socket_path);
