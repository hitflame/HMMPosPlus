PROJECT(c4jieba)

ADD_EXECUTABLE(HmmPos HmmViterbiPos.cpp)
ADD_EXECUTABLE(SegFile test/SegFileTest.cpp)

ADD_DEFINITIONS(-std=c++0x -O3)
