# movie-cutter

## About

movie-cutter is a gui application to remove selected parts of a
movie. Its primary use is in cutting out commercials from recorded TV
movies.

## Requirements

movie-cutter relies on FFmpeg version 2.1.3 and Gtk+ 2.

It might work with other versions as well, I just haven't tested.

## Build

Go to the `src` directory and do `make`. There's no fancy `configure`
script, so if FFmpeg or Gtk+ is in some unusual place, you might need
to modify the Makefile.
