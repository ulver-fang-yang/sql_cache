

CXX=g++

BASE_SRC=sql_cache.cpp
BASE_OBJS=$(patsubst %cpp, %o, $(BASE_SRC))


EXEC=sql_cache

CXXFLAGS=-I./ -std=gnu++11 -Wall -O2 -fpermissive -fPIC
LDFLAGS=-lpthread

all: $(EXEC)
$(EXEC): $(BASE_OBJS)
	$(CXX) -o $(EXEC) $(BASE_OBJS) $(LDFLAGS)
%.o: %.cpp
	$(CXX) -c -o $@ $(CXXFLAGS) $<

clean_obj:
	rm -f $(BASE_OBJS)

clean:
	rm -f $(BASE_OBJS) $(EXEC)

