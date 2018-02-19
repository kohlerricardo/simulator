CC = gcc
CPP = g++
LD = g++
CFLAGS = $(FLAGS)
CPPFLAGS = $(FLAGS)
BIN_NAME = orcs
RM = rm -f

FLAGS =   -O3 -ggdb -Wall -Wextra -Werror -std=c++11
LDFLAGS = -ggdb

########################################################################
##FOLDERS
FD_PACKAGE = package
FD_PROCESSOR = processor
FD_BRANCH_PREDICTOR = branch_predictor
FD_OTHER = utils



###
LIBRARY = -lz

SRC_PACKAGE = 		$(FD_PACKAGE)/opcode_package.cpp $(FD_PACKAGE)/uop_package.cpp

SRC_TRACE_READER = 	trace_reader.cpp

SRC_PROCESSOR =	 	$(FD_PROCESSOR)/processor.cpp 
SRC_BRANCH_PREDICTOR = $(FD_BRANCH_PREDICTOR)/plbp.cpp
SRC_CACHE = cache.cpp
SRC_OTHER = $(FD_OTHER)/circular_buffer.cpp
SRC_CORE =  simulator.cpp orcs_engine.cpp\
			$(SRC_TRACE_READER)	\
			$(SRC_PACKAGE) \
			$(SRC_PROCESSOR)
			#$(SRC_OTHER) \
			
			
			#$(SRC_BRANCH_PREDICTOR)\
			#$(SRC_CACHE)\
			

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
