g++ RPG/*.cpp RPG/PUGIXML/pugixml.cpp \
    -IRPG/include \
    -std=c++17 \
    -g -O0 -fsanitize=address -lraylib -lm -lpthread -ldl -lrt \
    -o RPGExample
