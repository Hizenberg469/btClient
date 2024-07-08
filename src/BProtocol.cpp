#include <iostream>
#include <fstream>
#include <BProtocol.hpp>
#include <torrent_structure.hpp>

#include <debug.hpp>


/*************************Decode Functions Implementation*************************/

long long _read_file(const std::string& torrentFile, char*& raw_data) {

	long long size;

	std::ifstream t_file(torrentFile, std::ios::in | std::ios::binary | std::ios::ate);

	if (t_file.is_open()) {

		size = (std::streampos)t_file.tellg();

		//std::cout << "Size of file : " << size << " bytes\n";

		raw_data = new char[size+1LL];
		t_file.seekg(0, std::ios::beg);

		t_file.read(raw_data, size);

		t_file.close();

		std::cout << "Raw Data reading from torrent file complete!!!!\n";

		return size;
	}
	else {

		std::cout << "Unable to open torrent file to read raw data\n";
	}

	return -1;
}

long long int integer_decode(char*& raw_data, long long int &size)
{
	long long int res = 0;

	while ((*raw_data) != 'e')
	{
		res = res * 10 + ((*raw_data) - 48);
		raw_data++;
		size--;
	}

	//now I am at e , since it is useless lets increment this too;
	raw_data++;
	size--;

	return res; //retrun the converted data;


}

std::string string_decode(char*& raw_data, long long int& size) {
	//to parse the string we first need to extract the length

	int length = 0;
	while ((*(raw_data)) != ':')
	{
		length = length * 10 + ((*raw_data) - 48);
		//std::cout << ((*raw_data));
		(raw_data)++;
		size--;
	}

	//std::cout << "Length:" << length << std::endl;


	//now I have length;
	(raw_data)++; //skipping the ":"
	size--;
	std::string value = "";

	while (length--)
	{
		value = value + (*raw_data);
		(raw_data)++;
		size--;
	}

	//(raw_data)++;
	//size--;

	return value;
}

bNode* b_decode(char* &raw_data, long long &size) {

	if (size == 0) {
		
		return NULL;
	}

	bNode* curNode = NULL;
	bNode* head;
	//bNode* value = NULL;

	switch (*raw_data) {

	case 'd': {

	
		curNode = new bNode;
		curNode->type = B_DICTIONARY;
		curNode->value.dict = new bDictionary;


		//for skipping 'd' character...

		(raw_data)++;
		size--;
		//now to decode the value , but value can be anything 

		while ((*raw_data) != 'e') {
			/* Parsing key */
			std::string key = string_decode((raw_data), size);

			bDictionaryNode* res = new bDictionaryNode;

			res->key = key;

			res->value = b_decode(raw_data, size);

			if (curNode->value.dict->count == 0) {
				curNode->value.dict->head = res;
				curNode->value.dict->tail = res;
				curNode->value.dict->count++;
				continue;
			}

			curNode->value.dict->tail->next = res;
			curNode->value.dict->tail = curNode->value.dict->tail->next;
			curNode->value.dict->count++;

		}

		//for skipping 'e'
		

		(raw_data)++;
		size--;

		return curNode;

		break;
	}
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	{
		std::string res = string_decode((raw_data), size);
		curNode = new bNode;
		curNode->type = B_STRING;
		curNode->value.str = res;

		

		return curNode;
		break;
	}
	case 'l': {

		//for skipping 'l' character...
		(raw_data)++;
		size--;

		curNode = new bNode;
		curNode->type = B_LIST;
		curNode->value.list = new bList;

		while ((*raw_data) != 'e') {
			bListNode* res = new bListNode;

			res->value = b_decode(raw_data, size);

			if (curNode->value.list->count == 0) {
				curNode->value.list->head = res;
				curNode->value.list->tail = res;
				curNode->value.list->count++;
				continue;
			}

			curNode->value.list->tail->next = res;
			curNode->value.list->tail = curNode->value.list->tail->next;
			curNode->value.list->count++;

		}
		
		//Escaping the 'e' character...
		raw_data++;
		size--;

		return curNode;
		break;
	}
	case 'i': {
		//now I have encountered an i;
		raw_data++; //now I have skipped i;
		size--;
		long long int integer = integer_decode(raw_data, size);

		curNode = new bNode;
		curNode->type = B_INTEGER;
		curNode->value.number = integer;
		

		return curNode;

		break;
	}
	default: {
		error_msg("data-type found is unknow while parsing.\n",
			__FUNCTION__);
		break;
	}


	}

	return curNode;
}

/*************************Encode Functions Implementation*************************/


void encode_integer(char*& e_data, long long& size, long long num) {

	
	std::string res = "";

	res += 'i';
	res += std::to_string(num);
	res += 'e';

	long long sz = (long long)res.size();

	for (long long i = 0; i < sz; i++) {
		e_data[size++] = res[i];
	}

	return;
}

void encode_string(char*& e_data, long long& size, const std::string& str) {

	long long len = (long long)str.size();

	std::string res = "";


	res += std::to_string(len);
	res += ':';
	res += str;

	long long sz = (long long)res.size();

	for (long long i = 0; i < sz; i++) {
		e_data[size++] = res[i];
	}

	return;
}

int b_encode(bNode* tFile, char*& e_data, long long& size) {

	b_type ty = tFile->type;
	switch (ty) {

	case B_DICTIONARY: {

		e_data[size] = 'd';
		size++;

		bDictionaryNode* curr = tFile->value.dict->head;
		std::string key;

		while (curr != NULL) {

			key = curr->key;
			encode_string(e_data,size, key);

			b_encode(curr->value, e_data, size);
			curr = curr->next;

		}

		e_data[size] = 'e';
		size++;

		return 1;
		break;
	}
	case B_LIST: {

		e_data[size] = 'l';
		size++;

		bListNode* curr = tFile->value.list->head;

		while (curr != NULL) {

			b_encode(curr->value, e_data, size);
			curr = curr->next;

		}

		e_data[size] = 'e';
		size++;

		return 1;
		break;
	}
	case B_STRING: {

		encode_string(e_data, size, tFile->value.str);

		return 1;
		break;
	}
	case B_INTEGER: {

		encode_integer(e_data, size, tFile->value.number);

		return 1;

		break;
	}
	default: {

		return -1;
		break;
	}
	}


	return -1;
}


/*************************Encode Functions Implementation*************************/


//void TorrentInfoDump(bNode* res, TorrentInfo*& torrentInfo)
//{
//	b_type ty = res->type;
//	switch (ty)
//	{
//	case B_DICTIONARY:
//	{
//		bDictionaryNode* curr = res->value.dict->head;
//		std::string key;
//
//		while (curr != NULL)
//		{
//			//announce would always be a string!!!
//			key = curr->key;
//
//			if (key == "info")
//			{
//				torrentInfo->start = curr->value->value.start;
//				torrentInfo->end = curr->value->value.end;
//			}
//
//			if (key == "announce") {
//				torrentInfo->announce = curr->value->value.str;
//			}
//			if (key == "comment")
//			{
//				torrentInfo->comment = curr->value->value.str;
//			}
//
//			if (key == "created by")
//			{
//				torrentInfo->created_by = curr->value->value.str;
//			}
//
//			if (key == "creation date")
//			{
//				torrentInfo->creation_date = curr->value->value.number;
//
//			}
//
//			if (key == "encoding")
//			{
//				torrentInfo->encoding = curr->value->value.str;
//			}
//
//			if (key == "piece length")
//			{
//				torrentInfo->pieceSize = curr->value->value.number;
//			}
//
//			if (key == "length")
//			{
//				torrentInfo->length = curr->value->value.number;
//			}
//
//			if (key == "pieces")
//			{
//				long long int num_pieces = (torrentInfo->length / torrentInfo->pieceSize) + 1;
//
//				torrentInfo->pieceHashes = new char* [num_pieces];
//
//				std::cout << "NumPieces:" << num_pieces << "\n";
//
//				for (long long int i = 0; i < num_pieces; i++)
//				{
//					torrentInfo->pieceHashes[i] = new char[20];
//					char* src = ((char*)(&curr->value->value.str) + (20 * i));
//					for (long long int j = 0; j < 20; j++) {
//						torrentInfo->pieceHashes[i][j] = curr->value->value.str[j + 20 * i];
//					}
//
//				}
//
//			}
//
//
//			//	if(key=="")
//
//
//
//
//			TorrentInfoDump(curr->value, torrentInfo);
//			curr = curr->next;
//		}
//		return;
//
//	}
//	case B_STRING:
//	{
//
//		return;
//	}
//
//	case B_LIST: {
//		bListNode* curr = res->value.list->head;
//		while (curr != NULL)
//		{
//			//torrentInfo->Files=curr->value->value.
//			TorrentInfoDump(curr->value, torrentInfo);
//			curr = curr->next;
//		}
//		return;
//	}
//
//
//
//	default: {
//		return;
//	}
//
//
//	}
//}
//

//
//void TorrentInfoDump(bNode* res, TorrentInfo*& torrentInfo)
//{
//	b_type ty = res->type;
//	switch (ty)
//	{
//	case B_DICTIONARY:
//	{
//		bDictionaryNode* curr = res->value.dict->head;
//		std::string key;
//
//		while (curr != NULL)
//		{
//			//announce would always be a string!!!
//			key = curr->key;
//
//			if (key == "info")
//			{
//				torrentInfo->start = curr->value->value.start;
//				torrentInfo->end = curr->value->value.end;
//			}
//
//			if (key == "announce") {
//				torrentInfo->announce = curr->value->value.str;
//			}
//			if (key == "comment")
//			{
//				torrentInfo->comment = curr->value->value.str;
//			}
//
//			if (key == "created by")
//			{
//				torrentInfo->created_by = curr->value->value.str;
//			}
//
//			if (key == "creation date")
//			{
//				torrentInfo->creation_date = curr->value->value.number;
//
//			}
//
//			if (key == "encoding")
//			{
//				torrentInfo->encoding = curr->value->value.str;
//			}
//
//			if (key == "piece length")
//			{
//				torrentInfo->pieceSize = curr->value->value.number;
//			}
//
//			if (key == "length")
//			{
//				torrentInfo->length = curr->value->value.number;
//			}
//
//			if (key == "pieces")
//			{
//				long long int num_pieces = (torrentInfo->length / torrentInfo->pieceSize) + 1;
//
//				torrentInfo->pieceHashes = new char* [num_pieces];
//
//				std::cout << "NumPieces:" << num_pieces << "\n";
//
//				for (long long int i = 0; i < num_pieces; i++)
//				{
//					torrentInfo->pieceHashes[i] = new char[20];
//					char* src = ((char*)(&curr->value->value.str) + (20 * i));
//					for (long long int j = 0; j < 20; j++) {
//						torrentInfo->pieceHashes[i][j] = curr->value->value.str[j + 20 * i];
//					}
//
//				}
//
//			}
//
//
//			//	if(key=="")
//
//
//
//
//			TorrentInfoDump(curr->value, torrentInfo);
//			curr = curr->next;
//		}
//		return;
//
//	}
//	case B_STRING:
//	{
//
//		return;
//	}
//
//	case B_LIST: {
//		bListNode* curr = res->value.list->head;
//		while (curr != NULL)
//		{
//			//torrentInfo->Files=curr->value->value.
//			TorrentInfoDump(curr->value, torrentInfo);
//			curr = curr->next;
//		}
//		return;
//	}
//
//
//
//	default: {
//		return;
//	}
//
//
//	}
//}
//
//