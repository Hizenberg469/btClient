#include "Network.hpp"
#include<string>
#include<iostream>


/*HANDHSAKE BUILD*/
char* buildHandShake(Torrent* torrent)
{
	
    //handshake: <pstrlen><pstr><reserved><info_hash><peer_id> [wikitheory.org]

    const char* pstr = "BitTorrent protocol";
    unsigned char pstrlen = strlen(pstr);
    const char reserved[8] = { 0 };
    const char* g_local_peer_id = "-MY0001-123456654321";

    size_t bufflen = 1 + pstrlen + sizeof(reserved) + 20 + strlen(g_local_peer_id);

    off_t off = 0;
    char* buff = (char*)malloc(bufflen);

    buff[0] = pstrlen;
    off++;

    memcpy(buff + off, pstr, pstrlen);
    off += pstrlen;
    assert(off == 20);

    memcpy(buff + off, reserved, sizeof(reserved));
    off += sizeof(reserved);
    assert(off == 28);

    memcpy(buff + off, torrent->Infohash, 20);

    off += 20;
    memcpy(buff + off, g_local_peer_id, strlen(g_local_peer_id));
	



	return buff;


}

int sendData(int sockfd,char handshake[],ssize_t len)
{
    ssize_t tot_sent = 0;
   // ssize_t len = 68;

    while (tot_sent < len) {
        ssize_t sent = send(sockfd, handshake, len - tot_sent, 0);
        if (sent < 0)

        {
            std::cout << "No data sent\n";
            return -1;
        }

        std::cout << "Sent:" << sent << '\n';

        tot_sent += sent;
        handshake += sent;
    }
    return 1;
}



int receiveData(int sockfd, ssize_t len, char* buff, int flag)
{
    unsigned tot_recv = 0;
    ssize_t nb=0;
  //  char buff[len];
    if (len == 0) {
        std::cout << "No data available" << '\n';
        return -1;
    }

    do {
        assert(len - tot_recv > 0);
        nb = recv(sockfd, buff + tot_recv, len - tot_recv, 0);
        if (nb <= 0) {
            std::cout << "No data received" << '\n';
            return -1;
        }
        std::cout << "Received in chunks :" << nb << "\n";
    
        tot_recv += nb;

    } while (nb > 0 && tot_recv < len);

    if (tot_recv == len) {
        std::cout << "Received All" << '\n';
        return 1;
        //  return;
    }

    return -1;
}


void start_client_handshake(int& sockfd,char handshake[],Torrent* torrent)
{
    struct sockaddr_in server_addr;

    char buffer[68];
    // Create a socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return;
    }

    // Initialize server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEST_PORT);

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP_ADDRESS, &server_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return;
    }

    // Connect to the server
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        return;
    }

    // Send the handshake message

    
    assert(sendData(sockfd, handshake, 68) == 1);
    
    printf("Handshake message sent successfully\n");


    char* buf = (char*)malloc(68);
    int data = receiveData(sockfd, 68,buf,0);
    assert(data==1);

    std::cout << "Received Handshake"<<'\n';
    
    //Receive BitField
       
    memset(buffer, 0, sizeof(buffer));
    bt_msg_t Recvmsg;

    int n_pieces = torrent->PieceCount();
    int sizeRmsg = sizeof(Recvmsg.length) + sizeof(Recvmsg.bt_type) + (sizeof(char) * n_pieces);

    std::cout << "sizeRmsg:" << sizeRmsg << '\n';

    // Interested Message
    bt_msg_t Sendmsg;
    Sendmsg.length = sizeof(Sendmsg.bt_type);
    Sendmsg.bt_type = BT_INTERSTED;
    assert(sendData(sockfd, (char*)&Sendmsg, sizeof(Sendmsg)) == 1);



    //Piece-Request
    Sendmsg;
    Sendmsg.bt_type = BT_REQUEST;
    Sendmsg.payload.request.begin = 0;
    Sendmsg.payload.request.index = 0;
    Sendmsg.payload.request.length = 4;
    Sendmsg.length = sizeof(Sendmsg.bt_type)+sizeof(Sendmsg.payload);
   
   
    assert(sendData(sockfd, (char*)&Sendmsg, sizeof(Sendmsg)) == 1);
   //PieceFetch
    do {
        receiveData(sockfd, 8, buffer, 1);
        memcpy(&Recvmsg, buffer, sizeof(Sendmsg));
        memset(buffer, 0, sizeof(buffer));
    } while (Recvmsg.bt_type!=BT_PIECE);

    std::cout << "\nTYPE:" << (int)Recvmsg.bt_type;
    std::cout <<"\nsize:" << Recvmsg.payload.bitfield.size;

    // Close the socket
    close(sockfd);
}

























