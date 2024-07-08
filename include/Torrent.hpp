#ifndef __TORRENT_H
#define __TORRENT_H

//To use POSIX TIMER library
#define __USE_POSIX199309 1

#include <time.h>
#include <signal.h>
#include <iterator>
#include <vector>
#include <string>
#include <string.h>
#include <math.h>
#include <filesystem>
#include <fstream>
#include <stdlib.h>
#include <curl/curl.h>
#include <openssl/sha.h>
#include <Tracker.hpp>
#include <BProtocol.hpp>
#include <torrent_structure.hpp>
#include <eventpp/eventdispatcher.h>

#ifdef _WIN32
#define DIR_SEPARATOR '\\'
#else
#define DIR_SEPARATOR '/'
#endif

class FileItem {
public:

	std::string Path;
	long long int Size;
	long long int Offset;

	std::string FormattedSize;

	FileItem();
	void set_FormattedSize();

};

class Tracker;
class Torrent {
public:

	eventpp::EventDispatcher <std::string, void(
		std::vector<std::string>&)> PeerListUpdated;

	long long int torrentFileSize;

	std::string Name;
	int IsPrivate;
	std::vector<FileItem> Files;
	std::string FileDirectory();
	std::string DownloadDirectory;

	std::vector<Tracker*> Trackers;
	std::string Comment;
	std::string CreatedBy;
	time_t CreationDate;
	std::string Encoding;

	int BlockSize;
	int PieceSize;
	long long int TotalSize();

	std::string FormattedPieceSize();
	std::string FormattedTotalSize();

	int PieceCount();

	std::vector<std::vector<unsigned char>> PieceHashes;
	std::vector<bool> IsPieceVerified;
	std::vector<std::vector<bool>> IsBlockAcquired;

	std::string VerifiedPiecesString();
	int VerifiedPieceCount();
	double VerifiedRatio();
	bool IsCompleted();
	bool IsStarted();

	long long int Uploaded;
	long long int Downloaded();
	long long int Left();


	char Infohash[20];
	std::string HexStringInfoHash(); //Not Implemented...
	std::string UrlSafeStringInfoHash();

	//Locking...

	Torrent();
	Torrent(std::string name, std::string location,
		std::vector<FileItem> files,
		std::vector<std::string> trackers,
		int pieceSize, char* pieceHashes,
		int blockSize,
		int isPrivate);

	void UpdateTrackers(TrackerEvent ev,
		std::string id, int port);
	void ResetTrackerLastRequest();

	bNode* TorrentToBEncodingObject(Torrent torrent);
	bDictionary* TorrentInfoToBEncodingObject(Torrent torrent);
	Torrent* BEncodingObjectToTorrent(bNode* bencoding,
		std::string name, std::string downloadPath);
	//Encoding and creationdate left to implement...

	void Verify(int piece);
	int GetPieceSize(int piece);
	int GetBlockCount(int piece);
	std::vector<unsigned char> GetHash(int piece);
	std::string ReadPiece(int piece);
	std::string Read(long long int start, int length);

	Torrent* LoadFromFile(std::string filePath, std::string downloadPath);
};


#endif