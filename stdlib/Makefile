CXX = c++
CXXFLAGS = -fPIC -shared -I ../include
TARGET = testlib.so
SRCS = testlib.cpp

$(TARGET): $(SRCS)
	$(CXX) $(SRCS) $(CXXFLAGS) -o $(TARGET)

clean:
	rm -f $(TARGET)