CXX = clang++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude -Ithird_party
PTHREAD_FLAG = -pthread

CLI_TARGET = smartroute
WEB_TARGET = smartroute_web
TEST_TARGET = test_runner

COMMON_SRCS = src/Graph.cpp src/Algorithms.cpp src/Navigation.cpp
CLI_SRCS = src/main.cpp src/UI.cpp $(COMMON_SRCS)
WEB_SRCS = src/server_main.cpp src/Server.cpp $(COMMON_SRCS)
TEST_SRCS = tests/test_suite.cpp $(COMMON_SRCS)

all: $(CLI_TARGET) $(WEB_TARGET)

$(CLI_TARGET): $(CLI_SRCS)
	$(CXX) $(CXXFLAGS) -Iinclude $(CLI_SRCS) -o $(CLI_TARGET)

$(WEB_TARGET): $(WEB_SRCS)
	$(CXX) $(CXXFLAGS) $(PTHREAD_FLAG) $(WEB_SRCS) -o $(WEB_TARGET)

$(TEST_TARGET): $(TEST_SRCS)
	$(CXX) $(CXXFLAGS) $(TEST_SRCS) -o $(TEST_TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

run: $(WEB_TARGET)
	./$(WEB_TARGET)

cli: $(CLI_TARGET)
	./$(CLI_TARGET)

demo: $(CLI_TARGET)
	./$(CLI_TARGET) --demo

clean:
	rm -f $(CLI_TARGET) $(WEB_TARGET) $(TEST_TARGET) *.o src/*.o

.PHONY: all clean run cli demo test
