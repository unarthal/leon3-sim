## This makefile is modeled based on the following blog:
#  https://scottmcpeak.com/autodepend/autodepend.html
##  


BUILD := DEBUG
CXX 	= g++
SRCDIR 	:= ./src

SRCS 	:= $(shell find $(SRCDIR) -name "*.cpp")
OBJS 	:= $(patsubst %.cpp,%.o,$(SRCS))
#CPPFLAGS += -MMD    # automatic .d dependency file generation
CPPFLAGS += -std=c++11

ifeq ($(BUILD),RELEASE)
    CPPFLAGS += -g -O3
    CPPFLAGS += -march=native
else
    CPPFLAGS += -g -O0
    CPPFLAGS += -DDEBUG -D_DEBUG
    CPPFLAGS += -DUNIT_TESTS
endif

CPPFLAGS += -Wall -Wextra -pedantic
#CPPFLAGS += -Werror

LDFLAGS += -lelf

%.o: %.cpp
	$(CXX) -I$(SRCDIR) $(CPPFLAGS) -o $@ -c $<
	$(CXX) -MM -I$(SRCDIR) $(CPPFLAGS) $*.cpp > $*.d

.PHONY: all clean

all: leon3-sim tags

tags: 
	ctags -R *

leon3-sim: $(OBJS)
	g++ -I$(SRCDIR) $^ $(LDFLAGS) -o $@

-include $(OBJS:%.o=%.d)

clean:
	rm -rf leon3-sim $(OBJS) tags TAGS
	rm -rf $(SRCS:%.cpp=%.d)

.PRECIOUS: $(OBJS)
.PHONY: clean all


