# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -g

# Source file
SRC = scheduler.cpp

# Targets
all: fifo_1 sjf_1 psjf_1 rr_1 fifo_2 sjf_2 psjf_2 rr_2

fifo_1: $(SRC)
	$(CXX) $(CXXFLAGS) -Dfifo_1 -o fifo_1 $(SRC)

sjf_1: $(SRC)
	$(CXX) $(CXXFLAGS) -Dsjf_1 -o sjf_1 $(SRC)

psjf_1: $(SRC)
	$(CXX) $(CXXFLAGS) -Dpsjf_1 -o psjf_1 $(SRC)

rr_1: $(SRC)
	$(CXX) $(CXXFLAGS) -Drr_1 -o rr_1 $(SRC)

fifo_2: $(SRC)
	$(CXX) $(CXXFLAGS) -Dfifo_2 -o fifo_2 $(SRC)

sjf_2: $(SRC)
	$(CXX) $(CXXFLAGS) -Dsjf_2 -o sjf_2 $(SRC)

psjf_2: $(SRC)
	$(CXX) $(CXXFLAGS) -Dpsjf_2 -o psjf_2 $(SRC)

rr_2: $(SRC)
	$(CXX) $(CXXFLAGS) -Drr_2 -o rr_2 $(SRC)

# Clean rule to remove the compiled binaries
clean:
	rm -f fifo_1 sjf_1 psjf_1 rr_1 fifo_2 sjf_2 psjf_2 rr_2
