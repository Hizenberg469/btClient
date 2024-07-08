set(TEST_MAIN "test_main.cpp")

add_executable(${TEST_EXE} 
				${TEST_MAIN})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

target_link_libraries(${TEST_EXE} PUBLIC
						${TORRENT_LIB}
                        ${OPENSSL_LIBRARIES}
						${OPENSSL_CRYPTO_LIBRARY}
						${OPENSSL_SSL_LIBRARY}
                        ${CURL_LIBRARIES}
						Threads::Threads)

target_include_directories(${TEST_EXE} PUBLIC
								${HEADER_DIR}
                                ${OPENSSL_INCLUDE_DIR}
								${CURL_INCLUDE_DIRS}
								${EVENTPP_HEADER_DIR})