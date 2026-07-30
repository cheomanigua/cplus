CXX = g++
TARGET = MyApp

CXXFLAGS = -O2 -Wall -DBACKEND_RAYLIB -std=c++20
LIBS = -lraylib -lm -lpthread -ldl -lrt

SRCS = main.cpp SimInput.cpp SimRenderer.cpp

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
