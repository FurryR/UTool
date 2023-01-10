OUTPUT = utool
CXX = g++
CXXFLAGS = -pipe --std=c++11 -Wall -Wextra -lwinmm
all : build
build :
	@echo "[CXX] utool.cpp -> $(OUTPUT)"
	@$(CXX) -Os -s $(CXXFLAGS) $(LDFLAGS) utool.cpp -o $(OUTPUT)
debug :
	@echo "[CXX] utool.cpp -> $(OUTPUT)"
	@$(CXX) -g $(CXXFLAGS) $(LDFLAGS) utool.cpp -o $(OUTPUT)
clean :
	@echo "[RM] $(OUTPUT)"
	@rm $(OUTPUT)