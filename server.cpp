/*
 *		Daniel T Dehaas
 *		CSS 432 -- UWB
 *		HW 1 - TCP Sockets
 *		10/15/2017
 *
 * 		The following is a server program that waits for a client and
 * 			writes data in order to take some measurements about read/write
 * 			times using different write strategies and using different
 * 			types of buffers (of the same size.)
 *
 * 		The program takes one command line argument: port number.
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <netinet/tcp.h>
#include <iostream>
#include <cstdlib>
#include <chrono>
#include <cerrno>
#include <cstring>
#include <clocale>


using namespace std::chrono;

const int REPETITIONS = 20000;

void PrintResults( int buffer_settings, int test_type, double stat) {

	std::string test_types[3] = { "Multiple Writes", "WriteV", "Single Write"};
	std::cout << "Test " << (buffer_settings + 1) << ": " << std::endl;
	std::cout << "data-sending Time = " << stat << " sec." << std::endl;
	std::cout << "Per lap avg       = " << ((stat / REPETITIONS) * 1000000 ) << " usec" << std::endl;
	std::cout << "# writes          = " << REPETITIONS << std::endl;
	std::cout << std::endl;

}

void MultipleWrites(const int socket_descriptor) {
	// useful for this design -
	// each buffer configuration is tested
	// for each write strategy
	int number_buffers[4] = { 15, 30, 60, 100 };
	int buffer_size[4] = { 100, 50, 25, 15 };
	char diagnostic_buffer[256];

	// using chrono class for timing
	// declare necessary time points
	// Per the instructions, server tracks
	// 'lap' time, which is read to be
	// time/1write
	steady_clock::time_point lap_start;
	steady_clock::time_point lap_reset;
	duration<double> lap_time;
	double total_lap_time = 0;
	std::cout <<  "---------- Multi Write -----------\n";

	for( int i = 0; i < 4; ++i ) {
		char data_buffer[number_buffers[i]][buffer_size[i]];
		for( int j = 0; j < REPETITIONS; ++j ) {
			for( int k = 0; k < number_buffers[i]; ++k ) {
				lap_start = steady_clock::now();
				if ( write( socket_descriptor, data_buffer[k], (buffer_size[i]) ) < 0 ) {
					std::cerr << "write error";
					std::cout << std::strerror(errno) << std::endl;
				}
				lap_reset = steady_clock::now();
				lap_time = duration_cast<duration<double>>(lap_reset - lap_start);
				total_lap_time += lap_time.count();
			}
		}

		if (read(socket_descriptor, diagnostic_buffer, sizeof(diagnostic_buffer)) < 0) {
			std::cout << "FAILURE!" << std::endl;
		}
		else {
			std::cout << "SUCCESS!" << std::endl;
		}
		lap_reset = steady_clock::now();
		lap_time = duration_cast<duration<double>>(lap_reset - lap_start);
		total_lap_time += lap_time.count();
		std::cout << "buffer[" << number_buffers[i] << "][" << buffer_size[i] << "]" << std::endl;
		PrintResults( i, 0, total_lap_time );
	}
}

void WriteVee(const int socket_descriptor) {
	// useful for this design -
	// each buffer configuration is tested
	// for each write strategy
	int number_buffers[4] = { 15, 30, 60, 100 };
	int buffer_size[4] = { 100, 50, 25, 15 };
	char diagnostic_buffer[256];
	std::cout << "------------ WriteV() --------------" << std::endl;
	// using chrono class for timing
	// declare necessary time points
	// Per the instructions, server tracks
	// 'lap' time, which is read to be
	// time/1write
	steady_clock::time_point lap_start;
	steady_clock::time_point lap_reset;
	duration<double> lap_time;
	double total_lap_time = 0;
	std::cout <<  " WriteV \n";
	for( int i = 0; i < 4; ++i ) {
		char data_buffer[number_buffers[i]][buffer_size[i]];
		for( int j = 0; j < REPETITIONS; ++j ) {
			struct iovec vector[number_buffers[i]];
			lap_start = steady_clock::now();

			for( int k = 0; k < number_buffers[i]; ++k ) {
				vector[k].iov_base = data_buffer[k];
				vector[k].iov_len = (buffer_size[i]);
			}

			if ( writev( socket_descriptor, vector, number_buffers[i])  < 0 ) {
				std::cerr << "write error";
				std::cout << std::strerror(errno) << std::endl;
			}
			lap_reset = steady_clock::now();
			lap_time = duration_cast<duration<double>>(lap_reset - lap_start);
			total_lap_time += lap_time.count();
		}

		if (read(socket_descriptor, diagnostic_buffer, sizeof(diagnostic_buffer)) < 0) {
			std::cout << "FAILURE!" << std::endl;
		}
		else {
			std::cout << "SUCCESS!" << std::endl;
		}
		lap_reset = steady_clock::now();
		lap_time = duration_cast<duration<double>>(lap_reset - lap_start);
		total_lap_time += lap_time.count();

		std::cout << "buffer[" << number_buffers[i] << "][" << buffer_size[i] << "]" << std::endl;

		PrintResults( i, 1, total_lap_time );

	}
}

void SingleWrite(const int socket_descriptor) {
	// useful for this design -
	// each buffer configuration is tested
	// for each write strategy
	int number_buffers[4] = { 15, 30, 60, 100 };
	int buffer_size[4] = { 100, 50, 25, 15 };
	char diagnostic_buffer[256];
	std::cout << "--------- Single Write() -----------" << std::endl;
	// using chrono class for timing
	// declare necessary time points
	// Per the instructions, server tracks
	// 'lap' time, which is read to be
	// time/1write
	steady_clock::time_point lap_start;
	steady_clock::time_point lap_reset;
	duration<double> lap_time;
	double total_lap_time = 0;
	std::cout <<  " Single Write \n";
	for( int i = 0; i < 4; ++i ) {
		char data_buffer[number_buffers[i]][buffer_size[i]];
		for( int j = 0; j < REPETITIONS; ++j ) {

			lap_start = steady_clock::now();

			if ( write( socket_descriptor, data_buffer, sizeof(data_buffer) ) < 0 ) {
				std::cerr << "write error";
				std::cout << std::strerror(errno) << std::endl;
			}

			lap_reset = steady_clock::now();
			lap_time = duration_cast<duration<double>>(lap_reset - lap_start);
				total_lap_time += lap_time.count();
		}

		if (read(socket_descriptor, diagnostic_buffer, sizeof(diagnostic_buffer)) < 0) {
			std::cout << "FAILURE!" << std::endl;
		}
		else {
			std::cout << "SUCCESS!" << std::endl;
		}

		lap_reset = steady_clock::now();
		lap_time = duration_cast<duration<double>>(lap_reset - lap_start);
		total_lap_time += lap_time.count();

		std::cout << "buffer[" << number_buffers[i] << "][" << buffer_size[i] << "]" << std::endl;
		PrintResults( i, 2, total_lap_time );

	}
}


// modes 15/100 , 30/50, 60/25, 100/15
void DoWorkSon( const int mode, const int socket_descriptor ) {
	switch(mode) {
		// Multiple Reads
		case 0 :
			std::cout << "case 0 \n";
			MultipleWrites( socket_descriptor );
			break;

		// ReadV()
		case 1 :
			std::cout << "case 1 \n";
			WriteVee( socket_descriptor );
			break;

		//Single Reads
		case 2 :
			std::cout << "case 2 \n";
			SingleWrite( socket_descriptor );
			break;

		//full report
		case 3 :
			std::cout << "case 3 \n";
			MultipleWrites( socket_descriptor );
			WriteVee( socket_descriptor );
			SingleWrite( socket_descriptor );
			break;

		// default will only be called if the mode
		// is out of range - mode is validated on client
		// side, so this was to test the char -> int
		// conversion
		default :
			std::cerr << "Mode data corrupted.";
			close(socket_descriptor);
			break;
		}
}



int main(int argc, char* argv[]) {

	// program validation
	if (argc != 2) {
		std::cerr << " Two arguments expected - abort \n";
		exit(1);
	}

	int port = atoi(argv[1]);

	//open a stream-oriented socket
	int server_socket_descriptor;
	server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);

	if ( server_socket_descriptor < 0 ) {
		std::cerr << "socket";
		std::cout << std::strerror(errno) << std::endl;
	}

	// create a zero-initialized socket address
	struct sockaddr_in acceptSockAddr;
	bzero((char*)&acceptSockAddr, sizeof(acceptSockAddr));
	acceptSockAddr.sin_family		= AF_INET;
	acceptSockAddr.sin_addr.s_addr	= htonl(INADDR_ANY);
	acceptSockAddr.sin_port			= htons(port);

	// set so_reuseaddr option and validate
	const int reuse_opt_on = 1;
	if (setsockopt(server_socket_descriptor, SOL_SOCKET,
		SO_REUSEADDR, (char *)&reuse_opt_on, sizeof(int)) < 0)
	{
		std::cerr << "setsockopt fail";
		std::cout << std::strerror(errno) << std::endl;
		exit(1);
	}


	// bind socket to its local address by calling bind
	//  and validate
	if (bind(server_socket_descriptor, (struct sockaddr*)&acceptSockAddr,
		sizeof(acceptSockAddr)) < 0)
	{
		std::cerr << "bind fail";
		std::cout << std::strerror(errno) << std::endl;
		exit(1);
	}

	//	Instruct os to listen to up to 5
	//		connection request from clients at once
	if (listen(server_socket_descriptor, 5) < 0) {
		std::cerr << "listen fail";
		std::cout << std::strerror(errno) << std::endl;
		exit(1);
	}

	// Receive a request from client by calling accept
	//		which will return a new socket specific to
	//		this connection request
	struct sockaddr_in new_socket_address;
	socklen_t new_socket_address_size = sizeof(new_socket_address);
	int new_socket_descriptor = accept(server_socket_descriptor, (struct sockaddr *)&new_socket_address, &new_socket_address_size);

	// buffers for initial configuration and welcome message.
	char buffer_data_in[256];
	char buffer_welcome[256] = "Welcome to the server\n";

	// write welcome message
	if ( write(new_socket_descriptor, buffer_welcome, sizeof(buffer_welcome)) < 0 ) {
		std::cerr << "write error " << std::endl;
		std::cout << std::strerror(errno) << std::endl;
	}

	// retrieve test mode
	if ( read(new_socket_descriptor, buffer_data_in, sizeof(buffer_data_in)) < 0 ) {
		std::cerr << "read error " << std::endl;
		std::cout << std::strerror(errno) << std::endl;
	}

	// convert char to int for use in test mode
	int mode = *buffer_data_in;
	mode -= 48;

	// call function to write to client and print metrics
	DoWorkSon( mode, new_socket_descriptor );

	// close the connection
	close(new_socket_descriptor);

	return 0;
}
