#ifndef RIRUD_H
#define RIRUD_H

#include <cstdint>
#include <vector>
#include <string>

#include <string>
#include "buff_string.h"

class RirudSocket {
public:
    constexpr static std::string_view RIRUD = "rirud";

    enum class Action : uint32_t {
        READ_FILE = 4,
        READ_DIR = 5,

        // used by riru itself only, could be removed in the future
        WRITE_STATUS = 2,
        READ_NATIVE_BRIDGE = 3,
        READ_RIRU_TMPFS_PATH = 6,

        READ_MODULES = 7,
    };

    enum class CODE : uint8_t {
        OK = 0,
        FAILED = 1,
    };

    class DirIter {
        constexpr static uint8_t continue_read = true;
        constexpr static size_t MAX_PATH_SIZE = 256u;
    private:
        DirIter(std::string_view path, const RirudSocket &socket) : socket_(socket), path({'\0'}) {
            int32_t reply;
            if (socket_.Write(Action::READ_DIR) && socket_.Write(path) && socket_.Read(reply) && reply == 0) {
                ContinueRead();
            }
        }

        void ContinueRead();

        DirIter(const DirIter &) = delete;

        DirIter operator=(const DirIter &) = delete;


        const RirudSocket &socket_;
        std::array<char, MAX_PATH_SIZE> path;

        friend class RirudSocket;

    public:
        operator bool() {
            return path[0];
        }

        DirIter &operator++() {
            ContinueRead();
            return *this;
        }

        std::string_view operator*() {
            return {path.data()};
        }
    };

    friend class RirudSocket::DirIter;

    bool valid() const {
        return fd_ != -1;
    }

    RirudSocket(unsigned retries = 1);

    std::string ReadFile(std::string_view path);

    std::string ReadRiruModulesPath() const;

    std::string ReadNativeBridge() const;

    DirIter ReadDir(std::string_view path) const {
        return {path, *this};
    }

    template<typename T>
    std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>, bool>
    Read(T &obj) const {
        return Read(reinterpret_cast<void *>(&obj), sizeof(T));
    }

    bool Read(std::string &str) const;

    template<typename T>
    std::enable_if_t<std::is_fundamental_v<T> || std::is_enum_v<T>, bool>
    Write(const T &obj) const {
        return Write(&obj, sizeof(T));
    }

    bool Write(std::string_view str) const;

    ~RirudSocket();

private:
    RirudSocket(const RirudSocket &) = delete;

    RirudSocket operator=(const RirudSocket &) = delete;

    bool Write(const void *buf, size_t len) const;

    bool Read(void *buf, size_t len) const;

    int fd_ = -1;
};

#endif //RIRUD_H
