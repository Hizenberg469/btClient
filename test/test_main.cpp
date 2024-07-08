#include <iostream>
#include <fstream>
#include <istream>
#include <string>
#include <string>
#include <sstream>
#include <iomanip>
#include <BProtocol.hpp>
#include <Torrent.hpp>
#include <debug.hpp>
#include <Network.hpp>
#include <curl/curl.h>
#include <openssl/sha.h>




int main(int argc, char** argv) {

    //Step 1 : Specify the .torrent file.
    std::string torrentFile;
    if (argc > 1) {
        torrentFile.assign(argv[1]);

        //std::cout << "torrent file : " << torrentFile << '\n';
    }
    else {
        error_msg("Torrent File not specified.\n\
                Please specify Torrent file (.torrent)\n",
            __FUNCTION__);
    }

    //Step2 : Read the .torrent file in raw byte.
    char* rawData;
    long long int tFilesize = _read_file(torrentFile, rawData);

    char* buffer=NULL; //buffer for storing the decoded data
    buffer = new char[tFilesize];

    long long sz = tFilesize;

    bNode* reso = b_decode(rawData, sz);

    reso->size = tFilesize;

    std::cout << "Decode successfull!!!!\n";


    Torrent* torrent = new Torrent();
    torrent = torrent->BEncodingObjectToTorrent(reso, torrentFile,
        "/");


    torrent->PeerListUpdated.appendListener("peerlist",
        [](std::vector<std::string>& peers) {
            std::cout << "Peer List Updated:\n";
            for (std::string peer : peers) {
                std::cout << peer << '\n';
            }
        });

    torrent->Trackers[0]->Update(*torrent, Started,
        "76433642664923430920", 9001);

 /*   std::cout << "Info Hash : " <<
        torrent->HexStringInfoHash() << '\n';*/


    char* handshake = buildHandShake(torrent);

    std::cout << "\nMY HANDSHAKE\n";
    for (int i = 0; i < 68; i++)
    {
    std::cout << handshake[i];
    }
    std::cout << "\n-------------------------\n";



    int sockfd;
    start_client_handshake(sockfd,handshake,torrent);

    delete torrent;

    return 0;

}
