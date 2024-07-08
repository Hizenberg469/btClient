#ifndef __DEBUG__
#define __DEBUG__
#include <string>
#include <iostream>
#include <torrent_structure.hpp>

inline void error_msg(std::string msg,const char* function_name) {
	std::cout << "Error:\n";
	std::cout << msg << '\n';
	std::cout << "Error in Function : " << 
		std::string(function_name) << '\n';
}


inline void _dump_parsed_torrent_file_data(bNode* tFile) {

	b_type ty = tFile->type;

	switch (ty) {
	case B_DICTIONARY: {

		int d_size = tFile->value.dict->count;

		bDictionaryNode* curr = tFile->value.dict->head;

		while (curr != NULL) {

			std::cout << curr->key << ' ';

			_dump_parsed_torrent_file_data(curr->value);
			std::cout << '\n';

			curr = curr->next;
		}
		break;
	}
	case B_LIST: {

		int l_size = tFile->value.list->count;

		bListNode* curr = tFile->value.list->head;

		while (curr != NULL) {

			_dump_parsed_torrent_file_data(curr->value);
			std::cout << '\n';

			curr = curr->next;
		}
		break;
	}
	case B_STRING: {

		std::cout << tFile->value.str << '\n';
		break;
	}
	case B_INTEGER: {

		std::cout << tFile->value.number << '\n';
		break;
	}
	case B_UNKNOWN: {

		error_msg("Data-type doesn't belong to .torrent file \
syntax.\n\
.torrent file is broken.\n", __FUNCTION__);
	}

	}
}
#endif