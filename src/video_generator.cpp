#include "video_generator.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

std::string generate_video(const std::string &f1_path,
                           const std::string &f2_path) {
  std::string temp_video_path = "/tmp/video.mp4";
  std::string temp_gif_path = "/tmp/video.gif";

  std::string cmd = "ffmpeg -y -loop 1 -i " + f1_path + " -i " + f2_path +
                    " -filter_complex "
                    "\"[0:v]rotate='if(lte(t,7),2*PI*t*(1-t/10),2*PI*7*(1-7/"
                    "10))':c=none:ow=rotw(iw):oh=roth(ih)[r];[r][1:v]overlay=("
                    "W-w)/2:(H-h)/2\" -t 7 -c:v libx264 -pix_fmt yuv420p " +
                    temp_video_path;

  int ret = system(cmd.c_str());
  if (ret != 0) {
    throw std::runtime_error("ffmpeg command failed!");
  }

  std::string gif_cmd = "ffmpeg -y -i " + temp_video_path + " " + temp_gif_path;

  int gif_ret = system(gif_cmd.c_str());
  if (gif_ret != 0) {
    throw std::runtime_error("ffmpeg gif command failed!");
  }

  std::ifstream video_file(temp_gif_path, std::ios::binary);
  if (!video_file) {
    throw std::runtime_error("Could not open temporary video file!");
  }

  std::string video_data((std::istreambuf_iterator<char>(video_file)),
                         std::istreambuf_iterator<char>());

  video_file.close();
  std::remove(temp_video_path.c_str());
  std::remove(temp_gif_path.c_str());

  return video_data;
}
