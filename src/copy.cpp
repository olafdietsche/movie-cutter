extern "C" {
#include "libavformat/avformat.h"
}

#include "demuxer.h"
#include "muxer.h"
#include <iostream>

void frame_loop(muxer &mux, demuxer &dmux)
{
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;

	while (dmux.read_next_packet(&pkt) >= 0) {
		mux.write_packet(&pkt);
		av_free_packet(&pkt);
	}

	// flush cached frames
	dmux.flush(&pkt);
}

int main(int argc, char **argv)
{
	av_log_set_flags(AV_LOG_SKIP_REPEATED);
	av_register_all();
	avformat_network_init();

	const char *input_file = argv[1];
	const char *output_file = argv[2];
	if (strcmp(input_file, output_file) == 0) {
		av_log(NULL, AV_LOG_ERROR, "Input and output file are identical\n");
		exit(2);
	}

	demuxer dmux(input_file);
	muxer mux(output_file, dmux.get_format_context());
	frame_loop(mux, dmux);
	return 0;
}
