CC := g++ # This is the main compiler
AR = ar rcs

SRCDIR := src
SRCDIR2 := ../common/src
BUILD := build

TARGET := ./test/testFSP
TARGET2 := libig.a

SRCEXT := cpp
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILD)/%,$(SOURCES:.$(SRCEXT)=.o)) $(BUILD)/subproblem.o
CFLAGS := -Wall  -Wno-unused-result -Wno-unused-function -std=c++11 -O3
LIB := -ldl -lpthread -lm -L../bounds/lib -lbounds
INC := -I include

all: $(TARGET) $(TARGET2)

$(TARGET2): $(OBJECTS)
	@echo " Linking..." $(OBJECTS)
	$(AR) $@ $^

$(TARGET): ./test/testFSP.cpp $(TARGET2)
	$(CC) $^ -o $(TARGET) $(LIB)

$(BUILD)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@ $(LIB)

$(BUILD)/%.o: $(SRCDIR2)/%.$(SRCEXT)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@ $(LIB)

clean:
	@echo " Cleaning...";
	$(RM) -r $(BUILD) $(TARGET) libig.a

.PHONY: clean
