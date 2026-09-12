CXX = g++
TARGET = MyApp
SRCS = basic-ecs.cpp

# Detect the operating system
UNAME_S := $(shell uname -s)

# Base compiler flags
CXXFLAGS = -O2 -Wall -std=c++20

# OS-specific configurations
ifeq ($(UNAME_S), Darwin)
    # --- macOS Settings ---
    # Include Homebrew paths (works for both Apple Silicon and Intel Macs)
    CXXFLAGS += -I/opt/homebrew/include
    LIBS = -L/opt/homebrew/lib -lraylib -lm -lpthread -ldl

else ifeq ($(UNAME_S), Linux)
    # --- Linux Settings ---
    # No extra include paths needed if installed via package manager/source, 
    # but you can add -I/path/to/include here if necessary.
    LIBS = -lraylib -lm -lpthread -ldl -lrt
endif

# Build target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
