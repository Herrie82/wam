// Copyright (c) 2021 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "Utils.h"

#include <cstdlib>
#include <fstream>
#include <limits>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <glib.h>
#include <json/json.h>
#include <sys/stat.h>
#include <unistd.h>

#include "BCP47.h"

namespace util {

static std::string getString(const char* value)
{
    return value ? std::string(value) : std::string();
}

std::vector<std::string> getErrorPagePaths(const std::string& errorPageLocation, const std::string& language)
{
    if (errorPageLocation.empty())
        return std::vector<std::string>();

    namespace fs = boost::filesystem;

    fs::path errPageLocation(errorPageLocation);
    fs::path filename = errPageLocation.filename();
    fs::path searchPath = fs::canonical(errPageLocation.parent_path());
    auto bcp47Pieces = BCP47::fromString(language);

    // search order:
    // searchPath/resources/<language>/<script>/<region>/html/fileName
    // searchPath/resources/<language>/<region>/html/fileName
    // searchPath/resources/<language>/html/fileName
    // searchPath/resources/html/fileName
    // searchPath/fileName

    std::vector<std::string> result;
    result.reserve(5);
    if (bcp47Pieces) {
        if (bcp47Pieces->hasScript()) {
            std::stringstream ss;
            ss << searchPath.string() << "/resources/";
            ss << bcp47Pieces->language() << "/";
            ss << bcp47Pieces->script();

            if (bcp47Pieces->hasRegion())
                ss << "/" << bcp47Pieces->region();

            ss << "/html/" << filename.string();
            result.emplace_back(ss.str());
        }
        if (bcp47Pieces->hasRegion()) {
            std::stringstream ss;
            ss << searchPath.string() << "/resources/";
            ss << bcp47Pieces->language() << "/";
            ss << bcp47Pieces->region() << "/html/";
            ss << filename.string();
            result.emplace_back(ss.str());
        }
        std::stringstream ss;
        ss << searchPath.string() << "/resources/";
        ss << bcp47Pieces->language() << "/html/";
        ss << filename.string();
        result.emplace_back(ss.str());
    }
    result.emplace_back(searchPath.string() + "/resources/html/" + filename.string());
    result.emplace_back(errPageLocation.string());

    return result;
}

std::string getHostname(const std::string& url)
{
    if (url.empty())
        return std::string();

    //source https://datatracker.ietf.org/doc/html/rfc3986#appendix-B

    std::regex rfc3986Regex(R"(^(([^:\/?#]+):)?(\/\/([^\/?#]*))?([^?#]*)(\?([^#]*))?(#(.*))?)");
    std::regex authorityRegex(R"(^(?:[\w\:]+[@])?([\w.]+)(?:[:])?(?:[0-9]+)?)");
    std::smatch matches;

    if (!std::regex_match(url, matches, rfc3986Regex))
        return std::string();

    std::string authority = matches[4];
    if (!std::regex_match(authority, matches, authorityRegex))
        return std::string();

    return matches[1];
}

bool doesPathExist(const std::string& path)
{
    if (path.empty())
        return false;

    struct stat st;
    if (stat(path.c_str(), &st))
        return false;

    return st.st_mode & S_IFDIR || st.st_mode & S_IFREG;
}

std::string readFile(const std::string& path)
{
    if (!doesPathExist(path))
        return std::string();

    std::ifstream file(path);
    return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

std::string uriToLocal(const std::string& uri)
{
    g_autofree gchar* cpath = g_filename_from_uri(uri.c_str(), nullptr, nullptr);
    return getString(cpath);
}

std::string localToUri(const std::string& uri)
{
    g_autofree gchar* cpath = g_filename_to_uri(uri.c_str(), nullptr, nullptr);
    return getString(cpath);
}

std::string getEnvVar(const char* env)
{
    return getString(getenv(env));
}

// STRING
bool strToInt(const std::string str, int& num)
{
    char* endptr = nullptr;
    errno = 0;
    long value = strtol(str.c_str(), &endptr, 10);
    if (endptr == str) {
        return false;
    }

    if (value > std::numeric_limits<int>::max() ||
        value < std::numeric_limits<int>::min() ||
        errno == ERANGE) {
        return false;
    }

    if (errno) {
        return false;
    }

    num = value;
    return true;
}

int strToIntWithDefault(const std::string& str, int defaultValue) {
  int convertedValue;
  return strToInt(str, convertedValue) ? convertedValue : defaultValue;
}

std::vector<std::string> splitString(const std::string &str, char delimiter)
{
    std::vector<std::string> resList;
    std::stringstream ss(str);
    std::string s;

    while (std::getline(ss, s, delimiter)) {
        resList.push_back(s);
    }

    return resList;
}

std::string trimString(const std::string& str)
{
    std::string trimmed(str);
    boost::trim_right(trimmed);
    boost::trim_left(trimmed);

    return trimmed;
}

void replaceSubstr(std::string& in, const std::string& toSearch,
                   const std::string& replaceStr /* ="" */)
{
    size_t pos = in.find(toSearch);
    while (pos != std::string::npos) {
        in.replace(pos, toSearch.size(), replaceStr);
        pos = in.find(toSearch, pos + replaceStr.size());
    }
}


// JSON
bool stringToJson(const std::string& str, Json::Value& value) {
  Json::CharReaderBuilder builder;
  Json::CharReaderBuilder::strictMode(&builder.settings_);
  std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

  return reader->parse(str.c_str(), str.c_str() + str.size(), &value, nullptr);
}

Json::Value stringToJson(const std::string& str) {
    Json::Value result;
    return stringToJson(str, result) ? result : Json::Value(Json::nullValue);
}

std::string jsonToString(const Json::Value& value) {
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "    ";
  builder["enableYAMLCompatibility"] = true;

  return Json::writeString(builder, value);
}

} // namespace
