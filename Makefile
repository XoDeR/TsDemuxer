# Specify compiler
CC=cl.exe

# Specify linker
LINK=link.exe

.PHONY : all
all : TsDemuxer

# Link the object files into a binary
TsDemuxer : DeMuxer.o File.o Launcher.o Utilities.o
	$(LINK) /OUT:TsDemuxer.exe DeMuxer.o File.o Launcher.o Utilities.o
	
# Compile the source files into object files
DeMuxer.o : DeMuxer.cpp
	$(CC) /c DeMuxer.cpp /FoDeMuxer.o
File.o : File.cpp
	$(CC) /c File.cpp /FoFile.o
Launcher.o : Launcher.cpp
	$(CC) /c Launcher.cpp /FoLauncher.o
Utilities.o : Utilities.cpp
	$(CC) /c Utilities.cpp /FoUtilities.o

# Clean target
clean :
	del DeMuxer.o File.o Launcher.o Utilities.o TsDemuxer.exe