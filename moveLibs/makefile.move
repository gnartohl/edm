EDML1=lib605432d2-f29d-11d2-973b-00104b8742df.so
EDML2=libEdmBase.so
EDML3=libEpics.so
EDML4=libcfcaa62e-8199-11d3-a77f-00104b8742df.so

.phony : all
.phony : 1
.phony : 2
.phony : 3
.phony : 4
.phony : 5
.phony : 6

all : 1 2 3 4 5 6

1 : ;
	echo " "
	echo "***************************************************"
	echo "*                                                 *"
	echo "*   ERRORS IN THE FOLLOWING BLOCK MAY BE IGNORED  *"
	echo "*                                                 *"
	echo "***************************************************"
	echo " "
	mv ../../../../bin/$(HOST_ARCH)/$(EDML1) ../../../../lib/$(HOST_ARCH)

2 : ;
	mv ../../../../bin/$(HOST_ARCH)/$(EDML2) ../../../../lib/$(HOST_ARCH)

3 : ;
	mv ../../../../bin/$(HOST_ARCH)/$(EDML3) ../../../../lib/$(HOST_ARCH)

4 : ;
	mv ../../../../bin/$(HOST_ARCH)/$(EDML4) ../../../../lib/$(HOST_ARCH)

5 : ;
	mv ../../pvFactory/O.Linux/*.so ../../../../lib/$(HOST_ARCH)

6 : ;
	echo " "
	echo "***************************************************"
	echo "*                                                 *"
	echo "*                    END OF BLOCK                 *"
	echo "*                                                 *"
	echo "***************************************************"
	echo " "

