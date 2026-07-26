CXX = g++
TARGET = MyApp

CXXFLAGS = -O2 -Wall -std=c++20
LIBS = -lraylib -lm -lpthread -ldl -lrt

SRCS = main.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
