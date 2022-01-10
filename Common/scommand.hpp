#ifndef SCOMMAND_HPP
#define SCOMMAND_HPP

#include <string>
namespace cm
{
#if ENABLED_NVIDIA_JETSON
static std::string gst_pipeline_cams[]
{
    // | Listing Name Cameras | Terminal : > v4l2-ctl --list-devices

    "nvarguscamerasrc sensor-id=0 ! video/x-raw(memory:NVMM), width=(int)640, height=(int)480,format=(string)NV12, framerate=(fraction)30/1 !"
    " nvvidconv ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink",

    "nvarguscamerasrc sensor-id=1 ! video/x-raw(memory:NVMM), width=(int)640, height=(int)480,format=(string)NV12, framerate=(fraction)30/1 !"
    " nvvidconv ! video/x-raw, format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink",

    "v4l2src device=/dev/video2 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink",

    "v4l2src device=/dev/video3 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink"
};
#else
static std::string gst_pipeline_cams[]
{
    // | Listing Name Cameras | Terminal : > v4l2-ctl --list-devices

    "v4l2src device=/dev/video2 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink",

    "v4l2src device=/dev/video2 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink",

    "v4l2src device=/dev/video4 ! video/x-raw, width=(int)640, height=(int)480, format=(string)YUY2, framerate=(fraction)30/1 !"
    " videoconvert ! video/x-raw, width=(int)640, height=(int)480, format=(string)RGB ! videoconvert ! appsink"
};
#endif


static std::string udp_gst_JPEG_send_video( std::string ip , int port , int fps=30)
{
    std::string gst_video_send = "appsrc ! videoconvert ! video/x-raw,format=YUY2,"
                                 "width=(int)640"
                                 ",height=(int)480"
                                 ",framerate=" + std::to_string(fps) + "/1 ! "
                                 "jpegenc ! rtpjpegpay ! "
                                 "udpsink host=" + ip +
                                 " port=" + std::to_string(port) +
                                 " sync=false async=false";
    return gst_video_send;
}



static std::string udp_gst_H264_send_video( std::string ip , int port , int bitrate=500)
{
    std::string gst_video_send = "appsrc ! videoconvert ! "
                                 "x264enc tune=zerolatency bitrate=" + std::to_string(bitrate) + " speed-preset=superfast ! queue ! rtph264pay ! "
                                 "udpsink host=" + ip +
                                 " port=" + std::to_string(port) +
                                 " sync=false async=false";
    return gst_video_send;
}


//static std::string Help_lisen_UDP_gst_JPEG_read_video( int port , int fps)
//{
//    std::string gst_video_send = "udpsrc port=" + std::to_string(port) +
//            " ! application/x-rtp,media=video,payload=26,clock-rate=90000,encoding-name=JPEG,"
//            "framerate=" + std::to_string(fps) + "/1 ! "
//            "rtpjpegdepay ! jpegdec ! videoconvert ! appsink max-buffers=1 drop=true sync=0";
//    return gst_video_send;
//}


//static std::string Help_lisen_UDP_gst_H264_read_video( int port , int fps)
//{
//    std::string gst_video_send = "udpsrc port=" + std::to_string(port) +
//            " ! application/x-rtp, media=(string)video, clock-rate=(int)90000, encoding-name=(string)H264, payload=(int)96,"
//            "framerate=" + std::to_string(fps) + "/1 ! "
//            "rtph264depay ! decodebin ! videoconvert ! appsink max-buffers=1 drop=true sync=0";
//    return gst_video_send;
//}


}


#endif // SCOMMAND_HPP