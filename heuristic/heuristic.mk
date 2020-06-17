CC := g++ # This is the main compiler
# CC := clang --analyze # and comment out the linker last line for sanity
SRCDIR := src
SRCDIR2 := ../common/src

BUILD := build
TARGET := ./testHeuristic
TARGET2 := libig.a

AR = ar rcs

SRCEXT := cpp
#SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
SOURCES := $(wildcard src/*.cpp) #permutation
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILD)/%,$(SOURCES:.$(SRCEXT)=.o)) $(BUILD)/subproblem.o
CFLAGS := -Wall  -Wno-unused-result -Wno-unused-function -std=c++11 -O2
LIB := -ldl -lpthread -lm -L../bounds/lib -lbounds
INC := -I include

all: $(TARGET) $(TARGET2)

$(TARGET2): $(OBJECTS)
	@echo " Linking..." $(OBJECTS)
	$(AR) $@ $^

# $(TARGET): $(OBJECTS)
# 	$(CC) $(OBJECTS) -o $@ $(LIB)

$(TARGET): testHeur.cpp $(TARGET2)
	$(CC) $^ -o $(TARGET) $(LIB)

# $(BUILD)/main.o:	main.cpp
# 	@mkdir -p $(BUILD)
# 	$(CXX) -o $@ -c $< $(OPT)

# $(TARGET): main.cpp #$(OBJECTS)
# 	$(CC) $^ -o $(TARGET) $(LIB)

$(BUILD)/%.o: $(SRCDIR)/%.$(SRCEXT) #test/%.$(SRCEXT)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@ $(LIB)

$(BUILD)/%.o: $(SRCDIR2)/%.$(SRCEXT) #test/%.$(SRCEXT)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@ $(LIB)

# @echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

# $(BUILDDIR)%.o: $(SRCDIR2)%.cpp
# 	@mkdir -p $(BUILDDIR)
# 	@echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) $(INC) -c -o $@ $<

# $(CC) $(CFLAGS) -c $< -o $@ $(OPT)

clean:
	@echo " Cleaning...";
	$(RM) -r $(BUILD) $(TARGET) libig.a

.PHONY: clean
