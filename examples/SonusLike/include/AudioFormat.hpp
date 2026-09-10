#pragma once

#include <Ogma/Parser.hpp>

#include <optional>
#include <string_view>

enum class [[ =Ogma::caseInsensitive ]] AudioFormat { Mp3, Flac, Opus, Wav };

std::optional<std::string_view> to_string(AudioFormat format);
