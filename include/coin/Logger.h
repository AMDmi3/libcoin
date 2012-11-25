/* -*-c++-*- libcoin - Copyright (C) 2012 Michael Gronager
 *
 * libcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * libcoin is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libcoin.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COIN_LOGGER_H
#define COIN_LOGGER_H

#include <coin/Export.h>

#include <boost/thread/thread.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <deque>

class COIN_EXPORT LineBuffer : public std::streambuf {
public:
    LineBuffer(size_t lines = 10) : std::streambuf(), _lines(lines) { }
    
    ~LineBuffer() {
        newline();
    }
    
    void flush(std::ostream& os);
    
protected:
    virtual int overflow(int ch);
    
private:
    void newline();
    
private:
    LineBuffer(LineBuffer const &); // disallow copy construction
    void operator= (LineBuffer const &); // disallow copy assignment
    
private:
    std::string _line;
    size_t _lines;
    typedef std::deque<std::string> Buffer;
    Buffer _buffer;
};

class COIN_EXPORT Logger {
public: // enums
    enum Level {
        TRACE = 1,
        DEBUG,
        INFO,
        WARN,
        ERROR,
        FATAL
    };
    
public: // static
    static Logger& instance();
    
    static std::ostream& log(Level level, std::string file, int line, std::string function = "");
    
    static void instantiate(std::ostream& log_stream, Level log_level = INFO);
    
    static void label_thread(std::string label);
    
public:
    const std::string label(Level level) const;
    
    const std::string& label();

    const std::string timestamp() const;
    
private:
    std::ostream* _log_stream;

    LineBuffer _line_buffer;
    std::ostream _buf_stream;
    
    Level _log_level;

    typedef std::map<Level, std::string> LevelLabels;
    LevelLabels _level_labels;

    typedef std::map<boost::thread::id, std::string> ThreadLabels;
    ThreadLabels _thread_labels;    

private:
    Logger();

    // not to be implemented!
    Logger(Logger const&);
    void operator=(Logger const&);
};

#define log_fatal(...) Logger::log(Logger::FATAL, __FILE__, __LINE__, __PRETTY_FUNCTION__) << cformat(__VA_ARGS__) << std::endl
#define log_error(...) Logger::log(Logger::ERROR, __FILE__, __LINE__, __PRETTY_FUNCTION__) << cformat(__VA_ARGS__) << std::endl
#define log_warn(...) Logger::log(Logger::WARN, __FILE__, __LINE__, __PRETTY_FUNCTION__) << cformat(__VA_ARGS__) << std::endl
#define log_info(...) Logger::log(Logger::INFO, __FILE__, __LINE__, __PRETTY_FUNCTION__) << cformat(__VA_ARGS__) << std::endl

//#define log_debug(...) Logger::log(Logger::DEBUG, __FILE__, __LINE__, __PRETTY_FUNCTION__) << cformat(__VA_ARGS__) << std::endl
//#define log_trace(...) Logger::log(Logger::TRACE, __FILE__, __LINE__, __PRETTY_FUNCTION__) << cformat(__VA_ARGS__) << std::endl

#define log_debug(...) do {} while(0)
#define log_trace(...) do {} while(0)

#define cformat(fmt, ...) (Format(fmt), ##__VA_ARGS__)

class COIN_EXPORT Format {
public:
    Format(std::string format);
    
    /// The format string parser - a format specifier follows the prototype:
    ///     %[flags][width][.precision][length]specifier
    template<class T>
    inline Format& operator,(const T& rhs) {
        // find the format string and insert
        _pos = _format.find('%', _pos);
        if (_pos != std::string::npos) {
            size_t specifier_pos = _format.find_first_of("diuoxXfFeEgGaAcspn%", _pos + 1);
            
            if (specifier_pos == std::string::npos)
                return *this;

            if (_format[specifier_pos] == '%') {
                _format.erase(specifier_pos, 1);
                _pos += 1;
                return operator,(rhs);
            }
            
            std::string fmt = _format.substr(_pos + 1, specifier_pos - _pos);

            std::stringstream ss;
            manipulate(ss, fmt);
            ss << rhs;
            const std::string& text = ss.str();
            
            _format.replace(_pos, fmt.length() + 1, text);
            _pos += text.length();
        }
        return *this;
    }

    std::string text() const;
private:
    void manipulate(std::ostream& os, std::string fmt) const;

private:
    std::string _format;
    size_t _pos;
};

inline std::ostream& operator<<(std::ostream& os, const Format& fmt) { return os << fmt.text(); }

#endif // COIN_LOGGER_H