//
// Created by Cheng MingBo on 2023/2/12.
//

#ifndef BUF_BUFFER_H
#define BUF_BUFFER_H

#include <vector>
#include <string_view>
#include <algorithm>
#include <cstring>
#include <string>
#include <string_view>

class Buffer {
  public:
    explicit Buffer(std::size_t initialSize = kInitialSize) :
            buffer_(kInitialSize + kCheapPrepend), readerIndex_(kCheapPrepend), writerIndex_(kCheapPrepend) {
        static_assert(kCheapPrepend >= kPrependSize, "kCheapPrepend must >= kPrependSize");
    }
    
    void swap(Buffer &rhs) {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }
    
    template<typename T>
    void append(const T &data) {
        append(static_cast<const char *>(&*data.begin()), data.size());
    }
    
    template<typename T>
    void append(const T* data) {
        append(static_cast<const char *>(data), strlen(data));
    }
    
    void append(const char *data, std::size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }
    
    [[nodiscard]] std::string toString(std::size_t len) const {
        auto str = std::string(peek(), peek() + len);
        return str;
    }
    
    std::string retrieveAsString(std::size_t len) {
        auto str = toString(len);
        retrieve(str.size());
        return str;
    }
    
    std::string retrieveAllAsString() {
        auto str = toString(readableBytes());
        retrieve(str.size());
        return str;
    }
    
    [[nodiscard]] std::string_view toStringView() const {
        auto str_view = std::string_view(peek(), readableBytes());
        return str_view;
    }
    
    std::string_view retrieveAsStringView() {
        auto str = toStringView();
        retrieve(str.size());
        return str;
    }
    
    void retrieve(std::size_t len) {
        if (len < readableBytes()) {
            readerIndex_ += len;
        }
        else {
            retrieveAll();
        }
    }
    
    void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }
    
    void prepend(const void *data, std::size_t len) {
        readerIndex_ -= len;
        const char *d = static_cast<const char *>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }
    
    void prependInt8(int8_t x) {
        prepend(&x, sizeof x);
    }
    
    [[nodiscard]] int8_t peekInt8() const {
        int8_t x = *peek();
        return x;
    }
    
    void retrieveInt8() {
        retrieve(sizeof(int8_t));
    }
    
    int8_t readInt8() {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }
    
    void shrink(std::size_t reserve) {
        buffer_.shrink_to_fit();
    }
    
    [[nodiscard]] std::size_t writableBytes() const { return buffer_.size() - writerIndex_; }
    
    [[nodiscard]] std::size_t readableBytes() const { return writerIndex_ - readerIndex_; }
    
    [[nodiscard]] std::size_t prependableBytes() const { return readerIndex_; }
    
    char *peek() { return begin() + readerIndex_; }
    
    [[nodiscard]] const char *peek() const { return begin() + readerIndex_; }
    
    static constexpr std::size_t kCheapPrepend = 8;
    static constexpr std::size_t kInitialSize = 1024;
    static constexpr std::size_t kPrependSize = 8;
  
  private:
    
    char *begin() { return &*buffer_.begin(); }
    
    [[nodiscard]] const char *begin() const { return &*buffer_.begin(); }
    
    char *beginWrite() { return begin() + writerIndex_; }
    
    [[nodiscard]] const char *beginWrite() const { return begin() + writerIndex_; }
    
    
    void ensureWritableBytes(std::size_t len) {
        if (writableBytes() < len) {
            makeSpace(len);
        }
    }
    
    void makeSpace(std::size_t len) {
        if (writableBytes() + prependableBytes() < len + kPrependSize) {
            buffer_.resize(writerIndex_ + len);
        }
        else {
            auto readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_, begin() + kCheapPrepend);
            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
        }
    }
    
    void hasWritten(std::size_t len) { writerIndex_ += len; }
    
    std::vector<char> buffer_;
    std::size_t readerIndex_;
    std::size_t writerIndex_;
};


#endif //BUF_BUFFER_H
