/*!
\file abstract-dataset.cpp
\brief Implementation of AbstractDataset.
\author Raul Tapia
*/
#include "openev/datasets/abstract-dataset.hpp"
#include <algorithm>
#include <array>
#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include <zip.h>

namespace {
constexpr std::size_t CHUNK = 65536;
constexpr const char *AGENT = "openev";

std::size_t write(const char *data, const std::size_t size, const std::size_t count, void *file) {
  return std::fwrite(data, size, count, static_cast<std::FILE *>(file));
}

int report(void *progress, const curl_off_t total, const curl_off_t now, const curl_off_t /*uploadTotal*/, const curl_off_t /*uploadNow*/) {
  (*static_cast<const ev::AbstractDataset::Progress *>(progress))(static_cast<double>(now), static_cast<double>(total));
  return 0;
}

bool fetch(const std::string &url, const std::string &file, const ev::AbstractDataset::Progress &progress, std::string &error) {
  static std::once_flag once;
  std::call_once(once, []() { curl_global_init(CURL_GLOBAL_DEFAULT); });

  CURL *curl = curl_easy_init();
  if(curl == nullptr) {
    error = "could not initialize libcurl";
    return false;
  }
  std::FILE *output = std::fopen(file.c_str(), "wb");
  if(output == nullptr) {
    curl_easy_cleanup(curl);
    error = "could not write " + file;
    return false;
  }

  std::array<char, CURL_ERROR_SIZE> detail{};
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_USERAGENT, AGENT);
  curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
  curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);
  curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, detail.data());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, output);
  if(progress) {
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, report);
    curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progress);
  }

  const CURLcode code = curl_easy_perform(curl);
  curl_easy_cleanup(curl);
  const bool flushed = std::fclose(output) == 0;
  if(code != CURLE_OK) {
    error = "could not download " + url + ": " + (detail.front() != '\0' ? detail.data() : curl_easy_strerror(code));
    return false;
  }
  if(!flushed) {
    error = "could not write " + file;
    return false;
  }
  return true;
}

bool extract(const std::string &file, const std::filesystem::path &directory, std::string &error) {
  int code = 0;
  zip_t *archive = zip_open(file.c_str(), ZIP_RDONLY, &code);
  if(archive == nullptr) {
    error = "could not open " + file + " as a zip file";
    return false;
  }

  bool ok = true;
  std::vector<char> buffer(CHUNK);
  const zip_int64_t entries = zip_get_num_entries(archive, 0);
  for(zip_int64_t i = 0; ok && i < entries; i++) {
    const char *entry = zip_get_name(archive, static_cast<zip_uint64_t>(i), 0);
    if(entry == nullptr) {
      continue;
    }
    const std::filesystem::path relative = std::filesystem::path(entry).lexically_normal();
    if(relative.empty() || relative.is_absolute() || *relative.begin() == "..") {
      error = "unsafe path in " + file + ": " + entry;
      ok = false;
      break;
    }

    const std::filesystem::path target = directory / relative;
    const std::string name = entry;
    std::error_code ignored;
    if(name.back() == '/') {
      std::filesystem::create_directories(target, ignored);
      continue;
    }
    std::filesystem::create_directories(target.parent_path(), ignored);

    zip_file_t *source = zip_fopen_index(archive, static_cast<zip_uint64_t>(i), 0);
    std::ofstream output(target, std::ios::binary);
    if(source == nullptr || !output) {
      error = "could not extract " + name;
      ok = false;
    }
    zip_int64_t read = 0;
    while(ok && (read = zip_fread(source, buffer.data(), buffer.size())) > 0) {
      output.write(buffer.data(), read);
    }
    if(ok && (read < 0 || !output)) {
      error = "could not extract " + name;
      ok = false;
    }
    if(source != nullptr) {
      zip_fclose(source);
    }
  }

  zip_close(archive);
  return ok;
}

bool published(const ev::AbstractDataset &dataset, const std::string &sequence) {
  const std::vector<std::string> names = dataset.sequences();
  return std::find(names.begin(), names.end(), sequence) != names.end();
}
} // namespace

std::string ev::AbstractDataset::cacheDirectory() {
  const char *xdg = std::getenv("XDG_CACHE_HOME");
  if(xdg != nullptr && xdg[0] != '\0') {
    return std::string(xdg) + "/openev/datasets";
  }
  const char *home = std::getenv("HOME");
  return std::string(home != nullptr ? home : ".") + "/.cache/openev/datasets";
}

void ev::AbstractDataset::setDirectory(const std::string &directory) {
  directory_ = directory;
}

std::string ev::AbstractDataset::directory() const {
  return directory_.empty() ? cacheDirectory() + "/" + shortname() : directory_;
}

std::string ev::AbstractDataset::path(const std::string &sequence) const {
  return directory() + "/" + sequence;
}

bool ev::AbstractDataset::isAvailable(const std::string &sequence) const {
  return published(*this, sequence) && isComplete_(path(sequence));
}

bool ev::AbstractDataset::download(const std::string &sequence, const Progress &progress /*= nullptr*/) {
  error_.clear();
  if(!published(*this, sequence)) {
    error_ = "unknown sequence " + sequence;
    return false;
  }
  if(isAvailable(sequence)) {
    return true;
  }

  const std::filesystem::path target = path(sequence);
  const std::string file = target.string() + ".zip";
  std::error_code code;
  std::filesystem::create_directories(target, code);
  if(code) {
    error_ = "could not create " + target.string();
    return false;
  }

  const bool ok = fetch(url(sequence), file, progress, error_) && extract(file, target, error_);
  std::filesystem::remove(file, code);
  if(ok && !isComplete_(target.string())) {
    error_ = "the downloaded sequence is incomplete";
  }
  if(!error_.empty()) {
    std::filesystem::remove_all(target, code);
    return false;
  }
  return true;
}

bool ev::AbstractDataset::open(const std::string &sequence, const Progress &progress /*= nullptr*/) {
  close();
  if(!download(sequence, progress)) {
    return false;
  }
  load_(sequence);
  if(!isOpen()) {
    return false;
  }
  sequence_ = sequence;
  return true;
}

void ev::AbstractDataset::close() {
  reader_.reset();
  frames_.clear();
  imu_.clear();
  sequence_.clear();
}

void ev::AbstractDataset::assign_(std::unique_ptr<AbstractReader_> reader, FrameFileVector frames, ImuVector imu) {
  reader_ = std::move(reader);
  frames_ = std::move(frames);
  imu_ = std::move(imu);
  std::stable_sort(frames_.begin(), frames_.end(), [](const FrameFile &a, const FrameFile &b) { return a.t < b.t; });
  std::stable_sort(imu_.begin(), imu_.end(), [](const Imu &a, const Imu &b) { return a.t < b.t; });
}
