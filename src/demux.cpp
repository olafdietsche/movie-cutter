extern "C" {
#include "libavformat/avformat.h"
}

#include "converter.h"
#include "demuxer.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

void write_ppm_image(const char *filename, int width, int height, const char *rgb24)
{
	std::ofstream f(filename);
	f << "P6 " << width << ' ' << height << " 255\n";
	f.write(rgb24, width * height * 3);
}

void write_ppm_image(int seconds, AVFrame *frame)
{
	std::ostringstream filename;
	filename << "img" << std::setw(3) << std::setfill('0') << seconds << ".ppm";
	write_ppm_image(filename.str().c_str(), frame->width, frame->height,
			reinterpret_cast<const char*>(frame->data[0]));
}

void frame_loop(demuxer &dmux)
{
	AVPacket pkt;
	av_init_packet(&pkt);
	pkt.data = NULL;
	pkt.size = 0;
	int video_stream_index = dmux.get_stream_index(AVMEDIA_TYPE_VIDEO);
	AVStream *video_stream = dmux.get_stream(video_stream_index);
	AVCodecContext *dec_ctx = video_stream->codec;
	av_log(dec_ctx, AV_LOG_DEBUG, "time_base=(%d/%d), start_time=%ld, duration=%ld\n", video_stream->time_base.num, video_stream->time_base.den, video_stream->start_time, video_stream->duration);
	int64_t seconds = 0;
	int64_t stream_pts = demuxer::rescale_timestamp(video_stream, seconds * AV_TIME_BASE);
	converter conv(video_stream, AV_PIX_FMT_RGB24);
	while (dmux.read_next_packet(&pkt, video_stream_index) >= 0) {
		AVPacket tmp = pkt;
		do {
			if (AVFrame *frame = dmux.decode_packet(dec_ctx, &tmp)) {
				if (pkt.pts >= stream_pts) {
					AVFrame *dest = conv.convert_frame(frame);
					write_ppm_image(seconds, dest);
					seconds += 30;
					av_log(dec_ctx, AV_LOG_DEBUG, "seconds=%ld\n", seconds);
					stream_pts = demuxer::rescale_timestamp(video_stream, seconds * AV_TIME_BASE);
					dmux.seek(video_stream_index, stream_pts);
					break;
				}
			}
		} while (tmp.size > 0);

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

	const char *filename = argv[1];
	demuxer dmux;
	dmux.open_input(filename);
	frame_loop(dmux);
	return 0;
}
