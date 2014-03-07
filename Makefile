OPENCV_LIBPATH=/usr/local/lib
OPENCV_INCLUDEPATH=/usr/include
OPENCV_LIBS=-lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_gpu -lopencv_core -lopencv_features2d -lopencv_nonfree

GCC_OPTS=-O3 -Wall -Wextra -m64

GCC=g++

INDENT=astyle

INDENT_OPTS=-n --style=allman --indent-classes --indent-switches --indent-cases --indent-namespaces --indent-labels -Y --min-conditional-indent=2 --max-instatement-indent=40 --pad-oper --unpad-paren --mode=c --add-brackets --break-closing-brackets --align-pointer=type

all: main.cpp
	$(GCC) -o gpu_optflow main.cpp $(GCC_OPTS) `pkg-config --libs --cflags opencv`

indent:
	$(INDENT) $(INDENT_OPTS) *.c *.cu *.h *.cpp
clean:
	rm -f *.o
	rm -f gpu_mosift
