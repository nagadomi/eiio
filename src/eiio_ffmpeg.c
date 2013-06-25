#include "eiio.h"
#if EIIO_FFMPEG
/* TODO: ALL. NOT TESTED. */
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>

static int eiio_video_initialized = 0;

struct eiio_video {
	AVFormatContext *format;
	AVCodecContext *codec;
	AVFrame *frame;
	AVFrame *frame_rgb24;
	void *frame_buffer;
	struct SwsContext *sws;
	int stream_index;
	int width;
	int height;
};

eiio_video_t *
eiio_video_open(const char *file)
{
	eiio_video_t *video = (eiio_video_t *)eiio_malloc(sizeof(eiio_video_t));
	int i;
	AVCodec *decoder;
	
	memset(video, 0, sizeof(*video));
	video->stream_index = -1;
	
	if (!eiio_video_initialized) {
		av_register_all();
		eiio_video_initialized = 1;
	}
	if (av_open_input_file(&video->format, file, NULL, 0, NULL) != 0) {
		eiio_video_close(&video);
		return NULL;
	}
	if (av_find_stream_info(video->format) < 0) {
		eiio_video_close(&video);
		return NULL;
	}
	
	for (i = 0; i < video->format->nb_streams; ++i) {
		if (video->format->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
			video->codec = video->format->streams[i]->codec;
			video->stream_index = i;
			break;
		}
	}
	if (video->codec == NULL) {
		eiio_video_close(&video);
		return NULL;
	}
	decoder = avcodec_find_decoder(video->codec->codec_id);
	if (decoder == NULL) {
		eiio_video_close(&video);
		return NULL;
	}
	if (avcodec_open(video->codec, decoder) < 0) {
		eiio_video_close(&video);
		return NULL;
	}
	video->frame = avcodec_alloc_frame();
	eiio_video_set_size(video, video->codec->width, video->codec->height);
	
	return video;
}

int
eiio_video_set_size(eiio_video_t *video, int width, int height)
{
	void *frame_buffer = av_malloc(sizeof(eiio_uint8_t) *
								   avpicture_get_size(PIX_FMT_RGB24,
													  width,
													  height));
	AVFrame *frame_rgb24 = avcodec_alloc_frame();
	
	avpicture_fill((AVPicture *)frame_rgb24,
				   frame_buffer,
				   PIX_FMT_RGB24,
				   width, height);

	if (video->frame_rgb24) {
		av_free(video->frame_rgb24);
	}
	if (video->frame_buffer) {
		av_free(video->frame_buffer);
	}
	if (video->sws) {
		sws_freeContext(video->sws);
	}
	
	video->frame_rgb24 = frame_rgb24;
	video->frame_buffer = frame_buffer;
	video->width = width;
	video->height = height;
	video->sws = sws_getContext(video->codec->width, video->codec->height,
								video->codec->pix_fmt, 
								width, height, PIX_FMT_RGB24, SWS_BILINEAR,
								NULL, NULL, NULL);

	return video->sws == NULL ? -1 : 0;
}

static int
eiio_video_grab(eiio_video_t *video)
{
	AVPacket packet;
	int remain = 0;
	
	memset(&packet, 0, sizeof(packet));
	
	while (av_read_frame(video->format, &packet) >= 0) {
		int finish;
		
		if (packet.stream_index != video->stream_index) {
			goto next;
		}
		avcodec_decode_video(video->codec, video->frame, &finish,
							 packet.data, packet.size);
		if (!finish) {
			goto next;
		}
		
		remain = 1;
		break;
		
	next:
		if (packet.data == NULL) {
			av_free_packet(&packet);
			packet.data = NULL;
		}
	}
	if (packet.data) {
		av_free_packet(&packet);
	}
	
	return remain;
}

eiio_image_t *
eiio_video_next(eiio_video_t *video)
{
	if (eiio_video_grab(video)) {
		eiio_image_t *image;;
		
		if (sws_scale(video->sws,
					  video->frame->data, video->frame->linesize, 0,
					  video->codec->height,
					  video->frame_rgb24->data, video->frame_rgb24->linesize) != 0)
		{
			return NULL;
		}
		
		image = eiio_image_alloc(video->width, video->height,
								 EIIO_COLOR_RGB, eiio_malloc, free);
		
		memmove(image->data, (eiio_uint8_t *)video->frame_rgb24->data[0],
				sizeof(eiio_uint8_t) * video->width * video->height * 3);
		
		return image;
	} else {
		return NULL;
	}
}

int
eiio_video_seek(eiio_video_t *video, int64_t frame)
{

}

void
eiio_video_close(eiio_video_t **video)
{
	if ((*video)->format) {
		av_close_input_file((*video)->format);
		(*video)->format = NULL;
	}
	if ((*video)->codec) {
		avcodec_close((*video)->codec);
		(*video)->codec = NULL;
	}
	if ((*video)->frame) {
		av_free((*video)->frame);
		(*video)->frame = NULL;
	}
	if ((*video)->frame_rgb24) {
		av_free((*video)->frame_rgb24);
		(*video)->frame_rgb24 = NULL;
	}
	if ((*video)->frame_buffer) {
		av_free((*video)->frame_buffer);
		(*video)->frame_buffer = NULL;
	}
	if ((*video)->sws) {
		sws_freeContext((*video)->sws);
		(*video)->sws = NULL;
	}
	free(*video);
	*video = NULL;
}

#endif

