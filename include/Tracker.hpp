#ifndef __TRACKER_H
#define __TRACKER_H

//To use POSIX TIMER library
#define __USE_POSIX199309 1

#include <time.h>
#include <signal.h>
#include <string>
#include <sstream>
#include <list>
#include <curl/curl.h>
#include <arpa/inet.h>
#include <Torrent.hpp>
#include <stdint.h>
#include <eventpp/eventdispatcher.h>


typedef enum {
	Started,
	Paused,
	Stopped
} TrackerEvent;


class Torrent;
class Tracker {
public:

	eventpp::EventDispatcher<std::string,
		void(std::vector<std::string>&)> PeerListUpdated;

	std::string Address;
	struct timespec LastPeerRequested;
	long long int PeerRequestInterval; //In seconds
	//The Response from tracker...
	std::string httpWebRequest;

	Tracker(std::string address);

	void Request(char* url);

	void Update(Torrent torrent, TrackerEvent ev, std::string id,
		int port);


	//To Implment response from tracker....

	void Handle_response();
	void ResetLastRequest();

	std::string returnTracker();
};
#endif