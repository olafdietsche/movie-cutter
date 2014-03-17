#! /usr/bin/make

FFMPEG_LIBS = \
libavdevice \
libavformat \
libavfilter \
libavcodec \
libswresample \
libswscale \
libavutil \

GTK_DEFINES =
GTK_INCLUDES = $(shell pkg-config --cflags gtk+-2.0)
GTK_LDLIBS = $(shell pkg-config --libs gtk+-2.0)

FFMPEG_BUILD_INCLUDES = -I /usr/src/extern/ffmpeg/libavutil
FFMPEG_DEFINES = -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
FFMPEG_INCLUDES = $(shell pkg-config --cflags libavformat libavcodec libavutil)
FFMPEG_LDLIBS = $(shell pkg-config --libs libavformat libavcodec libavutil libswscale)

CXXFLAGS = -Wall -g -std=c++0x $(FFMPEG_DEFINES) $(FFMPEG_INCLUDES) $(GTK_DEFINES) $(GTK_INCLUDES)
CFLAGS = -Wall -g $(FFMPEG_DEFINES) $(FFMPEG_INCLUDES) $(GTK_DEFINES) $(GTK_INCLUDES)

FFMPEG_OBJS = muxer.o demuxer.o converter.o

CMD_OBJS = demux.o
CMD_TARGET = demux
CMD_LDLIBS = $(FFMPEG_LDLIBS)

GUI_OBJS = gui.o main_screen.o frame_markers.o frame_sequence.o toolbar.o thumbnail.o
GUI_TARGET = movie-cutter
GUI_LDLIBS = $(GTK_LDLIBS) $(FFMPEG_LDLIBS)

OBJS = $(GUI_OBJS) $(CMD_OBJS) $(FFMPEG_OBJS)
TARGETS = $(CMD_TARGET) $(GUI_TARGET)

all: $(TARGETS)

$(GUI_TARGET): $(GUI_OBJS) $(FFMPEG_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(GUI_OBJS) $(FFMPEG_OBJS) $(GUI_LDLIBS)

$(CMD_TARGET): $(CMD_OBJS) $(FFMPEG_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(CMD_OBJS) $(FFMPEG_OBJS) $(CMD_LDLIBS)

depend deps: .depends

.depends:
	$(CXX) -MM $(CXXFLAGS) $(OBJS:.o=.cpp) >$@

clean:
	$(RM) $(TARGETS) $(OBJS) .depends

-include .depends
