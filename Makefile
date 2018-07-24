# Automatic Makefile
# $@ -- The file name of the target of the rule.
# $* -- The stem with which an implicit rule matches
# $< -- The name of the first prerequisite.
# $^ -- The names of all the prerequisites
# $(VARS:%.cpp=%.o) -- pattern replace

# compiler
CC=clang++

EXT=debug
debug: EXT=debug
release: EXT=release

# determine targets
TARGETS:=$(wildcard src/*/main*.cpp) $(wildcard src/*/test*.cpp)
TARGETS:=$(TARGETS:src/%.cpp=bin/%.$(EXT))
$(info TARGETS is $(TARGETS))
#TARGETS=bin/huy/main bin/ext/test
#CPPS:=$(shell find src -name '*.cpp')
#CPPS:=$(CPPS:src/%.cpp=%)
#HPPS:=$(shell find src -name '*.h*')
#HPPS:=$(HPPS:src/%.hpp=%)
#HPPS:=$(HPPS:src/%.h=%)
#TARGETS:=$(filter-out $(HPPS),$(CPPS))
#TARGETS:=a/main
#TARGETS:=$(filter %/main%,$(TARGETS))
#$(info TARGETS is $(TARGETS))
#$(info HPPS is $(HPPS))
#$(info CPPS is $(CPPS))

# output
OUTS:=${TARGETS}

# targets
debug: ${TARGETS}
release: ${TARGETS}

# compiler options
CCOPTS:=
CCOPTS+=-std=c++14
CCOPTS+=-I./src
CCOPTS+=-Wsign-compare -Wreturn-type -Wparentheses -Wpedantic -Wconversion-null
#CCOPTS+=-Wno-vla-extension
CCOPTS+=-ferror-limit=3
CCOPTS+=$(shell pkg-config --cflags freetype2)
#CCOPTS+=$(shell pkg-config --cflags gtk+-3.0)

# linker options
LLOPTS:=
#LLOPTS+=-lSDL2
#LLOPTS+=-lpthread
#LLOPTS+=-lGL -lGLEW
LLOPTS+=$(shell freetype-config --libs)
#LLOPTS+=$(shell pkg-config --libs gtk+-3.0)


debug: CCOPTS+=-O0 -g
debug: LLOPTS+=-O0 -g

release: CCOPTS+=-O3
release: LLOPTS+=-O3

# automatic

# assert dirs
$(shell mkdir -p obj)
$(shell find src/ -type d | cut -c 5- | xargs -I{} mkdir -p obj/{})
$(shell mkdir -p bin)
$(shell find src/ -type d | cut -c 5- | xargs -I{} mkdir -p bin/{})



# objects src/FPATH.cpp -> obj/FPATH.cpp.obj.EXT
OBJS:=$(shell find src -name '*.cpp')
OBJS:=$(OBJS:src/%.cpp=obj/%.cpp.obj.${EXT})

$(info OBJS is $(OBJS))

#-include $(OBJS:%.obj.${EXT}=%.d)
-include $(OBJS:%=%.dep)

# compiler
${OBJS}:
	${CC} -c -MMD -MF $@.dep -o $@ $(@:obj/%.obj.$(EXT)=src/%) ${CCOPTS}
	#${CC} -c -MMD -MP -MF $@.dep -o $@ $< ${CCOPTS}

# linker
#${OUTS}: bin/%.${EXT}: obj/%.cpp.obj.${EXT}
${OUTS}: ${OBJS}
	${CC} -o $@ obj/$(@:bin/%.$(EXT)=%).cpp.obj.${EXT} $(filter-out $(OUTS:bin/%.${EXT}=obj/%.cpp.obj.${EXT}),$(OBJS)) ${LLOPTS}
	

clean:
	rm -rf obj/* bin/*





