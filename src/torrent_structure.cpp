#include <torrent_structure.hpp>
#include <assert.h>

/*************************** (.torrent) content structure for parsing*************************/

bNode::bNode() {
	this->size = 0;
	this->type = B_UNKNOWN;
}

bNode::val::val() {
	this->str = "";
	this->number = 0;
	this->dict = NULL;
	this->list = NULL;
}

bList::bList() {
	this->head = NULL;
	this->tail = NULL;
	this->count = 0;
}

std::vector<bNode*>
bList::to_Stl_list() {

	bListNode* curr = this->head;
	assert(curr != NULL);

	std::vector<bNode*> resList;

	for (; curr != NULL; curr = curr->next) {
		resList.push_back(curr->value);
	}

	return resList;
}

bListNode::bListNode() {
	this->value = NULL;
	this->next = NULL;
}

bDictionary::bDictionary() {
	head = tail = NULL;
	count = 0;
}

std::map<std::string, bNode*> 
bDictionary::to_Stl_map() {

	bDictionaryNode* curr = this->head;
	assert(curr != NULL);
	
	std::map<std::string, bNode*> resMap;

	for (; curr != NULL; curr = curr->next) {
		resMap[curr->key] = curr->value;
	}

	return resMap;
}

bDictionaryNode::bDictionaryNode() {
	this->key = "";
	this->value = NULL;
	this->next = NULL;
}


/*************************** Torrent info structure for processing****************************/
//FileItem::FileItem(std::string path, long long size,
//	long long offset) {
//	
//	this->Path = path;
//	this->size = size;
//	this->offset = offset;
//	this->FormattedSize = std::to_string(size);
//}
//

