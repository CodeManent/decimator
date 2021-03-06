OUTFILENAME=decimator
#OUTPATH=../Debug
OUTPATH=.
OUTFILE=$(OUTPATH)/$(OUTFILENAME)
INPATH=../src

SRC=$(INPATH)/*.c $(INPATH)/*.cpp
HRD=$(INPATH)/*.h $(INPATH)/*.hpp
KERNELS=kernels.cl

#PLATFORM=-m32
CFLAGS+=$(PLATFORM) -g -DLINUX -I/opt/amdgpu-pro/include
CXXFLAGS+=$(PLATFORM) -std=c++17 -g -Wall -Wextra -pedantic -DLINUX -D__CL_ENABLE_EXCEPTIONS -DCL_TARGET_OPENCL_VERSION=120


LDFLAGS=-L/opt/amdgpu-pro/lib/x86_64-linux-gnu
LIBS=-lOpenCL -lrt

CPPSOURCES= configuration.cpp \
			decimator.cpp \
			decimatorDataValidators.cpp \
			decimatorIndependentPoints.cpp \
			decimatorPrepareData.cpp \
			decimatorSort.cpp \
			main.cpp \
			object.cpp \
			plyobject.cpp \
			rply.c

OBJECTS=$(addprefix $(OUTPATH)/,$(subst .c,.o,$(subst .cpp,.o,$(CPPSOURCES))))

.PHONY: all copy_kernels clean test test_cpu run
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
	$(RM) $(OUTFILE)
	$(RM) $(OUTPATH)/$(KERNELS)

test: $(OUTFILE) $(OUTPATH)/$(KERNELS) ../testModels/horse.ply
	$(OUTFILE) ../testModels/horse.ply --target 1000 --gpu --kernels $(OUTPATH)/$(KERNELS) --cw --antialiasing -o horse_100.ply

test_cpu: $(OUTFILE) $(OUTPATH)/$(KERNELS) ../testModels/horse.ply
	$(OUTFILE) ../testModels/horse.ply --cpu

run: $(OUTFILE) $(OUTPATH)/$(KERNELS)
	$(OUTFILE) `zenity --file-selection --file-filter '*.ply'` --gpu --kernels $(OUTPATH)/$(KERNELS) --cw --antialiasing

