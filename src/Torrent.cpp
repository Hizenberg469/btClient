#include <Torrent.hpp>
#include <debug.hpp>
#include <assert.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <algorithm>


/**********************************************************/

static std::string encodeURl(std::string Info)
{
	//Info Hash is 20byte long , so no stress of extra character at end
	std::string encoded = "";

	encoded += "%";
	for (int i = 0; i < Info.size();)
	{
		if (i + 1 < Info.size())
		{
			encoded += Info[i];
			encoded += Info[i + 1];
		}
		encoded += "%";
		i += 2;
	}
	encoded.pop_back();
	return encoded;
}

/**********************************************************/

static std::vector<unsigned char> sha1(const char* input,
	long long int size);

FileItem::FileItem() {

	this->Path = "";
	this->Size = -1;
	this->Offset = -1;
	this->FormattedSize = "";
}



void FileItem::set_FormattedSize() {
	this->FormattedSize = std::to_string(Size);
}

std::string Torrent::FileDirectory() {
	return this->Files.size() > 1 ? this->Name +
		DIR_SEPARATOR : "";
}

long long int Torrent::TotalSize() {
	long long int totalSize = 0;

	
	for (int i = 0; i < (int)Files.size(); i++) {
		totalSize += Files[i].Size;
	}

	return totalSize;
}

std::string Torrent::FormattedPieceSize() {
	return std::to_string(PieceSize);
}

std::string Torrent::FormattedTotalSize() {
	return std::to_string(this->TotalSize());
}

int Torrent::PieceCount() {
	return (int)this->PieceHashes.size();
}

std::string Torrent::VerifiedPiecesString() {
	std::string verificationString = "";

	for (int i = 0; i < IsPieceVerified.size(); i++) {
		verificationString += IsPieceVerified[i] ? "1" : "0";
	}
	
	return verificationString;
}

int Torrent::VerifiedPieceCount() {
	int count = 0;

	for (int i = 0; i < (int)IsPieceVerified.size(); i++) {
		count += IsPieceVerified[i] ? 1 : 0;
	}
	return count;
}

double Torrent::VerifiedRatio() {
	return this->VerifiedPieceCount() / (double)this->PieceCount();
}

bool Torrent::IsCompleted() {
	return this->VerifiedPieceCount() == this->PieceCount();
}

bool Torrent::IsStarted() {
	return this->VerifiedPieceCount() > 0;
}


long long int Torrent::Downloaded() {
	return this->PieceSize * this->VerifiedPieceCount();
}

long long int Torrent::Left() {
	return this->TotalSize() - this->Downloaded();
}


//To encode string into Url safe string.
static std::string url_encode(const std::string& decoded)
{
	const auto encoded_value = curl_easy_escape(nullptr, decoded.c_str(), static_cast<int>(decoded.length()));
	std::string result(encoded_value);
	curl_free(encoded_value);
	return result;
}

//To decode string into Url safe string.
static std::string url_decode(const std::string& encoded)
{
	int output_length;
	const auto decoded_value = curl_easy_unescape(nullptr, encoded.c_str(), static_cast<int>(encoded.length()), &output_length);
	std::string result(decoded_value, output_length);
	curl_free(decoded_value);
	return result;
}

std::string Torrent::HexStringInfoHash() {
	std::ostringstream oss;
	for (unsigned char byte : this->Infohash) {
		oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
	}
	return oss.str();
}

std::string Torrent::UrlSafeStringInfoHash() {
	std::string infoHash(this->HexStringInfoHash());
	infoHash = url_encode(infoHash);
	return encodeURl(infoHash);
}

Torrent::Torrent() {
	

}

Torrent::Torrent(std::string name, std::string location,
	std::vector<FileItem> files,
	std::vector<std::string> trackers,
	int pieceSize, char* pieceHashes = NULL,
	int blockSize = 16384,
	int isPrivate = -1) {

	this->Name = name;
	this->DownloadDirectory = location;
	this->Files = files;


	//locking....

	this->Trackers.resize(trackers.size());

	if ((int)trackers.size() > 0) {
		Tracker* tracker = NULL;
		int i = 0;
		for (std::string url : trackers) {
			tracker = new Tracker(url);
			tracker->PeerListUpdated.appendListener("peerlist",
				[this](std::vector<std::string>& peerList) {
					this->PeerListUpdated.dispatch("peerlist",
						peerList);
				});
			this->Trackers[i] = tracker;
			i++;
			
			tracker = NULL;
		}
	}


	this->PieceSize = pieceSize;
	this->BlockSize = blockSize;
	this->IsPrivate = isPrivate;

	int count = (int)(std::ceil(this->TotalSize() / 
		(double)this->PieceSize));

	PieceHashes.resize(count);
	IsPieceVerified.resize(count);
	IsBlockAcquired.resize(count);

	for (int i = 0; i < this->PieceCount(); i++) {
		IsBlockAcquired[i].resize(this->GetBlockCount(i));
	}

	if (pieceHashes == NULL) {

		//this is a new torrent so we have to create the hashes form 
		//the files...
		for (int i = 0; i < this->PieceCount(); i++) {
			this->PieceHashes[i] = this->GetHash(i);
		}

	}
	else {

		for (int i = 0; i < this->PieceCount(); i++) {
			this->PieceHashes[i].resize(20);
			
			for (int j = 20 * i; j < 20 + 20 * i; j++) {
				this->PieceHashes[i][j-20*i] = *(pieceHashes + j);
			}
		}
	}

	bDictionary* dict = this->TorrentInfoToBEncodingObject(*this);
	bNode* info = new bNode();
	info->type = B_DICTIONARY;
	info->value.dict = dict;

	//_dump_parsed_torrent_file_data(info);

	char* buf = new char[1048576]; //1MB .torrent file only
	long long int size = 0;

	b_encode(info, buf, size);
	char* buffer = new char[size + 1];

	for (int i = 0; i <=size; i++) {
		buffer[i] = buf[i];
	}

	delete[] buf;

	std::vector<unsigned char> hash = sha1(buffer,size);

	for (int i = 0; i < 20; i++) {
		this->Infohash[i] = hash[i];
	}

	//for (int i = 0; i < this->PieceCount(); i++) {
	//	this->Verify(i);
	//}
}

static bool isHashEqual(const std::vector<unsigned char>& hash1,
	const std::vector<unsigned char>& hash2) {

	assert(hash1.size() == hash2.size());

	for (int i = 0; i < hash1.size(); i++) {
		if (hash1[i] != hash2[i])
			return false;
	}

	return true;
}

static bool areAllBlockPieceAcquired(std::vector<bool>&
	IsBlockAcquired) {

	for (int i = 0; i < IsBlockAcquired.size(); i++) {
		if (!IsBlockAcquired[i]) {
			return false;
		}
	}

	return true;
}

void Torrent::Verify(int piece) {
	
	std::vector<unsigned char> hash = GetHash(piece);

	bool isVerfied = (hash.size() > 0 &&
		isHashEqual(hash, this->PieceHashes[piece]));

	if (isVerfied) {
		this->IsPieceVerified[piece] = true;

		for (int i = 0; i < this->IsBlockAcquired[piece].size(); i++) {
			this->IsBlockAcquired[piece][i] = true;
		}

		return;
	}
	
	IsPieceVerified[piece] = false;

	//reload the entire piece..
	if (areAllBlockPieceAcquired(this->IsBlockAcquired[piece])) {

		for (int j = 0; j < this->IsBlockAcquired[piece].size(); j++) {
			this->IsBlockAcquired[piece][j] = false;
		}
	}
}

int Torrent::GetPieceSize(int piece) {
	
	if (piece == this->PieceCount() - 1) {

		int remainder = this->TotalSize() % this->PieceSize;

		if( remainder != 0 )
			return remainder;
	}

	return this->PieceSize;
}

int Torrent::GetBlockCount(int piece) {

	return (int)(std::ceil(this->GetPieceSize(piece) / 
		(double)this->BlockSize));
}

static std::vector<unsigned char> sha1(const char* input,
	long long int size) {
	// Buffer to hold the hash
	std::vector<unsigned char> hash(SHA_DIGEST_LENGTH);

	// Perform the SHA1 hash
	SHA1((const unsigned char*)input, size, hash.data());

	return hash;
}

std::vector<unsigned char> Torrent::GetHash(int piece) {

	std::string data = this->ReadPiece(piece);

	if (data == "") {
		error_msg("Piece Reading went wrong!", __FUNCTION__);
		return {};
	}

	return sha1((const char*)data.c_str(),
		(long long int)data.size());
}

std::string Torrent::ReadPiece(int piece) {
	return Read(1LL * piece * this->PieceSize, this->GetPieceSize(piece));
}

std::string Torrent::Read(long long int start, int length) {

	long long int end = start + length;

	std::string resbuffer;
	char* buffer = new char[length];

	for (int i = 0; i < (int)Files.size(); i++)
	{
		if ((start < Files[i].Offset && end < Files[i].Offset) ||
			(start > Files[i].Offset + Files[i].Size && end > Files[i].Offset + Files[i].Size))
			continue;

		std::string filePath = DownloadDirectory + 
			DIR_SEPARATOR +
			FileDirectory() + Files[i].Path;

		if (!std::filesystem::exists(filePath)) {
			std::cout << "filePath: " << filePath << "\n";
			error_msg("File don't exist.\
Something went wrong!", __FUNCTION__);
			return "";
		}
		

		long long int fstart = std::max(0LL,
			start - Files[i].Offset);
		long long int fend = std::min(end -
			Files[i].Offset, Files[i].Size);
		int flength = fend - fstart;
		int bStart = std::max(0LL,
			Files[i].Offset - start);

		std::ifstream file(filePath, std::ios::in);

		if (file.is_open()) {
			file.seekg(fstart, std::ios::beg);
			file.read(buffer, flength);
			file.close();
		}
		else {
			error_msg("File didn't open", __FUNCTION__);
			return "";
		}

	}

	resbuffer.assign(buffer);
	delete[] buffer;

	return resbuffer;
}


bNode* Torrent::TorrentToBEncodingObject(Torrent torrent) {

	bNode* dict = new bNode();
	dict->type = B_DICTIONARY;
	dict->value.dict = new bDictionary();

	if ((int)torrent.Trackers.size() == 0) {
		error_msg("Torrent class doesn't contain\
any tracker right now!!", __FUNCTION__);
		return NULL;
	}
	
	bDictionaryNode* dictNode = NULL;
	dictNode = new bDictionaryNode();
	dictNode->key = "announce";
	dictNode->value = new bNode();
	dictNode->value->type = B_STRING;
	dictNode->value->value.str = Trackers[0]->Address;
	
	dict->value.dict->head = 
	dict->value.dict->tail = dictNode;
	dict->value.dict->count++;
	dictNode = NULL;

	if ((int)torrent.Trackers.size() > 1) {
		for (int i = 1; i < (int)torrent.Trackers.size(); i++) {
			dictNode = new bDictionaryNode();
			dictNode->key = "announce";
			dictNode->value = new bNode();
			dictNode->value->type = B_STRING;
			dictNode->value->value.str = Trackers[i]->Address;

			dict->value.dict->tail->next = dictNode;
			dict->value.dict->tail =
				dict->value.dict->tail->next;
			dict->value.dict->count++;
			dictNode = NULL;
		}
	}

	dictNode = new bDictionaryNode();
	dictNode->key = "comment";
	dictNode->value = new bNode();
	dictNode->value->type = B_STRING;
	dictNode->value->value.str = torrent.Comment;

	dict->value.dict->tail->next = dictNode;
	dict->value.dict->tail =
		dict->value.dict->tail->next;
	dict->value.dict->count++;
	dictNode = NULL;

	dictNode = new bDictionaryNode();
	dictNode->key = "created by";
	dictNode->value = new bNode();
	dictNode->value->type = B_STRING;
	dictNode->value->value.str = torrent.CreatedBy;

	dict->value.dict->tail->next = dictNode;
	dict->value.dict->tail =
		dict->value.dict->tail->next;
	dict->value.dict->count++;
	dictNode = NULL;

	dictNode = new bDictionaryNode();
	dictNode->key = "creation date";
	dictNode->value = new bNode();
	dictNode->value->type = B_STRING;
	dictNode->value->value.str = torrent.CreationDate;

	dict->value.dict->tail->next = dictNode;
	dict->value.dict->tail =
		dict->value.dict->tail->next;
	dict->value.dict->count++;
	dictNode = NULL;


	dictNode = new bDictionaryNode();
	dictNode->key = "encoding";
	dictNode->value = new bNode();
	dictNode->value->type = B_STRING;
	dictNode->value->value.str = torrent.Encoding;

	dict->value.dict->tail->next = dictNode;
	dict->value.dict->tail =
		dict->value.dict->tail->next;
	dict->value.dict->count++;
	dictNode = NULL;

	dictNode = new bDictionaryNode();
	dictNode->key = "info";
	dictNode->value = new bNode();
	dictNode->value->type = B_DICTIONARY;
	dictNode->value->value.dict =
		this->TorrentInfoToBEncodingObject(torrent);

	dict->value.dict->tail->next = dictNode;
	dict->value.dict->tail =
		dict->value.dict->tail->next;
	dict->value.dict->count++;
	dictNode = NULL;

	return dict;
}

bDictionary* Torrent::TorrentInfoToBEncodingObject(Torrent torrent) {

	bDictionary* dict = new bDictionary();

	bDictionaryNode* node = new bDictionaryNode();

	if (torrent.Files.size() == 0) {
		error_msg("No file mentioned!", __FUNCTION__);
		return NULL;
	}
	else if (torrent.Files.size() == 1) {

		//node = new bDictionaryNode();
		node->key = "length";
		node->value = new bNode();
		node->value->type = B_INTEGER;
		node->value->value.number = torrent.Files[0].Size;

		dict->head = dict->tail = node;
		dict->count++;
		node = NULL;

		


		node = new bDictionaryNode();
		node->key = "name";
		node->value = new bNode();
		node->value->type = B_STRING;
		node->value->value.str = torrent.Files[0].Path;

		dict->tail->next = node;
		dict->tail = dict->tail->next;
		dict->count++;
		node = NULL;

	}
	else {

		node = new bDictionaryNode();
		node->key = "files";
		node->value = new bNode();
		node->value->type = B_LIST;
		bList* list = new bList();

		bListNode* l_node = NULL;
		bDictionaryNode* n_node = NULL;

		for (int i = 0; i < torrent.Files.size(); i++) {
			l_node = new bListNode();
			l_node->value = new bNode();
			l_node->value->type = B_DICTIONARY;
			l_node->value->value.dict = new bDictionary();

			n_node = new bDictionaryNode();
			n_node->key = "length";
			n_node->value = new bNode();
			n_node->value->type = B_INTEGER;
			n_node->value->value.number = torrent.Files[i].Size;


			l_node->value->value.dict->tail = n_node;
			l_node->value->value.dict->tail =
				l_node->value->value.dict->tail->next;
			l_node->value->value.dict->count++;
			n_node = NULL;

			n_node = new bDictionaryNode();
			n_node->key = "path";
			n_node->value = new bNode();
			n_node->value->type = B_STRING;
			n_node->value->value.str = torrent.Files[i].Path;


			l_node->value->value.dict->head =
				l_node->value->value.dict->tail = n_node;
			l_node->value->value.dict->count++;
			n_node = NULL;

			if (list->count == 0) {
				list->head = list->tail = l_node;
				list->count++;
				continue;
			}

			list->tail->next = l_node;
			list->tail = list->tail->next;
			list->count++;
			l_node = NULL;
		}

		node->value->value.list = list;
		dict->tail->next = node;
		dict->tail = dict->tail->next;
		dict->count++;
		node = NULL;

		node = new bDictionaryNode();
		node->key = "name";
		node->value = new bNode();
		node->value->type = B_STRING;
		node->value->value.str = torrent.FileDirectory();

		dict->tail->next = node;
		dict->tail = dict->tail->next;
		dict->count++;
		node = NULL;
	}

	node = new bDictionaryNode();
	node->key = "piece length";
	node->value = new bNode();
	node->value->type = B_INTEGER;
	node->value->value.number = torrent.PieceSize;

	dict->tail->next = node;
	dict->tail = dict->tail->next;
	dict->count++;
	node = NULL;

	std::string pieces(20 * torrent.PieceCount(), ' ');
	for (int i = 0; i < torrent.PieceCount(); i++) {
		for (int j = 20 * i; j < 20 + 20 * i; j++) {
			pieces[j] = torrent.PieceHashes[i][j - 20LL * i];
		}
	}

	node = new bDictionaryNode();
	node->key = "pieces";
	node->value = new bNode();
	node->value->type = B_STRING;
	node->value->value.str = pieces;

	dict->tail->next = node;
	dict->tail = dict->tail->next;
	dict->count++;
	node = NULL;

	if (torrent.IsPrivate != -1) {
		node = new bDictionaryNode();
		node->key = "private";
		node->value = new bNode();
		node->value->type = B_INTEGER;
		node->value->value.number = torrent.IsPrivate;

		dict->tail->next = node;
		dict->tail = dict->tail->next;
		dict->count++;
		node = NULL;
	}

	return dict;
}

Torrent* Torrent::BEncodingObjectToTorrent(bNode* bencoding,
	std::string name, std::string downloadPath) {

	if (bencoding->type != B_DICTIONARY) {
		error_msg("Decoding .torrent file is not\
 in correct format", __FUNCTION__);
		return NULL;
	}

	std::map<std::string, bNode*> dict = bencoding->
		value.dict->to_Stl_map();
	
	std::vector<std::string> trackers;
	if (dict.find("announce") != dict.end()) {

		if (dict["announce"]->type == B_STRING) {
			trackers.push_back(dict["announce"]->value.str);
		}
		else if (dict["announce"]->type == B_LIST) {
			std::vector<bNode*>
				t_list= dict["announce"]->value.list->to_Stl_list();

			for (int i = 0; i < t_list.size(); i++) {
				if (t_list[i]->type == B_STRING) {
					trackers.push_back(t_list[i]->value.str);
				}
				else {
					error_msg("announce list element \
is wrong", __FUNCTION__);
				}
			}
		}
	}

	if (dict.find("info") == dict.end()) {
		error_msg("info section missing decoded bNode \
object", __FUNCTION__);
		exit(EXIT_FAILURE);
	}

	std::map<std::string, bNode*>
		info = dict["info"]->value.dict->to_Stl_map();

	std::vector<FileItem> files;

	if (info.find("name") != info.end() &&
		info.find("length") != info.end()) {
		FileItem item;
		item.Path = info["name"]->value.str;

		item.Size = info["length"]->value.number;

		files.push_back(item);
	}
	else if( info.find("files") != info.end()){

		long long int running = 0;
		std::vector<bNode*> f_list = info["files"]->value.list
			->to_Stl_list();



		for (bNode* item : f_list) {
			std::map<std::string, bNode*>
				dict = item->value.dict->to_Stl_map();

			if (dict.find("path") == dict.end() &&
				dict.find("length") == dict.end()) {

				error_msg("Incorrent .torrent file specification",
					__FUNCTION__);
				exit(EXIT_FAILURE);
			}

			std::string path = "";

			std::vector<bNode*> l_path = dict["path"]->value
				.list->to_Stl_list();

			for (bNode* l_item : l_path) {
				
				path += (DIR_SEPARATOR +
				l_item->value.str);
			}

			long long int size = (long long int)dict["length"]->value
				.number;


			FileItem fileItem;
			fileItem.Path = path;
			fileItem.Size = size;
			fileItem.Offset = running;

			files.push_back(fileItem);
			
			running += size;
		}
	}
	else {
		error_msg("No file specified in .torrent file",
			__FUNCTION__);
		exit(EXIT_FAILURE);
	}

	if (info.find("piece length") == info.end()) {
		error_msg("piece length no mentioned in torrent file",
			__FUNCTION__);
		exit(EXIT_FAILURE);
	}

	int pieceSize = info["piece length"]->value.number;

	if (info.find("pieces") == info.end()) {
		error_msg("pieces not mentioned in torrent file",
			__FUNCTION__);
		exit(EXIT_FAILURE);
	}

	std::string pieceHashes = info["pieces"]->value.str;

	int IsPrivate = -1;
	if (info.find("private") != info.end()) {
		IsPrivate = info["private"]->value.number;
	}



	Torrent* torrent = new Torrent(name, downloadPath,
		files, trackers, pieceSize, (char*)pieceHashes.c_str(),
		16384, IsPrivate);

	if (dict.find("comment") != dict.end()) {
		torrent->Comment = dict["comment"]->value.str;
	}

	if (dict.find("created by") != dict.end()) {
		torrent->CreatedBy = dict["created by"]->value.str;
	}

	return torrent;

}

void Torrent::UpdateTrackers(TrackerEvent ev, std::string id,
	int port) {

	for (Tracker* t : this->Trackers) {
		t->Update(*this, ev, id, port);
	}
}

void Torrent::ResetTrackerLastRequest() {
	for (Tracker* t : this->Trackers) {
		t->ResetLastRequest();
	}
}

Torrent* LoadFromFile(std::string filePath, std::string downloadPath) {

}