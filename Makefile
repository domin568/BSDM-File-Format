CXX = g++
CXXFLAGS = -Wall -std=c++17 `libpng-config --I_opts` -w -Isrc/headers
LDFLAGS = `libpng-config --L_opts` `libpng-config --libs`

SRCDIR = src
OBJDIR = obj

EXECNAME = bsdmConvert

SRC := $(wildcard $(SRCDIR)/*.cpp)
OBJ := $(SRC:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

.PHONY: all

all: $(EXECNAME)

$(EXECNAME): $(OBJ)
	$(CXX) $^ $(LDFLAGS) -o $@

$(OBJ): $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

analyze: 
	$(CXX) $(CXXFLAGS) --analyze $(SRC)

debug: CXXFLAGS += -ggdb
debug: $(EXECNAME)

.PHONY: clean
clean:
	rm -rf $(OBJDIR) *.plist $(EXECNAME)