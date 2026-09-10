// musics pKQqsJGJKfU KB9wq6sPjzA
// musics "Teardrop" "Angel" --search
// album PL123
// artist "Massive Attack" --search --videos --singles --live
// playlist PL456
#include <App.hpp>

#include <print>
#include <ranges>
#include <string_view>
#include <vector>

std::optional<std::string_view> to_string(AudioFormat format) {
    switch (format) {
        case AudioFormat::Mp3:  return "mp3";
        case AudioFormat::Flac: return "flac";
        case AudioFormat::Opus: return "opus";
        case AudioFormat::Wav:  return "wav";
    }
    return std::nullopt;
}

std::expected<int, std::string> MusicsCmd::operator()() const {
    std::println("Downloading musics...");
    std::println(
        "{}: {:s}", search ? "search terms" : "video IDs", ids | Ogma::Utils::joinByComma);
    return 0;
}

std::expected<int, std::string> AlbumCmd::operator()() const {
    std::println("Downloading album...");
    std::println("{}: {}", search ? "album name" : "album ID", album);
    return 0;
}

std::expected<int, std::string> ArtistCmd::operator()() const {
    std::println("Downloading from artist...");
    std::println("{}: {}", search ? "artist name" : "artist ID", artist);
    std::println("videos: {}", downloadVideos);
    std::println("singles and EPs: {}", downloadSinglesAndEPs);
    std::println("live performances: {}", downloadLivePerformances);
    return 0;
}

std::expected<int, std::string> PlaylistCmd::operator()() const {
    std::println("Downloading playlist...");
    std::println("{}: {}", search ? "playlist name" : "playlist ID", playlist);
    std::println("keep order: {}", keepPlaylistOrder);
    return 0;
}

std::expected<int, std::string> App::operator()() const {
    std::println("quality: {}", quality.value_or("default"));
    std::println(
        "format:  {}",
        format ? to_string(*format).value_or("default") : "default");
    std::println(
        "output:  {}",
        outputDirectory ? outputDirectory->string() : std::string{ "default" });
    return std::visit([](const auto& subcommand) {
        return subcommand();
    }, command);
}

int main(const int argc, const char* argv[]) {
    std::vector<std::string_view> args{ argv + 1, argv + argc };

    // default args
    if (args.empty())
        args = {
            "--quality", "best-quality",
            "--format", "mp3",
            "musics", "pKQqsJGJKfU", "KB9wq6sPjzA",
            "--search", "Teardrop", "Angel"
        };

    return Ogma::ParseAndExecute<App>(args);
}
