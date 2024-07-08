set(TORRENT_STRUCTURE_SRC
		"torrent_structure.cpp")

set(BPROTOCOL_SRC 
		"BProtocol.cpp")

set(TORRENT_SRC
		"Torrent.cpp")

set(TRACKER_SRC
		"Tracker.cpp")

set(NETWORK_SRC
		"Network.cpp")



add_library(${TORRENT_LIB} STATIC
				${TORRENT_STRUCTURE_SRC}
				${BPROTOCOL_SRC}
				${TORRENT_SRC}
				${TRACKER_SRC} 
				${NETWORK_SRC})

target_link_libraries(${TORRENT_LIB} PUBLIC
                        ${OPENSSL_LIBRARIES}
						${OPENSSL_CRYPTO_LIBRARY}
						${OPENSSL_SSL_LIBRARY}
                        ${CURL_LIBRARIES}
						eventpp::eventpp)


target_include_directories(${TORRENT_LIB} PUBLIC
								${HEADER_DIR}
                                ${OPENSSL_INCLUDE_DIR}
								${CURL_INCLUDE_DIRS}
								${EVENTPP_HEADER_DIR})