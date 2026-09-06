CXX = g++
TARGET = MyApp
SRCS = main.cpp

# Detect the operating system
UNAME_S := $(shell uname -s)

# Base compiler flags
CXXFLAGS = -O2 -Wall -std=c++20

# OS-specific configurations (empty LIBS for console apps, but keeps path setup if needed)
ifeq ($(UNAME_S), Darwin)
    # --- macOS Settings ---
    CXXFLAGS += -I/opt/homebrew/include
    LIBS = 
else ifeq ($(UNAME_S), Linux)
    # --- Linux Settings ---
    LIBS = 
endif

# Build target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
