export CXX 		:= g++
export CXXFLAGS 	:= -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-include-dirs -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits 


export DESTDIR		:= $(CURDIR)/bin
export OBJDIR		:= $(CURDIR)/obj
export LOGDIR		:= $(CURDIR)/logs
export GRAPHDIR		:= $(CURDIR)/graph

.PHONY: compile src logs
compile: src

src: | $(OBJDIR) $(DESTDIR) $(LOGDIR) $(GRAPHDIR)
	@ echo ------ COMPILE src ------
	@ cd $(CURDIR)/src && $(MAKE)
	@ cd $(CURDIR)/src/logs && $(MAKE)
	@ $(CXX) $(OBJDIR)/src.o $(OBJDIR)/logs.o -o $(DESTDIR)/main $(CXXFLAGS)

.PHONY: run
run:
	./bin/main $(filter-out $@,$(MAKECMDGOALS))

clean:
	rm -r $(DESTDIR) $(OBJDIR) $(LOGDIR) $(GRAPHDIR)

$(DESTDIR):
	mkdir $(DESTDIR)
$(OBJDIR):
	mkdir $(OBJDIR)
$(LOGDIR):
	mkdir $(LOGDIR)
$(GRAPHDIR):
	mkdir $(GRAPHDIR)

asm1:
	nasm -f elf64 commands.asm -o commands.o
	ld commands.o -o commands
	./commands

asm2 : casm.o
	g++ -no-pie commands.o -o commands
	./commands

casm.o:
	nasm -f elf64 commands.asm -o commands.o

