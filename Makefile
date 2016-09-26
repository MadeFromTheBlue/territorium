TARGET = territorium

USING_OPENGL = 1
USING_SOIL = 1

CXX = clang++ 

CFLAGS = -Wall -g -std=c++11

#############################
## SETUP OpenGL & GLUT 
#############################

# if we are using OpenGL & GLUT in this program
ifeq ($(USING_OPENGL), 1)
    # Windows builds
    ifeq ($(OS), Windows_NT)
        INCPATH += -I$(LAB_INC_PATH)
        LIBPATH += -L$(LAB_LIB_PATH)
        LIBS += -lglut -lopengl32 -lglu32

    # Mac builds
    else ifeq ($(shell uname), Darwin)
        LIBS += -framework GLUT -framework OpenGL

    # Linux and all other builds
    else
        LIBS += -lglut -lGL -lGLU
    endif
endif

#############################
## SETUP SOIL
#############################

# if we are using SOIL in this program
ifeq ($(USING_SOIL), 1)
    # Windows builds
    ifeq ($(OS), Windows_NT)
        INCPATH += -I$(LAB_INC_PATH)
        LIBPATH += -L$(LAB_LIB_PATH)
    # Mac builds
    else ifeq ($(shell uname), Darwin)
        LIBS += -framework Cocoa

    # Linux and other builds
    else
    	
    endif
    
    LIBS += -lSOIL
endif

#############################
## COMPILATION INSTRUCTIONS 
#############################

all: $(TARGET)

clean:
	rm -f $(TARGET)

$(TARGET): clean
	$(CXX) $(CFLAGS) $(INCPATH) ./src/*.cpp -o $(TARGET) $(LIBPATH) $(LIBS)