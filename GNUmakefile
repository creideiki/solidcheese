# $Id$

CXX=g++

ifdef windir
EXE=.exe
LINKFLAGS=-lopengl32 -lglu32 -lglut32
else
# If you don't have /student/lib FIRST in your library search path, you'll
# get libGL.so.1 (from OpenWindows) instead of libGL.so.3 when compiling on
# the Sun systems at Linköping University. This is a Bad Thing.
LINKFLAGS=-L/student/lib -L/usr/X11R6/lib \
	  -lGL -lGLU -lglut -lX11 -lXmu -lXi -lm
EXE=
endif

COMMON_OBJS=csg_object.o csg_tree.o matrix.o persistence.o camera.o

INTERFACE_TEST_OBJS=dummy_renderer.o dummy_modeler.o
MATRIX_TEST_OBJS=matrix_test.o matrix.o
MODELER_TEST_OBJS=dummy_renderer.o modeler.o normalize.o
RENDERER_TEST_OBJS=dummy_modeler.o renderer.o
NORMALIZE_TEST_OBJS=normalize.o normalize_test.o
SOLIDCHEESE_OBJS=modeler.o renderer.o normalize.o

TARGETS=interface_test$(EXE) matrix_test$(EXE) modeler_test$(EXE) \
	renderer_test$(EXE) normalize_test$(EXE) glinfo$(EXE) solidcheese$(EXE)

SRC=$(wildcard *.cpp)
CXXFLAGS=-ansi -pedantic -Wall -g3 -DDEBUG -I/student/include

all: $(TARGETS)

doc:
	doxygen Doxyfile

# Do not try to build the 'debug' or 'release' targets without at least GNU Make 3.77.
# And remember to 'make clean' when switching between 'debug' and 'release'.

debug: override CXXFLAGS=-ansi -pedantic -Wall -g3 -DDEBUG -I/student/include
debug: all

release: override CXXFLAGS=-ansi -pedantic -Wall -O3 -DNDEBUG -I/student/include
release: all

normalize_test$(EXE): $(COMMON_OBJS) $(NORMALIZE_TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o normalize_test \
	   $(COMMON_OBJS) $(NORMALIZE_TEST_OBJS) \
	   $(LINKFLAGS)

interface_test$(EXE): $(COMMON_OBJS) $(INTERFACE_TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o interface_test \
	   $(COMMON_OBJS) $(INTERFACE_TEST_OBJS) \
	   $(LINKFLAGS)

matrix_test$(EXE): $(MATRIX_TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o matrix_test \
	   $(MATRIX_TEST_OBJS) \
	   $(LINKFLAGS)

modeler_test$(EXE): $(COMMON_OBJS) $(MODELER_TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o modeler_test \
	   $(COMMON_OBJS) $(MODELER_TEST_OBJS) \
	   $(LINKFLAGS)

renderer_test$(EXE): $(COMMON_OBJS) $(RENDERER_TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o renderer_test \
	   $(COMMON_OBJS) $(RENDERER_TEST_OBJS) \
	   $(LINKFLAGS)

glinfo$(EXE): glinfo.cpp
	$(CXX) $(CXXFLAGS) -o glinfo glinfo.cpp $(LINKFLAGS)

solidcheese$(EXE): $(COMMON_OBJS) $(SOLIDCHEESE_OBJS)
	$(CXX) $(CXXFLAGS) -o solidcheese \
	   $(COMMON_OBJS) $(SOLIDCHEESE_OBJS) \
	   $(LINKFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: all clean doc release debug

CLEAN=doc/ *.o *.d *~ .\\\#* core $(TARGETS)
ifdef windir
clean:
	-@echo '$(CLEAN)' | xargs -t rm -rf
else
clean:
	-rm -rf $(CLEAN)
endif

# Magical autodependencies borrowed from rockbox.haxx.se

%.d: %.cpp
	@echo "Updating dependencies for $<"
	@$(SHELL) -ec '$(CXX) -MM $(CXXFLAGS) $< \
		|sed '\''s|\($*\)\.o[ :]*|\1.o $(<:%.cpp=%.d) : |g'\'' > $@; \
		[ -s $@ ] || rm -f $@'

ifneq ($(MAKECMDGOALS),clean)
-include $(SRC:%.cpp=%.d)
endif
