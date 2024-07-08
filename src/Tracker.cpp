#include <stdio.h>
#include <cstring>
#include <torrent_structure.hpp>
#include <BProtocol.hpp>
#include <debug.hpp>


Tracker::Tracker(std::string address) {
	this->Address = address;
	this->LastPeerRequested = { 0,0 };
	this->PeerRequestInterval = 1800;
	this->httpWebRequest = "";
}

void Tracker::Update(Torrent torrent, TrackerEvent ev, 
	std::string id, int port) {
	struct timespec curr_time;

	clock_gettime(CLOCK_REALTIME, &curr_time);

	long long int time_check = ((double)curr_time.tv_sec +
		(double)curr_time.tv_nsec / 1.0e9);

	long long int time_limit = ((double)LastPeerRequested.tv_sec +
		(double)LastPeerRequested.tv_nsec / 1.0e9) + 
		this->PeerRequestInterval;

	if (ev == Started && time_check < time_limit)
		return;

	clock_gettime(CLOCK_REALTIME, &(this->LastPeerRequested));

	std::string eventEnum;

	switch (ev) {
	case Started: {
		eventEnum.assign("started");
		break;
	}
	case Paused: {
		eventEnum.assign("paused");
		break;
	}
	case Stopped: {
		eventEnum.assign("stopped");
		break;
	}
	default: {
		error_msg("TrackerEvent ev have undefined value\n",
			__FUNCTION__);
	}
	}


	char url[400];
	sprintf(url, "%s?info_hash=%s&peer_id=%s\
&port=%d&uploaded=%lld&\
downloaded=%lld&left=%lld&event=%s&compact=1",
(char*)(this->Address).c_str(), (char*)(torrent.UrlSafeStringInfoHash()).c_str(),
(char*)id.c_str(), port,
torrent.Uploaded, torrent.Downloaded(), torrent.Left(),
(char*)eventEnum.c_str());

	std::string Url(url);

	this->Request((char*)Url.c_str());
	this->Handle_response();
}

// Callback function to write the response data
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t totalSize = size * nmemb;
	std::string* str = (std::string*)userp;
	str->append((char*)contents, totalSize);
	return totalSize;
}

void Tracker::Request(char* url) {

	CURL* curl;
	CURLcode res;
	//std::string readBuffer;

	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();
	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L); // 30 seconds timeout
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &(this->httpWebRequest));
		// Increase timeout

		// Perform the request
		res = curl_easy_perform(curl);

		// Check for errors
		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
		}
		/*else {
			std::cout << this->httpWebRequest << std::endl;
		}*/

		// Cleanup
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}

void Tracker::Handle_response() {

	long long int sz_response = (long long int)this->httpWebRequest.size();
	char* response = new char[sz_response];

	if (this->httpWebRequest.size() == 0) {
		error_msg("Tracker didn't reply any response!!",
			__FUNCTION__);
		return;
	}

	strncpy(response, this->httpWebRequest.c_str(),
		sz_response);


	bNode* info = b_decode(response,
		sz_response);

	if (sz_response > 0) {
		error_msg("b_decode() didn't parse the\
complete responce\n", __FUNCTION__);
		return;
	}

	bDictionary* dict = info->value.dict;

	std::map<std::string, bNode*>
		dictionary = dict->to_Stl_map();

	this->PeerRequestInterval = dictionary["interval"]->value.number;
	const char* peerInfo = dictionary["peers"]->value.str.c_str();

	std::vector<std::string> peers;

	int offset;
	std::string address;

	uint8_t ip[4];
	uint16_t ip_port;

	for (int i = 0; i < strlen(peerInfo) / 6; i++) {

		offset = i * 6;
		memcpy(ip, peerInfo + offset, 4);
		memcpy(&ip_port, peerInfo + 4, 2);
		ip_port = ntohs(ip_port); //convert port number..
		char ip_str[INET_ADDRSTRLEN];
		if (inet_ntop(AF_INET, ip, ip_str, INET_ADDRSTRLEN) == NULL) {
			error_msg("inet_ntop failed", __FUNCTION__);
			return;
		}

		address.assign(ip_str);
		std::stringstream ss;
		ss << ip_port;

		peers.push_back(address + "::" + ss.str());

		address.clear();
	}

	this->PeerListUpdated.dispatch("peerlist", peers);
}

void Tracker::ResetLastRequest() {
	LastPeerRequested = { 0,0 };
}

std::string Tracker::returnTracker() {
	std::string trackerAddress = "[Tracker : " + this->Address + " ]";
	return trackerAddress;
}