###############################################################################
INCLUDE_DIR += ../common/include ./include
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

# BOUNDS=bound_abstract bound_fsp_strong bound_fsp_weak bound_nqueens bound_test
# INST=instance_flowshop instance_vrf instance_nqueens

FILES=mcBBLL tree_controller tree bbthread treeheuristic
FFILES=arguments pbab solution subproblem ttime

OOBJS=$(addprefix $(BUILD), $(addsuffix .o, $(FFILES))) $(addprefix $(BUILD), $(addsuffix .o, $(FILES)))

bb: $(OOBJS)
	$(CXX) $(OOBJS) -o $@ $(OPT)

$(BUILD)mcBBLL.o: $(SRCDIR)mcBBLL.cpp
	@mkdir -p $(BUILD)
	$(CXX) -o $@ -c $< $(OPT)

$(BUILD)%.o: $(SRCDIR)%.cpp $(HDRS)%.h
	@mkdir -p $(BUILD)
	$(CXX) $(CFLAGS) -c $< -o $@ $(OPT)

$(BUILD)%.o: $(SRCDIR2)%.cpp
	@mkdir -p $(BUILD)
	$(CXX) $(CFLAGS) -c $< -o $@ $(OPT)

clean:
	$(RM) -r $(BUILD) ./bb
