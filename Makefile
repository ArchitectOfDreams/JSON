# Files
Sources := $(sort $(wildcard source/*.cpp))
Objects := $(Sources:%.cpp=%.o)
Library := lib/libJSON.a
TestSources := $(sort $(wildcard source/test/*.cpp))
TestExe := tests.exe



# Variables
ifneq "" "$(findstring Win,$(OS))"
Delete = del /q /s $(subst /,\,$(1)) >nul
else
Delete = rm $(1)
endif

CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++14
release: CXXFLAGS += -O2 -DNDEBUG

AR := ar
ARFLAGS := cru



# Rules
.PHONY: default clean info docs

default: $(Library) $(TestExe)

clean:
	$(call Delete,source/*.o)

info:
	@echo [Sources]
	@echo $(Sources)
	@echo [Objects]
	@echo $(Objects)
	@echo [Library]
	@echo $(Library)

docs:
	cd docs && doxygen Doxyfile-html.cfg

$(Library): $(Objects) | lib
	$(AR) $(ARFLAGS) $@ $^

lib:
	mkdir $@

$(TestExe): $(TestSources) $(Library)
	$(CXX) $(CXXFLAGS) -Isource $(TestSources) -Llib -lJSON -o$@

source/json.o: source/json.cpp source/json.h
