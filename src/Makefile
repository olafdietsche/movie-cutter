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

FFMPEG_OBJS = demuxer.o converter.o

CMD_OBJS = demux.o
CMD_TARGET = demux
CMD_LDLIBS = $(FFMPEG_LDLIBS)

GUI_OBJS = gui.o
GUI_TARGET = movie-cutter
GUI_LDLIBS = $(GTK_LDLIBS) $(FFMPEG_LDLIBS)

OBJS = $(GUI_OBJS) $(CMD_OBJS) $(FFMPEG_OBJS)

all: $(GUI_TARGET) $(CMD_TARGET)

$(GUI_TARGET): $(GUI_OBJS) $(FFMPEG_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(GUI_OBJS) $(FFMPEG_OBJS) $(GUI_LDLIBS)

$(CMD_TARGET): $(CMD_OBJS) $(FFMPEG_OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(CMD_OBJS) $(FFMPEG_OBJS) $(CMD_LDLIBS)

gui.o: demuxer.h converter.h
demux.o: demuxer.h converter.h
demuxer.o: demuxer.h
converter.o: converter.h

clean:
	$(RM) $(TARGETS) $(OBJS)

scribble-simple: scribble-simple.o
	$(CXX) $(LDFLAGS) -o $@ scribble-simple.o $(GTK_LDLIBS)
