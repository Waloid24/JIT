CXX 		?= g++
CXXFLAGS 	?= 	   -O2 -g --static-pie -std=c++14 -fmax-errors=100 -Wall -Wextra   				\
    			   -Weffc++ -Waggressive-loop-optimizations -Wc++0x-compat 	   			\
    			   -Wc++11-compat -Wc++14-compat -Wcast-align -Wcast-qual 	   			\
    			   -Wchar-subscripts -Wconditionally-supported -Wconversion        			\
    			   -Wctor-dtor-privacy -Wempty-body -Wfloat-equal 		   			\
    			   -Wformat-nonliteral -Wformat-security -Wformat-signedness       			\
    			   -Wformat=2 -Winline -Wlarger-than=8192 -Wlogical-op 	           			\
    			   -Wmissing-declarations -Wnon-virtual-dtor -Wopenmp-simd 	   			\
    			   -Woverloaded-virtual -Wpacked -Wpointer-arith -Wredundant-decls 			\
    			   -Wshadow -Wsign-conversion -Wsign-promo -Wstack-usage=8192      			\
    			   -Wstrict-null-sentinel -Wstrict-overflow=2 			   			\
    			   -Wsuggest-attribute=noreturn -Wsuggest-final-methods 	   			\
    			   -Wsuggest-final-types -Wsuggest-override -Wswitch-default 	   			\
    			   -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused   			\
    			   -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix 	   			\
    			   -Wno-missing-field-initializers -Wno-narrowing 	           			\
    			   -Wno-old-style-cast -Wno-varargs -fcheck-new 		   			\
    			   -fsized-deallocation -fstack-check -fstack-protector            			\
    			   -fstrict-overflow -flto-odr-type-merging 	   		   			\
    			   -fno-omit-frame-pointer                                         			\
    			   -fPIE                                                           			\
    			   -lm -pie

SRC		:= logs.cpp
# OUT		:= logs.o

OBJDIR		?= $(CURDIR)

OUT 		:= $(addprefix $(OBJDIR)/, $(SRC:.cpp=.o))

.PHONY:	all
all:
	@ $(CXX) -c $(SRC) -o $(OUT) $(CXXFLAGS)
