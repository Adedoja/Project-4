project5loader: main.o project5loader.o
        xlc -q64 -o project5loader main.o project5loader.o

main.o: main.c
        xlc -q64 -c -g main.c

project5loader.o: project5loader.c
        xlc -q64 -c -g project5loader.c

.PHONY: clean
clean:
        rm -f project5loader *.o *.dbg *.obj *.sic.obj *.sic.sic.obj
