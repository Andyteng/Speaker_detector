# set the binaries that have to be built
TARGETS := DenseTrack Video

INSTALL_DIR:= /usr/local/Cellar/opencv/3.4.2/lib

export LD_LIBRARY_PATH="$INSTALL_DIR/build/lib:$LD_LIBRARY_PATH"

export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:-L/usr/local/Cellar/opencv/3.4.2/lib/pkgconfig

# set the build configuration set 
BUILD := release
#BUILD := debug

# set bin and build dirs
BUILDDIR := .build_$(BUILD)
BINDIR := $(BUILD)

# libraries 
LDLIBS = $(addprefix -l, $(LIBS) $(LIBS_$(notdir $*)))
LIBS := \
	opencv_core opencv_highgui opencv_video opencv_imgproc opencv_videoio
	#\
	#avformat avdevice avutil avcodec swscale

# set some flags and compiler/linker specific commands
CXXFLAGS = -pipe -D __STDC_CONSTANT_MACROS -D STD=std -Wall $(CXXFLAGS_$(BUILD)) -I. -I/usr/local/Cellar/opencv/3.4.2/include
CXXFLAGS_debug := -ggdb
CXXFLAGS_release := -o1 -DNDEBUG -ggdb
LDFLAGS = -L/usr/local/Cellar/opencv/3.4.2/lib -L$(INSTALL_DIR) -pipe -Wall $(LDFLAGS_$(BUILD))
LDFLAGS_debug := -ggdb
LDFLAGS_release := -o1 -ggdb

include make/generic.mk
