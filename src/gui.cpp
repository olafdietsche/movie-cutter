extern "C" {
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#include <gtk/gtk.h>
#include "demuxer.h"
#include <vector>

std::vector<GtkWidget*> thumbnails;

static void setup_gui()
{
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GtkWidget *hbox = gtk_hbox_new(false, 0);
	GtkWidget *markers = gtk_vbox_new(true, 0);
	gtk_box_pack_start(GTK_BOX(hbox), markers, false, false, 0);
	gtk_widget_show(markers);
	guint rows = 3, columns = 5;
	GtkWidget *grid = gtk_table_new(rows, columns, 0);
	for (guint i = 0; i < rows; ++i) {
		for (guint j = 0; j < columns; ++j) {
			GtkWidget *tn = gtk_image_new();
			thumbnails.push_back(tn);
			gtk_table_attach_defaults(GTK_TABLE(grid), tn, j, j + 1, i, i + 1);
			gtk_widget_show(tn);
		}
	}

	gtk_box_pack_start(GTK_BOX(hbox), grid, true, true, 0);
	gtk_widget_show(grid);
	gtk_container_add(GTK_CONTAINER(window), hbox);
	gtk_widget_show(hbox);
	gtk_widget_show(window);
}

static int images = 0;
static int64_t next_pts = 0;

static bool filter(AVStream *st, AVPacket *pkt)
{
	//av_log(NULL, AV_LOG_ERROR, "filter, images=%d, next_pts=%ld\n", images, next_pts);
	AVCodecContext *dec_ctx = st->codec;
	if (dec_ctx->codec_type != AVMEDIA_TYPE_VIDEO)
		return false;

	int64_t pts = pkt->pts;
	if (st->start_time != AV_NOPTS_VALUE)
		pts -= st->start_time;

	bool use = pts * st->time_base.num * AV_TIME_BASE >= next_pts * st->time_base.den;
	if (use)
		next_pts += 60 * AV_TIME_BASE;

	return use;
}

#if 0
avframe::avframe(AVFrame *src, AVCodecContext *ctx) : f(0),tobefreed(0)
{
	f=avcodec_alloc_frame();
	tobefreed=malloc(avpicture_get_size(ctx->pix_fmt, ctx->width, ctx->height));

	avpicture_fill((AVPicture *)f,
		       (u_int8_t*)tobefreed,
		       ctx->pix_fmt,ctx->width,ctx->height);

#if LIBAVCODEC_VERSION_INT >= (51 << 16)
	av_picture_copy((AVPicture *)f, (const AVPicture *) src,
			ctx->pix_fmt, ctx->width, ctx->height);
#else
	img_copy((AVPicture *)f, (const AVPicture *) src,
		 ctx->pix_fmt, ctx->width, ctx->height);
#endif

	f->pict_type              = src->pict_type;
	f->quality                = src->quality;
	f->coded_picture_number   = src->coded_picture_number;
	f->display_picture_number = src->display_picture_number;
	f->pts                    = src->pts;
	f->interlaced_frame       = src->interlaced_frame;
	f->top_field_first        = src->top_field_first;
	f->repeat_pict            = src->repeat_pict;
	f->quality                = src->quality;

	w=ctx->width;
	h=ctx->height;
	pix_fmt=ctx->pix_fmt;
	dw=w*ctx->sample_aspect_ratio.num/ctx->sample_aspect_ratio.den;
#ifdef HAVE_LIB_SWSCALE
	img_convert_ctx=sws_getContext(w, h, pix_fmt, 
				       w, h, PIX_FMT_BGR24, SWS_BICUBIC, 
				       NULL, NULL, NULL);
#endif
}

QImage avframe::getqimage(bool scaled, double viewscalefactor)
{
#ifdef HAVE_LIB_SWSCALE
	if (w<=0 || h<=0 || img_convert_ctx==NULL)
#else
		if (w<=0 || h<=0)
#endif
			return QImage();

	uint8_t *rgbbuffer=(uint8_t*)malloc(avpicture_get_size(PIX_FMT_RGB24, w, h)+64);
	int headerlen=sprintf((char *) rgbbuffer, "P6\n%d %d\n255\n", w, h);

	AVFrame *avframergb=avcodec_alloc_frame();

	avpicture_fill((AVPicture*)avframergb, rgbbuffer+headerlen,
		       PIX_FMT_RGB24,w,h);

#ifdef HAVE_LIB_SWSCALE
	sws_scale(img_convert_ctx, f->data, f->linesize, 0, h, 
		  avframergb->data, avframergb->linesize);
#else
	img_convert((AVPicture *)avframergb, PIX_FMT_RGB24, (AVPicture*)f, pix_fmt, w, h);
#endif

	QImage im;
	im.loadFromData(rgbbuffer, headerlen+w*h*3, "PPM");

#ifdef HAVE_LIB_SWSCALE
	im = im.swapRGB();
#endif

	if ((scaled && w!=dw)||(viewscalefactor!=1.0)) {
#ifdef SMOOTHSCALE
		im = im.smoothScale(int((scaled?dw:w)/viewscalefactor+0.5), int(h/viewscalefactor+0.5));
#else
		im = im.scale(int((scaled?dw:w)/viewscalefactor+0.5), int(h/viewscalefactor+0.5));
#endif
	}

	free(rgbbuffer);
	av_free(avframergb);
	return (im);
}
#endif

static bool decoder(AVCodecContext *dec_ctx, AVFrame *frame)
{
	int width = 200, height = 80;
	size_t size = avpicture_get_size(PIX_FMT_RGB24, width, height);
	uint8_t *data = (uint8_t*) av_malloc(size);
	AVFrame *dest = av_frame_alloc();
	avpicture_fill((AVPicture*) dest, data, PIX_FMT_RGB24, width, height);

	av_log(NULL, AV_LOG_ERROR, "w=%d, h=%d\n", frame->width, frame->height);
	struct SwsContext *sws_ctx = sws_getContext(frame->width, frame->height,
						    dec_ctx->pix_fmt,
						    width, height,
						    PIX_FMT_RGB24, SWS_BICUBIC,
						    NULL, NULL, NULL);

	sws_scale(sws_ctx, frame->data, frame->linesize, 0, frame->height,
		  dest->data, dest->linesize);

	GdkPixbuf *buf = gdk_pixbuf_new_from_data(data, GDK_COLORSPACE_RGB,
						  false, 8, width, height,
						  dest->linesize[0], NULL, NULL);
	gtk_image_set_from_pixbuf(GTK_IMAGE(thumbnails[images++]), buf);
	return images >= 10;
}

void setup_video(const char *filename)
{
	demuxer dmux(filename);
	dmux.frame_loop(filter, decoder);
}

int main(int argc, char **argv)
{
	gtk_init(&argc, &argv);
	setup_gui();

	av_log_set_flags(AV_LOG_SKIP_REPEATED);
	av_register_all();
	avformat_network_init();

	setup_video(argv[1]);

	gtk_main();
	return 0;
}
