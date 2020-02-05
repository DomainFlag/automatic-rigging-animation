# Makefile for UI
CC = g++
CCFLAGS = -c -O3 -Wall -I../Pinocchio/ -I../fbx/include -I/usr/include/libxml2/libxml -I/usr/include/eigen3 -I./../stb -DDEBUG -g
LIBS = -lm -pthread -L../Pinocchio/ -lpinocchio -lfbxsdk -lxml2 -ldl -lrt -luuid -lz -lfltk -lfltk_gl -lGL

OBJECTS = main.o MyWindow.o defmesh.o motion.o

Project: $(OBJECTS)
	$(CC) $(OBJECTS) $(LIBS) -o Project

.cpp.o:
	$(CC) $(CCFLAGS) $<


#do a makedepend and remove all the external dependencies from the makefile
#works on my system, no one else should need to do it.
depend:
	makedepend *.cpp > /dev/null >& 1
	perl -pi -e 's/\.c\.o/\.o/' Makefile
	perl -pi -e 's/\/u[Ss]r[a-zA-Z0-9\+\-\/\.\_]*//' Makefile
	perl -pi -e 's/\/u[Ss]r[a-zA-Z0-9\+\-\/\.\_]*//' Makefile
	perl -pi -e 's/\/u[Ss]r[a-zA-Z0-9\+\-\/\.\_]*//' Makefile
	perl -pi -e 's/\/u[Ss]r[a-zA-Z0-9\+\-\/\.\_]*//' Makefile
	perl -pi -e 's/\/u[Ss]r[a-zA-Z0-9\+\-\/\.\_]*//' Makefile
	perl -pi -e 's/\/u[Ss]r[a-zA-Z0-9\+\-\/\.\_]*//' Makefile
	perl -pi -e 's/\/u[Ss]r[a-zA-Z0-9\+\-\/\.\_]*//' Makefile
	perl -pi -e 's/\/u[Ss]r[a-zA-Z0-9\+\-\/\.\_]*//' Makefile
	perl -pi -e 's/^[a-zA-Z].*\.o\:\s*$$//' Makefile

clean:
	rm -f $(OBJECTS) Project *~ core *.out *.bak

cleanobj:
	rm -f $(OBJECTS)


# DO NOT DELETE

MyWindow.o:   MyWindow.h
MyWindow.o: ../Pinocchio/mesh.h ../Pinocchio/vector.h
MyWindow.o: ../Pinocchio/hashutils.h ../Pinocchio/mathutils.h
MyWindow.o: ../Pinocchio/Pinocchio.h ../Pinocchio/rect.h
MyWindow.o: ../Pinocchio/transform.h ../Pinocchio/skeleton.h
MyWindow.o: ../Pinocchio/graphutils.h
defmesh.o: defmesh.h ../Pinocchio/attachment.h ../Pinocchio/mesh.h
defmesh.o: ../Pinocchio/vector.h ../Pinocchio/hashutils.h
defmesh.o: ../Pinocchio/mathutils.h 
defmesh.o: ../Pinocchio/Pinocchio.h ../Pinocchio/rect.h
defmesh.o: ../Pinocchio/skeleton.h ../Pinocchio/graphutils.h
defmesh.o: ../Pinocchio/transform.h  ../Pinocchio/matrix.h
defmesh.o: ../Pinocchio/vector.h ../Pinocchio/transform.h
defmesh.o: ../Pinocchio/mesh.h motion.h ../Pinocchio/intersector.h
defmesh.o: ../Pinocchio/vecutils.h
UI.o:  MyWindow.h
UI.o: ../Pinocchio/mesh.h ../Pinocchio/vector.h ../Pinocchio/hashutils.h
UI.o: ../Pinocchio/mathutils.h 
UI.o: ../Pinocchio/Pinocchio.h ../Pinocchio/rect.h
motion.o: motion.h ../Pinocchio/transform.h ../Pinocchio/vector.h
motion.o: ../Pinocchio/hashutils.h ../Pinocchio/mathutils.h
motion.o:  ../Pinocchio/Pinocchio.h
motion.o: ../Pinocchio/skeleton.h ../Pinocchio/graphutils.h
motion.o: ../Pinocchio/utils.h 
main.o:   MyWindow.h
main.o: ../Pinocchio/mesh.h ../Pinocchio/vector.h
main.o: ../Pinocchio/hashutils.h ../Pinocchio/mathutils.h
main.o:  ../Pinocchio/Pinocchio.h
main.o: ../Pinocchio/rect.h ../Pinocchio/transform.h
main.o: ../Pinocchio/skeleton.h ../Pinocchio/graphutils.h
main.o: ../Pinocchio/utils.h ../Pinocchio/debugging.h
main.o: ../Pinocchio/attachment.h ../Pinocchio/mesh.h
main.o: ../Pinocchio/skeleton.h ../Pinocchio/transform.h
main.o: ../Pinocchio/pinocchioApi.h ../Pinocchio/quaddisttree.h
main.o: ../Pinocchio/dtree.h ../Pinocchio/indexer.h
main.o: ../Pinocchio/multilinear.h ../Pinocchio/intersector.h
main.o: ../Pinocchio/vecutils.h ../Pinocchio/pointprojector.h
main.o: ../Pinocchio/debugging.h ../Pinocchio/attachment.h defmesh.h
main.o:  ../Pinocchio/matrix.h ../Pinocchio/vector.h motion.h
