#pragma once

#include <string>

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

class LemonAV
{
public:

	LemonAV(

	)
	{

	}


private:

};

/*

libav:

Muxers take encoded data in the form of @ref AVPacket "AVPackets" and write
it into files or other output bytestreams in the specified container format.

avformat_alloc_context()	to create muxing context.
	AVFormatContext.oformat select the muxer that will be used.
	AVFormatContext.pb   	set to an opened IO context, returned from avio_open2() or a custom one.
avformat_new_stream()		create stream
	AVStream.codecpar.codec_type = AVMEDIA_TYPE_VIDEO
	AVStream.codecpar.codec_id = AV_CODEC_ID_RAWVIDEO
	AVStream.codecpar.width
	AVStream.codecpar.height
	AVStream.codecpar.format = AV_PIX_FMT_BGRA
avformat_write_header() 	to init.
av_write_frame() 			to write frame.
av_write_trailer() 			to flush and finalize.
avformat_free_context()		to free muxing context.


ei toimi
ffmpeg -y -vsync 0 -hwaccel cuda -hwaccel_output_format cuda -i output0.mp4 -vcodec hevc_nvenc -vf scale=1920:1080 -crf 28 -preset slow asd_s.mp4

toimii
ffmpeg -y -hwaccel cuda -hwaccel_output_format cuda -i output2.mp4 -vf "hwupload_cuda,scale_npp=1920:1080" -c:v h264_nvenc -tune hq -preset p6 -b:v 20M asd_s.mp4

./configure --enable-nonfree --enable-cuda-nvcc --enable-libnpp --extra-cflags=-I/usr/local/cuda/include --extra-ldflags=-L/usr/local/cuda/lib64 --enable-nvenc --enable-filters --enable-avfilter --enable-runtime-cpudetect --enable-cuvid --enable-cuda-llvm --enable-ffnvcodec

/home/files/sdc/movies/Fury.2014.2160p.iT.WEB-DL.DDP.5.1.Atmos.DV.HEVC-MiON.mkv

ffmpeg -hwaccel cuda -hwaccel_output_format cuda -i /home/files/sdc/movies/Fury.2014.2160p.iT.WEB-DL.DDP.5.1.Atmos.DV.HEVC-MiON.mkv -c copy -map 0 -c:v hevc_nvenc -b:v 20M -vf format=yuv420p -c:a copy -c:s copy /home/files/sdc/movies/Fury.2014.2160p.iT.WEB-DL.DDP.5.1.Atmos.DV.HEVC8.mp4

ffmpeg -hwaccel cuda -hwaccel_output_format cuda -i /home/files/sdc/movies/Fury.2014.2160p.iT.WEB-DL.DDP.5.1.Atmos.DV.HEVC-MiON.mkv -c:a copy -c:s copy -b:v 20M -preset p6 -vf format=yuv420p -c:v h264_nvenc /home/files/sdc/movies/Fury.2014.2160p.iT.WEB-DL.DDP.5.1.Atmos.DV.HEVC-MiON.mp4

ue65nu7455u
 */

