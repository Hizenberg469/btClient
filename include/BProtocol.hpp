#ifndef __B_PROTOCOL
#define __B_PROTOCOL

#include "torrent_structure.hpp"
#include <Torrent.hpp>


/*************************B_Protocol literals*********************/



#define DictionaryStart 'd'
#define DictionaryEnd	'e'
#define ListStart		'l'
#define ListEnd			'e'
#define NumberStart		'i'
#define NumberEnd		'e'
#define ByteArrayDivider ':'


/*************************B_Protocol literals*********************/


/*************************Decode Functions Implementation*************************/

/*Implemented*/
long long _read_file(const std::string& torrentFile, char*& raw_data);

/*Implemented*/
bNode* b_decode(char*& raw_data, long long &size);

/*Implemented*/
long long int integer_decode(char*& raw_data, long long int &size);

/*Implemented*/
std::string string_decode(char*& raw_data, long long int& size);



/*************************Decode Functions Implementation*************************/


/*************************Encode Functions Implementation*************************/


/* Always decrease the return size of e_data by 1, after processing*/
int b_encode(bNode* tFile, char*& e_data, long long& size);



/*************************Encode Functions Implementation*************************/



//void TorrentInfoDump(bNode* res, TorrentInfo* &torrentInfo);

#endif