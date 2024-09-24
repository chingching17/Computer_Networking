CC = g++
tar = client server
LIB = -std=c++11
INCLUDE = same.cpp
all:$(tar)

%:%.cpp
	$(CC) $^ $(INCLUDE) -o $@ $(LIB)
clean:
	rm $(tar)