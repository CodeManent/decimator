OUTFILENAME=decimator
#OUTPATH=../Debug
OUTPATH=.
OUTFILE=$(OUTPATH)/$(OUTFILENAME)
INPATH=../src

SRC=$(INPATH)/*.c $(INPATH)/*.cpp
HRD=$(INPATH)/*.h $(INPATH)/*.hpp
KERNELS=kernels.cl

PLATFORM=-m32
CFLAGS+=$(PLATFORM) -DLINUX #-I$(AMDAPPSDKROOT)/include/
CXXFLAGS+=$(PLATFORM) -DLINUX



LDFLAGS=-L/usr/lib32/nvidia-304-updates -L/usr/lib32/nvidia-current -L./libs # -L$(AMDAPPSDKROOT)/lib/x86
LIBS=-lOpenCL -lGL -lGLU 

CPPSOURCES=camera.cpp configuration.cpp decimator.cpp decimatorDataValidators.cpp decimatorIndependentPoints.cpp decimatorPrepareData.cpp decimatorSort.cpp main.cpp object.cpp plyobject.cpp rply.c scene.cpp
#HEDAERS=camera.hpp configuration.hpp decimator.hpp glut_callbacks.hpp plyobject.hpp point3.hpp rply.h cl.hpp object.hpp 
OBJECTS=$(addprefix $(OUTPATH)/,$(subst .c,.o,$(subst .cpp,.o,$(CPPSOURCES))))

.PHONY: all copy_kernels clean test
all: $(OUTFILE) $(OUTPATH)/$(KERNELS)

$(OUTFILE): $(OBJECTS)
	$(CXX) $(PLATFORM) $(OBJECTS) -o $(OUTFILE) $(LDFLAGS) $(LIBS)

$(OUTPATH)/%.o:$(INPATH)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
$(OUTPATH)/%.o:$(INPATH)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
	
copy_kernels $(OUTPATH)/$(KERNELS):$(INPATH)/$(KERNELS)
	cp $(INPATH)/$(KERNELS) $(OUTPATH)/$(KERNELS)
	
clean:
	$(RM) $(OBJECTS)
#	rm $(OUTPATH)/$(OUTFILE)

test: $(OUTFILE) $(OUTPATH)/$(KERNELS) ../testModels/horse.ply
	LD_LIBRARY_PATH=./libs/:/usr/lib32/nvidia-304-updates:$LD_LIBRARY_PATH $(OUTFILE) ../testModels/horse.ply --target 1000 --gpu --kernels $(OUTPATH)/$(KERNELS) --cw --antialiasing

run: $(OUTFILE) $(OUTPATH)/$(KERNELS)
	LD_LIBRARY_PATH=./libs/:/usr/lib32/nvidia-304-updates:$LD_LIBRARY_PATH $(OUTFILE) `zenity --file-selection --file-filter '*.ply'` --gpu --kernels $(OUTPATH)/$(KERNELS) --cw --antialiasing

