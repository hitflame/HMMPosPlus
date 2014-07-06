PROJECT(c4jieba)

ADD_EXECUTABLE(segFile segFileTest.cpp)
ADD_EXECUTABLE(HmmPos HmmViterbiPos.cpp)
ADD_EXECUTABLE(TrieSegFile test/Trie_test.cpp)

ADD_DEFINITIONS(-std=c++0x -O3)
