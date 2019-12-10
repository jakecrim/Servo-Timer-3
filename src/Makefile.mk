PROGRAM := demo
SOURCES	:= $(wildcard *.cc)
HEADERS	:= $(wildcard *.h)
$(OBJS) := $(SOURCES:.cc=.o)

CFLAGS := -std=c11 -03 -pthread
CXXFLAGS := -std=c++11 -03 -pthread

LDFLAGS += -L usr/local/include -pthread
LDLIBS += -l wiringPi

LINK.o = $(Link.cc)

#build rule
$(PROGRAM1) : $(OBJS)
	$(LINK.o) $^ $(LDLIBS) -o $@

.PHONY : clean
clean : ; rm $(PROGRAM) -f $(OBJS) 

#some other thing from the practice makefile