/*
    Author:  https://github.com/burshlatt
    Project: https://github.com/burshlatt/CPP_tools
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
#include <limits>
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
class generator_int {
public:
    generator_int() :
        generator_int(std::numeric_limits<T>::min(), std::numeric_limits<T>::max())
    {}

    explicit generator_int(T min, T max) {
        if (!std::is_integral<T>::value)
            throw std::invalid_argument("Incorrect type");

        if (min > max)
            throw std::invalid_argument("Incorrect argument");

        if (sizeof(T) <= sizeof(std::mt19937::result_type))
            gen32_ = std::make_unique<std::mt19937>(rd_());
        else
            gen64_ = std::make_unique<std::mt19937_64>(rd_());

        distribution_ = std::make_unique<std::uniform_int_distribution<T>>(min, max);
    }

    ~generator_int() = default;

public:
    T get_random_value() const {
        if (sizeof(T) <= sizeof(std::mt19937::result_type))
            return (*distribution_)(*gen32_);
        else
            return (*distribution_)(*gen64_);
    }

private:
    std::random_device rd_;
    std::unique_ptr<std::mt19937> gen32_;
    std::unique_ptr<std::mt19937_64> gen64_;
    std::unique_ptr<std::uniform_int_distribution<T>> distribution_;
};

template <typename T>
class generator_real {
public:
    generator_real() :
        generator_real(std::numeric_limits<T>::min(), std::numeric_limits<T>::max())
    {}

    explicit generator_real(T min, T max) {
        if (!std::is_floating_point<T>::value)
            throw std::invalid_argument("Incorrect type");

        if (min > max)
            throw std::invalid_argument("Incorrect argument");

        gen32_ = std::make_unique<std::mt19937>(rd_());

        distribution_ = std::make_unique<std::uniform_real_distribution<T>>(min, max);
    }

    ~generator_real() = default;

public:
    T get_random_value() const {
        return (*distribution_)(*gen32_);
    }

private:
    std::random_device rd_;
    std::unique_ptr<std::mt19937> gen32_;
    std::unique_ptr<std::uniform_real_distribution<T>> distribution_;
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

    explicit file_t(path_reference path) : file_t() {
        set_path(path);
    }

    explicit file_t(string_reference text) : file_t() {
        set_text(text);
    }

    explicit file_t(const char* text, size_type size) : file_t() {
        set_text(text, size);
    }

    explicit file_t(path_reference path, string_reference text) : file_t() {
        set_path(path);
        set_text(text);
    }

    explicit file_t(path_reference path, const char* text, size_type size) : file_t() {
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

    void set_filename(string_reference name) {
        path_.replace_filename(fs::path(name));
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

    fs::path get_dir_fs() const { return path_.parent_path(); }

    std::string get_dir() const { return path_.parent_path().generic_string(); }

    std::string get_filename() const { return path_.filename().generic_string(); }

public:
    char& operator[](int index) {
        return text_[index];
    }

    char operator[](int index) const {
        return text_[index];
    }

public:
    std::size_t size() const noexcept { return size_; }

    bool empty() const noexcept { return !size_; }

    bool exists() const noexcept {
        return fs::exists(path_) && !fs::is_directory(path_);
    }

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

        return file_t(path, buffer.get(), file_size);
    }

    file_t read_file(string_reference path) const {
        return read_file(fs::path(path));
    }

    void read_file(file_t& file) const {
        file = read_file(file.get_path_fs());
    }

    void write_file(path_reference path, std::string_view text) const {
        if (!fs::exists(path) || fs::is_directory(path))
            return;

        std::ofstream file_stream(path, std::ios::app);

        if (file_stream.is_open()) {
            file_stream << text;
        } else {
            std::string error_text{"Error: Cannot open file: "};
            std::string filename{path.filename().generic_string()};
            throw std::ios_base::failure(error_text + filename);
        }
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

    void create_file(path_reference path) const {
        create_file(file_t(path));
    }

    void create_file(string_reference path) const {
        create_file(fs::path(path));
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
            } else if (opt == "d") {
                return path_str;
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
        dirs_.clear();
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
        console::print_text("d. SELECT CURRENT DIRECTORY", color::red, mod::bold);
        console::print_text("0. EXIT\n", color::red, mod::bold);
        console::print_text("Select menu item:", color::green, "", " ");
    }

private:
    mutable std::map<std::string, std::pair<bool, std::string>> dirs_;
};
} // namespace filesystem
} // namespace console_tools

#endif // TOOLS_TOOLS_HPP
