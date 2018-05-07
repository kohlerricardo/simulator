CC = gcc
CPP = g++
LD = g++
CFLAGS = $(FLAGS)
CPPFLAGS = $(FLAGS)
BIN_NAME = orcs
RM = rm -f

FLAGS =   -O3 -ggdb3 -Wall -Wextra -Werror -std=c++11 -lefence
LDFLAGS = -ggdb3

########################################################################
##FOLDERS
FD_PACKAGE = package
FD_PROCESSOR = processor
FD_BRANCH_PREDICTOR = branch_predictor
FD_OTHER = utils
FD_CACHE = cache
FD_PREFETCHER = prefetcher



###
LIBRARY = -lz

SRC_PACKAGE = 		$(FD_PACKAGE)/opcode_package.cpp $(FD_PACKAGE)/uop_package.cpp

SRC_TRACE_READER = 	trace_reader.cpp

SRC_PROCESSOR =		$(FD_PROCESSOR)/processor.cpp\
					$(FD_PROCESSOR)/reorder_buffer_line.cpp\
					$(FD_PROCESSOR)/memory_order_buffer_line.cpp
SRC_BRANCH_PREDICTOR =	$(FD_BRANCH_PREDICTOR)/branch_predictor.cpp\
						$(FD_BRANCH_PREDICTOR)/piecewise.cpp
						#$(FD_BRANCH_PREDICTOR)/twoBit.cpp
SRC_CACHE = $(FD_CACHE)/cache.cpp\
			$(FD_CACHE)/cache_manager.cpp

SRC_OTHER = $(FD_OTHER)/utils.cpp\
			$(FD_OTHER)/enumerations.cpp\
			$(FD_OTHER)/sanityTest.cpp
SRC_PREFETCHER = $(FD_PREFETCHER)/prefetcher.cpp\
				 $(FD_PREFETCHER)/stride_prefetcher.cpp
SRC_CORE =  simulator.cpp orcs_engine.cpp\
			$(SRC_TRACE_READER)\
			$(SRC_PACKAGE)\
			$(SRC_PROCESSOR)\
			$(SRC_OTHER)\
			$(SRC_BRANCH_PREDICTOR)\
			$(SRC_CACHE)\
			$(SRC_PREFETCHER)

########################################################
OBJS_CORE = ${SRC_CORE:.cpp=.o}
OBJS = $(OBJS_CORE)
########################################################
# implicit rules
%.o : %.cpp %.hpp
	$(CPP) -c $(CPPFLAGS) $< -o $@

########################################################

all: orcs

orcs: $(OBJS_CORE)
	$(LD) $(LDFLAGS) -o $(BIN_NAME) $(OBJS) $(LIBRARY)

clean:
	-$(RM) $(OBJS)
	-$(RM) $(BIN_NAME)
	@echo OrCS cleaned!
	@echo
