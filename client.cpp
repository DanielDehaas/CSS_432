/*
 *		Daniel T Dehaas
 *		CSS 432 -- UWB
 *		HW 1 - TCP Sockets
 *		10/15/2017
 *
 * 		The following is a client program that connects to a server and
 * 			writes configuration data to instruct the server which tests
 * 			to perform.
 *
 * 		The program takes three command line argument: port number, ip-address,
 * 			and mode [0-3] which signify: multi-write, writeV, single write, and
 * 			run all three tests for full report.
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <netinet/tcp.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <cerrno>
#include <cstring>
#include <clocale>
#include <chrono>

using namespace std::chrono;

const int REPETITIONS = 20000;
const int BUFFER_SIZE = 1500;

// formatted print results
void PrintResults(int mode, int reads, double stats) {
	// stop - start = data-receiving time
	std::cout << "Test " << (mode + 1) << ": " << std::endl;
	std::cout << stats << " seconds" << std::endl;
	std::cout << REPETITIONS << " repetitions" << std::endl;
	std::cout << std::endl;
}

//
void ReadFromServer(const int socket_descriptor) {
	char data_buffer[BUFFER_SIZE];
	int counter = 0;
	int num_read;
	steady_clock::time_point start;
	steady_clock::time_point end;
	start = steady_clock::now();
	for ( int i = 0; i < 4; ++i ) {

		start = steady_clock::now();
		for ( int j = 0; j < REPETITIONS; ++j ) {

			// read until buffer is full
			for ( num_read = 0;
						( num_read += read( socket_descriptor, data_buffer,
									BUFFER_SIZE - num_read )) < BUFFER_SIZE;
									 ++counter);
			if (num_read < 0) {
				std::cerr << "read error";
				std::cout << std::strerror(errno) << std::endl;
			} else {
				++counter;
			}

		}

		// send number of buffers received to server
		char diagnostic_buffer[] = {(char)counter};
		if ( write( socket_descriptor, diagnostic_buffer, sizeof(diagnostic_buffer) ) < 0) {
			std::cout << "FAIL SQUAD!" << std::endl;
		}
		else {
			std::cout << "SUCCESS!" << std::endl;
		}

		end = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>( end - start );
		double stat = time_span.count();
		PrintResults( i, num_read, stat );


	}
}

// poorly named function which chooses whether to read one test pass
//		from socket or to read all three tests
void DoWorkSon( const int mode, const int socket_descriptor ) {
	// this doesn't need to be a switch. Should just be an
	// if (0||1||2) else if (3) else (default)
	// Refactoring unnecessary since this does work fine.
	switch(mode) {
		case 0 :
			// Multiple Reads
			std::cout << "--------- Multiple Writes ----------" << std::endl;
			ReadFromServer( socket_descriptor );
			break;
		case 1 :
			// ReadV()
			std::cout << "------------ WriteV() --------------" << std::endl;
			ReadFromServer( socket_descriptor );
			break;
		case 2 :
			//Single Read
			std::cout << "---------- Single Writes -----------" << std::endl;
			ReadFromServer( socket_descriptor );
			break;

		case 3 :
			//full report
			std::cout << "--------- Multiple Writes ----------" << std::endl;
			ReadFromServer( socket_descriptor );
			std::cout << "------------ WriteV() --------------" << std::endl;
			ReadFromServer( socket_descriptor );
			std::cout << "---------- Single Writes -----------" << std::endl;
			ReadFromServer( socket_descriptor );
			break;

		default :
			// if mode gets translated from command line wrong
			std::cerr << "Mode data corrupted.";
			break;
	}
}

int main(int argc, const char* argv[]) {

	// basic command line checking
	if (argc != 4) {
		std::cerr << " Three arguments expected - abort";
		exit(1);
	}

	int server_port			= atoi(argv[1]);
	const char* server_name = argv[2];
	int mode				= atoi(argv[3]);

	while (mode < 0 || mode > 3 ) {
		std::cerr << "Mode must be in range [0-4] - reenter mode";
		std::cin >> mode;
	}

	// retrieve a hostent struc cooresponding to this IP name by calling...
	struct hostent* host = gethostbyname(server_name);

	// specify an address and socket descriptor
	int client_socket_descriptor;
	struct sockaddr_in send_socket_address;
	client_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

	// validate socket setup
	if (client_socket_descriptor < 0) {
		std::cerr << "socket";
		exit(1);
	}

	// set socket address after zero-initialize
	bzero((char*)&send_socket_address, sizeof(send_socket_address));
	send_socket_address.sin_family 		= AF_INET;
	send_socket_address.sin_port 		= htons(server_port);
	send_socket_address.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));

	// use connect()'s return value for validation
	int connection_stat = connect(client_socket_descriptor, (struct sockaddr*)&send_socket_address, sizeof(send_socket_address));

	// validate
	if (connection_stat < 0) {
		std::cerr << "error making connection";
		std::cout << std::strerror(errno) << std::endl;
		exit(1);
	}
	else {
		std::cout << "Connection established" << std::endl;
	}


	// buffer to receive welcome message
	char server_response[256];

	if ( read(client_socket_descriptor, server_response, sizeof(server_response)) < 0 ) {
		std::cerr << "read error " << std::endl;
		std::cout << std::strerror(errno) << std::endl;
	}

	std::cout << server_response << std::endl;

	// buffer to send test mode to server
	const char metadata[] = {*argv[3]};


	if ( write(client_socket_descriptor, metadata, sizeof(metadata)) < 0 ) {
		std::cerr << "write error " << std::endl;
		std::cout << std::strerror(errno) << std::endl;
	}

	// enter read mode
	DoWorkSon( mode, client_socket_descriptor );


	return 0;
}
