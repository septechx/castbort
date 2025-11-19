#include <stdexcept>
#include <string>
#include <vector>

std::string generate_video(const std::string &f1_path,
                           const std::string &f2_path) {
  std::string cmd = "ffmpeg -y -loglevel error -nostats "
                    "-loop 1 -t 7 -i \"" +
                    f1_path + "\" -i \"" + f2_path +
                    "\" "
                    "-filter_complex "
                    "\"[0:v]rotate='if(lte(t,7),2*PI*t*(1-t/10),2*PI*7*(1-7/"
                    "10))':c=none:ow=rotw(iw):oh=roth(ih)[r];"
                    "[r][1:v]overlay=(W-w)/2:(H-h)/2,split=2[s0][s1];"
                    "[s0]palettegen=stats_mode=diff[p];"
                    "[s1][p]paletteuse\" "
                    "-f gif -";

  FILE *pipe = popen(cmd.c_str(), "r");
  if (!pipe) {
    throw std::runtime_error("Failed to start ffmpeg process");
  }

  std::string result;
  std::vector<char> buffer(8192);
  size_t n;
  while ((n = fread(buffer.data(), 1, buffer.size(), pipe)) > 0) {
    result.append(buffer.data(), n);
  }

  int rc = pclose(pipe);
  if (rc == -1) {
    throw std::runtime_error("Error waiting for ffmpeg to finish");
  }
  if (WIFEXITED(rc)) {
    int exit_status = WEXITSTATUS(rc);
    if (exit_status != 0) {
      throw std::runtime_error("ffmpeg failed with exit code " +
                               std::to_string(exit_status));
    }
  } else {
    throw std::runtime_error("ffmpeg terminated abnormally");
  }

  return result;
}
