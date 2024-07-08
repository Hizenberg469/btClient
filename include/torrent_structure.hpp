#ifndef __TORRENT_STRUCT_H
#define __TORRENT_STRUCT_H


#include <string>
#include <map>
#include <vector>
#include<cmath>
#include <list>

/*************************** (.torrent) content structure for parsing*************************/

typedef enum {

	B_STRING,
	B_INTEGER,
	B_DICTIONARY,
	B_LIST,
	B_UNKNOWN

}b_type;

class bDictionary;
class bList;
class bListNode;
class bDictionaryNode;

typedef bDictionary TorrentFile;

class bNode {

public:

	b_type type;
	long long int size;

	class val {
	public:

		std::string str;
		long long number;
		bDictionary* dict;
		bList* list; //Pointer to array of pointer of bNode...
		uint64_t start;
		uint64_t end;

		val();
	};

	val value;

	bNode();
};

class bList {
public:
	bListNode* head;
	bListNode* tail;
	int count;

	bList();

	std::vector<bNode*> to_Stl_list();
};

class bListNode {
public:

	bNode* value;
	bListNode* next;

	bListNode();
};


class bDictionary {
public:
	bDictionaryNode* head;
	bDictionaryNode* tail;
	int count;

	bDictionary();

	std::map<std::string,bNode*> to_Stl_map();
};



class bDictionaryNode {

public:
	std::string key;
	bNode* value;
	bDictionaryNode* next;

	bDictionaryNode();
};

/*************************** (.torrent) content structure for parsing*************************/




//
//class TorrentInfo {
//
//public:
//	std::string announce;
//	std::string comment;
//	std::string created_by;
//	long long int creation_date;
//	std::string encoding;
//
//
//	uint64_t start;
//	uint64_t end;
//
//	long long int length;
//	std::string name;
//	long long int blockSize;
//	long long int pieceSize;
//
//
//	char** pieceHashes;
//
//	std::string DownloadDirectory;
//	std::string FileDirectory;
//
//	//std::list<FileItem>Files;
//
//
//
//
//};




#endif