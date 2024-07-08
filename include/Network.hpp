#ifndef __NETWORK__


#define __NETWORK__

//Networking Feature;
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <assert.h>
#include <Torrent.hpp>
#include <arpa/inet.h>


//178.17.4.234:51413
//78.159.131.21:42363
//136.54.172.182:50413

//91.103.253.141:1

//84.17.63.56:44320

//122.162.144.112:32927

//122.162.144.112:32927  helloworld.txt.to
//182.69.177.39::9001

#define DEST_PORT            9001
#define SERVER_IP_ADDRESS   "182.69.183.180"


#define BT_CHOKE 0
#define BT_UNCHOKE 1
#define BT_INTERSTED 2
#define BT_NOT_INTERESTED 3
#define BT_HAVE 4
#define BT_BITFIELD 5
#define BT_REQUEST 6
#define BT_PIECE 7
#define BT_CANCEL 8


typedef struct {
	char* bitfield;
	size_t size;
}bt_bitfield_t;

typedef struct {
	int index;
	int begin;
	int length;
}bt_request_t;

typedef struct {
	int index;
	int begin;
	char piece[0];
}bt_piece_t;

typedef struct bt_msg {
	int length;
	unsigned char bt_type;

	union {
		bt_bitfield_t bitfield;
		int have;
		bt_piece_t piece;
		bt_request_t request;
		bt_request_t cancel;
		char data[0];
	}payload;


}bt_msg_t;

void start_client_handshake(int& sockfd,char handshake[],Torrent *torrent);
char* buildHandShake(Torrent *torrent);




#endif