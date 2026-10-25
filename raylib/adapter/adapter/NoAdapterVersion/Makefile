CXX = g++
TARGET = MyApp

CXXFLAGS = -O2 -Wall
LIBS = -lraylib -lm -lpthread -ldl

SRCS = main.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
