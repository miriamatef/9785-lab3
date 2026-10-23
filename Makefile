## you have command.cc command.h tokenizer.cc tokenizer.h you need compile them and link
## them to one object file called  : "myshell" 
## so we should run make , that will create our object myshell ,
## ./myshell should run your application 

CXX = g++
CXXFLAGS = -Wall -g #-Wall-> for warnings

TARGET = myshell
SOURCE_FILES = command.cc tokenizer.cc
OBJECT_FILES = command.o tokenizer.o

.PHONY: shell clean

shell: $(TARGET)

$(TARGET): $(OBJECT_FILES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECT_FILES)

%.o: %.cc command.h tokenizer.h #any .o file is built from its .cc
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECT_FILES) $(TARGET)