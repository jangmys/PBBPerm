###############################################################################
INCLUDE_DIR += ../bounds/include ../common/include ./include
LIBRARY_DIR +=
LDFLAGS += $(addprefix -L,$(LIBRARY_DIR))

BUILD=./build/
BINDIR=./bin/
SRCDIR=./src/
SRCDIR2=../common/src/
HDRS=./include/

LIB=-ldl -lpthread -lm -L../bounds/lib -lbounds -L../heuristic -lig

OPT = -Wall -pedantic -Wno-unused-function -Wno-unused-result -O3

CXX=g++

CPPFLAGS = -std=c++11
CPPFLAGS += $(addprefix -I,$(INCLUDE_DIR))

OPT += $(LDFLAGS) $(LIB) $(CPPFLAGS)

FILES=main sequentialbb bbthread matrix_controller ivm ivm_bound
FFILES=arguments pbab solution subproblem ttime

OOBJS=$(addprefix $(BUILD), $(addsuffix .o, $(FFILES))) $(addprefix $(BUILD), $(addsuffix .o, $(FILES)))

bb: $(OOBJS) ../bounds/lib/libbounds.a
	$(CXX) $(OOBJS) -o $@ $(OPT)

$(BUILD)main.o:	$(SRCDIR)main.cpp
	@mkdir -p $(BUILD)
	$(CXX) -o $@ -c $< $(OPT)

$(BUILD)%.o: $(SRCDIR)%.cpp $(HDRS)%.h
	@mkdir -p $(BUILD)
	$(CXX) $(CFLAGS) -c $< -o $@ $(OPT)

$(BUILD)%.o: $(SRCDIR2)%.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CFLAGS) -c $< -o $@ $(OPT)

../bounds/lib/libbounds.a:
	@cd "../bounds" && "$(MAKE)" -f  "makefile" && cd ../multicore

../heuristic/libig.a:
	@cd "../heuristic" && "$(MAKE)" -f  "heuristic.mk" && cd ../multicore

clean:
	$(RM) -r $(BUILD) $(EXEC) *.o bb
