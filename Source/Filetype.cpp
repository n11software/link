#include <Filetype.hpp>

std::string getFromExtension(std::string extension) {
  if (extension == "html") return "text/html";
  else if (extension == "css") return "text/css";
  else if (extension == "js") return "text/javascript";
  else if (extension == "json") return "application/json";
  else if (extension == "png") return "image/png";
  else if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
  else if (extension == "gif") return "image/gif";
  else if (extension == "ico") return "image/x-icon";
  else if (extension == "svg") return "image/svg+xml";
  else if (extension == "pdf") return "application/pdf";
  else if (extension == "zip") return "application/zip";
  else if (extension == "tar") return "application/x-tar";
  else if (extension == "txt") return "text/plain";
  else if (extension == "xml") return "application/xml";
  else if (extension == "mp3") return "audio/mpeg";
  else if (extension == "mp4") return "video/mp4";
  else if (extension == "webm") return "video/webm";
  else if (extension == "ogg") return "audio/ogg";
  else if (extension == "wav") return "audio/wav";
  else if (extension == "webp") return "image/webp";
  else if (extension == "woff") return "font/woff";
  else if (extension == "woff2") return "font/woff2";
  else if (extension == "ttf") return "font/ttf";
  else if (extension == "otf") return "font/otf";
  else if (extension == "eot") return "application/vnd.ms-fontobject";
  else if (extension == "sfnt") return "application/font-sfnt";
  else if (extension == "manifest") return "text/cache-manifest";
  return "application/octet-stream";
}

std::string getExtension(std::string filename) {
  size_t pos = filename.find_last_of('.');
  if (pos == std::string::npos) return "";
  return filename.substr(pos + 1);
}

std::string getMIMEType(std::string filename) {
  return getFromExtension(getExtension(filename));
}