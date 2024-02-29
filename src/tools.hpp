/*
    https://github.com/burshlatt
*/

#ifndef TOOLS_TOOLS_HPP
#define TOOLS_TOOLS_HPP

#include <iostream>
#include <filesystem>
#include <type_traits>
#include <string_view>
#include <algorithm>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <string>
#include <chrono>
#include <random>
#include <map>

namespace fs = std::filesystem;

namespace tools {
namespace console {
/*
    ANSI Escape Codes
*/
namespace ansi {
struct colors {
    static constexpr const char* red{"\x1b[31m"};
    static constexpr const char* blue{"\x1b[34m"};
    static constexpr const char* cyan{"\x1b[36m"};
    static constexpr const char* white{"\x1b[37m"};
    static constexpr const char* black{"\x1b[30m"};
    static constexpr const char* green{"\x1b[32m"};
    static constexpr const char* yellow{"\x1b[33m"};
    static constexpr const char* purple{"\x1b[35m"};
    static constexpr const char* back_red{"\x1b[41m"};
    static constexpr const char* back_blue{"\x1b[44m"};
    static constexpr const char* back_cyan{"\x1b[46m"};
    static constexpr const char* back_white{"\x1b[47m"};
    static constexpr const char* back_black{"\x1b[40m"};
    static constexpr const char* back_green{"\x1b[42m"};
    static constexpr const char* back_yellow{"\x1b[43m"};
    static constexpr const char* back_purple{"\x1b[45m"};
};

struct mods {
    static constexpr const char* dim{"\x1b[2m"};
    static constexpr const char* bold{"\x1b[1m"};
    static constexpr const char* blink{"\x1b[5m"};
    static constexpr const char* hidden{"\x1b[8m"};
    static constexpr const char* reverse{"\x1b[7m"};
    static constexpr const char* italics{"\x1b[3m"};
    static constexpr const char* underline{"\x1b[4m"};
};

static constexpr const char* reset{"\x1b[0m"};
static constexpr const char* console_clear{"\x1b[2J\x1b[H"};
} // namespace ansi

void console_clear() noexcept {
    std::cout << ansi::console_clear;
}

void input_stream_clear() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void print_text(std::string_view text, const char* color="", const char* mod="", std::string_view sep="\n") noexcept {
    std::cout << mod << color << text << sep << ansi::reset;
}

int get_correct_int() {
    int result{};
    bool is_correct{false};
    while (!is_correct) {
        try {
            std::string input_value;
            std::cin >> input_value;
            result = std::stoi(input_value);
            is_correct = true;
        } catch (...) {
            print_text("\nERROR: Invalid input!\n", ansi::colors::red, ansi::mods::bold);
            print_text("Try again:", ansi::colors::green, "", " ");
            input_stream_clear();
        }
    }
    return result;
}
} // namespace console

namespace random {
template <typename Iterator>
void shuffle(Iterator begin, Iterator end) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(begin, end, gen);
}

template <typename T>
class generator {
private:
    using value_type = T;
    using engine     = std::default_random_engine;
    using duration   = std::chrono::_V2::system_clock::duration;
    using time_point = std::chrono::_V2::system_clock::time_point;

public:
    generator() = delete;

    explicit generator(value_type min, value_type max) {
        if (std::is_same<value_type, int>::value)
            range_int_ = std::make_unique<std::uniform_int_distribution<>>(min, max);
        else if (std::is_same<value_type, float>::value)
            range_real_ = std::make_unique<std::uniform_real_distribution<>>(min, max);
        else if (std::is_same<value_type, double>::value)
            range_real_ = std::make_unique<std::uniform_real_distribution<>>(min, max);
        else
            throw std::invalid_argument("Incorrect type");
            
        now_ = std::chrono::system_clock::now();
        time_since_ = now_.time_since_epoch();
        engine_ = std::make_unique<engine>(time_since_.count());
    }

    ~generator() = default;

public:
    value_type get_random_value() const {
        if (std::is_same<value_type, int>::value)
            return (*range_int_)(*engine_);
        else if (std::is_same<value_type, float>::value)
            return static_cast<float>((*range_real_)(*engine_));
        else if (std::is_same<value_type, double>::value)
            return (*range_real_)(*engine_);
        else
            throw std::invalid_argument("Incorrect type");
        return value_type();
    }

private:
    time_point now_;
    duration time_since_;
    std::unique_ptr<engine> engine_;
    std::unique_ptr<std::uniform_int_distribution<>> range_int_;
    std::unique_ptr<std::uniform_real_distribution<>> range_real_;
};
} // namespace random

namespace time {
class monitoring {
private:
    using size_type = std::size_t;
    using time_type = std::chrono::_V2::system_clock::time_point;

public:
    monitoring() = default;
    ~monitoring() = default;

public:
    void set_start_point() {
        this->start_point_ = std::chrono::system_clock::now();
    }

    void set_end_point() {
        this->end_point_ = std::chrono::system_clock::now();
    }

public:
    std::size_t get_time_offset() const {
        if (this->start_point_ == time_type()) {
            throw std::out_of_range("Time.err(): missing start point");
            return -1;
        } else if (this->end_point_ == time_type()) {
            throw std::out_of_range("Time.err(): missing end point");
            return -1;
        }
        auto seconds_end{std::chrono::time_point_cast<std::chrono::seconds>(this->end_point_)};
        auto seconds_start{std::chrono::time_point_cast<std::chrono::seconds>(this->start_point_)};
        auto seconds_end_int{seconds_end.time_since_epoch().count()};
        auto seconds_start_int{seconds_start.time_since_epoch().count()};
        this->end_point_ = time_type();
        this->start_point_ = time_type();
        return seconds_end_int - seconds_start_int;
    }

private:
    mutable time_type start_point_;
    mutable time_type end_point_;
};
} // namespace time

namespace filesystem {
class file_t {
private:
    using size_type        = std::size_t;
    using path_reference   = const fs::path&;
    using string_reference = const std::string&;

public:
    file_t() :
        path_(fs::current_path() / "temporary_file.txt")
    {}

    explicit file_t(path_reference path) {
        set_path(path);
    }

    explicit file_t(string_reference text) : file_t() {
        set_text(text);
    }

    explicit file_t(const char* text, size_type size) : file_t() {
        set_text(text, size);
    }

    explicit file_t(path_reference path, string_reference text) {
        set_path(path);
        set_text(text);
    }

    explicit file_t(path_reference path, const char* text, size_type size) {
        set_path(path);
        set_text(text, size);
    }

    explicit file_t(path_reference path, const char* text) = delete;

    ~file_t() = default;

public:
    void set_path(path_reference path) {
        fs::path tmp_path{path};
        tmp_path = tmp_path.remove_filename();
        if (fs::exists(tmp_path)) {
            path_ = path;
            if (fs::is_directory(path))
                path_ /= "temporary_file.txt";
        }
    }

    void set_path(string_reference path) {
        set_path(fs::path(path));
    }

    void set_text(string_reference text) {
        text_ = text;
        size_ = text.size();
    }

    void set_text(const char* text, size_type size) {
        set_text(std::string(text, size));
    }

    void set_text(const char* text) = delete;

public:
    std::string get_text() const { return text_; }

    fs::path get_path_fs() const { return path_; }

    std::string get_path() const { return path_.generic_string(); }

public:
    char& operator[](int index) {
        return text_[index];
    }

    char operator[](int index) const {
        return text_[index];
    }

public:
    [[nodiscard]] std::size_t size() const noexcept { return size_; }

private:
    std::size_t size_{};
    std::string text_;
    fs::path path_;
};

class monitoring {
private:
    using size_type = std::size_t;
    using mod       = console::ansi::mods;
    using color     = console::ansi::colors;
    using path_reference   = const fs::path&;
    using string_reference = const std::string&;

public:
    monitoring() = default;
    ~monitoring() = default;

public:
    file_t read_file(path_reference path) const {
        if (!fs::exists(path) || fs::is_directory(path))
            return file_t();

        std::ifstream file_stream(path, std::ios::binary | std::ios::in);
        std::unique_ptr<char[]> buffer;
        std::size_t file_size{};

        if (file_stream.is_open()) {
            file_stream.seekg(0, std::ios::end);
            file_size = file_stream.tellg();
            file_stream.seekg(0, std::ios::beg);
            buffer = std::make_unique<char[]>(file_size);
            file_stream.read(buffer.get(), file_size);
        } else {
            std::string error_text{"Error: Cannot open file: "};
            std::string filename{path.filename().generic_string()};
            throw std::ios_base::failure(error_text + filename);
        }

        file_t new_file(path, buffer.get(), file_size);

        return new_file;
    }

    file_t read_file(string_reference path) const {
        return read_file(fs::path(path));
    }

    void read_file(file_t& file) const {
        file = read_file(file.get_path_fs());
    }

    void create_file(path_reference path) const {
        fs::path tmp_path(path);
        tmp_path = tmp_path.remove_filename();
        if (fs::exists(tmp_path)) {
            fs::path new_file{path};
            if (fs::is_directory(path))
                new_file /= "temporary_file.txt";

            std::ofstream file_stream(new_file, std::ios::out);

            if (!file_stream.is_open()) {
                std::string error_text{"Error: Cannot create file: "};
                std::string filename{new_file.filename().generic_string()};
                throw std::ios_base::failure(error_text + filename);
            }
        }
    }

    void create_file(string_reference file_path) const {
        create_file(fs::path(file_path));
    }

    void create_file(const file_t& file) const {
        fs::path path(file.get_path_fs());
        std::ofstream file_stream(path, std::ios::out);

        if (file_stream.is_open()) {
            file_stream << file.get_text();
        } else {
            std::string error_text{"Error: Cannot create file: "};
            std::string filename{path.filename().generic_string()};
            throw std::ios_base::failure(error_text + filename);
        }
    }

public:
    std::string get_file_path() const {
        fs::path path(fs::current_path());
        while (true) {
            std::string path_str{path.generic_string()};

            print_filesystem(path_str);
            print_menu(path_str);

            std::string opt;
            std::cin >> opt;
            if (opt == "0") {
                break;
            } else if (opt == "b") {
                path = path.parent_path();
            } else if (opt == "c") {
                console::print_text("\nEnter filename: ", color::blue, "", " ");
                std::string filename;
                std::cin >> filename;
                create_file(path / filename);
                continue;
            } else if (dirs_.find(opt) != dirs_.end()) {
                auto [is_dir, name]{dirs_[opt]};
                path /= name;

                if (!is_dir) {
                    path_str = path.generic_string();
                    if (!fs::exists(path_str)) {
                        console::print_text("The file does not exist", color::red, "", " ");
                        continue;
                    }
                    return path_str;
                }
            }
        }
        return "";
    }

private:
    void print_filesystem(std::string_view path) const {
        console::console_clear();
        console::print_text("DIRS / FILES:\n", color::blue, mod::bold);
        int num{1};
        for (const auto& entry : fs::directory_iterator(path)) {
            if (entry.is_directory()) {
                console::print_text(std::to_string(num) + ".", color::red, "", " ");
                console::print_text("(Dir)", color::blue, mod::bold, "\t");
                dirs_[std::to_string(num)] = { true, entry.path().filename().generic_string() };
            } else {
                console::print_text(std::to_string(num) + ".", color::red, "", " ");
                console::print_text("(File)", color::green, mod::bold, "\t");
                dirs_[std::to_string(num)] = { false, entry.path().filename().generic_string() };
            }
            console::print_text(entry.path().filename().generic_string());
            num++;
        }
    }

    void print_menu(std::string_view path) const noexcept {
        console::print_text("\nCURRENT_DIR: ", color::red, mod::bold, " ");
        console::print_text(path, color::blue, mod::bold, "\n\n");
        console::print_text("b. BACK", color::red, mod::bold);
        console::print_text("c. CREATE FILE", color::red, mod::bold);
        console::print_text("0. EXIT\n", color::red, mod::bold);
        console::print_text("Select menu item:", color::green, "", " ");
    }

private:
    mutable std::map<std::string, std::pair<bool, std::string>> dirs_;
};
} // namespace filesystem
} // namespace console_tools

#endif // TOOLS_TOOLS_HPP
