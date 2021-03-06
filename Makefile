# Project: dynamic

CPP  = g++
CC   = gcc
OBJ  = main.o bin.o ram.o dynarec.o cache.o compiler.o clprogram.o emucl.o
LINKOBJ  = main.o bin.o ram.o dynarec.o cache.o compiler.o clprogram.o emucl.o
LIBS =  -L"/opt/AMDAPP/lib/x86_64" -O3 -lOpenCL   
#LIBS =  -L"/opt/ati-stream-sdk-v2.3-lnx32/lib" -O3 -lOpenCL   
#LIBS =  -L"C:/Dev-Cpp/lib" -L"C:/Program Files/ATI Stream/lib/x86" -O3 -lopencl   
#INCS =  -I"C:/Dev-Cpp/include"  -I"C:/Program Files/ATI Stream/include" 
INCS =  -I"/opt/AMDAPP/include" 
#INCS =  -I"/opt/ati-stream-sdk-v2.3-lnx32/include" 
#CXXINCS =  -I"C:/Dev-Cpp/lib/gcc/mingw32/3.4.2/include"  -I"C:/Dev-Cpp/include/c++/3.4.2/backward"  -I"C:/Dev-Cpp/include/c++/3.4.2/mingw32"  -I"C:/Dev-Cpp/include/c++/3.4.2"  -I"C:/Dev-Cpp/include"  -I"C:/Program Files/ATI Stream/include" 
CXXINCS =  -I"/opt/AMDAPP/include" 
#CXXINCS =  -I"/opt/ati-stream-sdk-v2.3-lnx32/include" 
BIN  = ../dynamic
CXXFLAGS = $(CXXINCS) -O3 -m32 -msse2 -Wfloat-equal -Wpointer-arith -DATI_OS_LINUX -g3 -ffor-scope  -enable-auto-import -ansi
CFLAGS = $(INCS) -O3 -m32 -msse2 -Wfloat-equal -Wpointer-arith -DATI_OS_LINUX -g3 
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before $(BIN) all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CPP) $(LINKOBJ) -o $(BIN) $(LIBS)

main.o: main.cpp
	$(CPP) -c main.cpp -o main.o $(CXXFLAGS)

bin.o: bin.cpp
	$(CPP) -c bin.cpp -o bin.o $(CXXFLAGS)

ram.o: ram.c
	$(CC) -c ram.c -o ram.o $(CFLAGS)

dynarec.o: dynarec.c
	$(CC) -c dynarec.c -o dynarec.o $(CFLAGS)

cache.o: cache.c
	$(CC) -c cache.c -o cache.o $(CFLAGS)

compiler.o: compiler.c
	$(CC) -c compiler.c -o compiler.o $(CFLAGS)

clprogram.o: clprogram.cpp
	$(CPP) -c clprogram.cpp -o clprogram.o $(CXXFLAGS)

emucl.o: emucl.cpp
	$(CPP) -c emucl.cpp -o emucl.o $(CXXFLAGS)
