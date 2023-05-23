//****************************************************************************
// Copyright © 2023 Jan Erik Breimo. All rights reserved.
// Created by Jan Erik Breimo on 2023-05-06.
//
// This file is distributed under the BSD License.
// License text is included with the source distribution.
//****************************************************************************
#pragma once
#include <iterator>

template <typename T, unsigned N>
class RingBufferIterator
{
public:
    using difference_type = ptrdiff_t;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = std::bidirectional_iterator_tag;

    RingBufferIterator(const T* current, const T* first)
        : current_(current), first_(first)
    {
    }

    [[nodiscard]]
    const T& operator*() const
    {
        return *current_;
    }

    const T* operator->() const
    {
        return current_;
    }

    RingBufferIterator& operator++()
    {
        if (std::distance(first_, ++current_) == N + 1)
            current_ = first_;
        return *this;
    }

    RingBufferIterator operator++(int)
    {
        auto result = *this;
        if (std::distance(first_, ++current_) == N + 1)
            current_ = first_;
        return result;
    }

    RingBufferIterator& operator--()
    {
        if (first_ != current_)
            --current_;
        else
            current_ = first_ + N;
        return *this;
    }

    RingBufferIterator operator--(int)
    {
        auto result = *this;
        if (first_ != current_)
            --current_;
        else
            current_ = first_ + N;
        return result;
    }

    [[nodiscard]]
    friend bool operator==(const RingBufferIterator& a, const RingBufferIterator& b)
    {
        return a.current_ == b.current_ && a.first_ == b.first_;
    }

    [[nodiscard]]
    friend bool operator!=(const RingBufferIterator& a, const RingBufferIterator& b)
    {
        return a.current_ != b.current_ || a.first_ != b.first_;
    }

private:
    const T* current_ = nullptr;
    const T* first_ = nullptr;
};

template <typename T, unsigned N>
class RingBuffer
{
public:
    using iterator = RingBufferIterator<T, N>;

    void push(T value)
    {
        auto index = (start_index_ + size_) % (N + 1);
        values_[index] = std::move(value);
        if (size_ == N)
            start_index_ = (start_index_ + 1) % (N + 1);
        else
            size_++;
    }

    [[nodiscard]]
    const T& operator[](size_t i) const
    {
        return values_[(start_index_ + i) % (N + 1)];
    }

    [[nodiscard]]
    T& operator[](size_t i)
    {
        return values_[(start_index_ + i) % (N + 1)];
    }

    [[nodiscard]]
    size_t size() const
    {
        return size_;
    }

    [[nodiscard]]
    bool empty() const
    {
        return size_ == 0;
    }

    [[nodiscard]]
    const T& front() const
    {
        return values_[start_index_];
    }

    [[nodiscard]]
    T& front()
    {
        return values_[start_index_];
    }

    [[nodiscard]]
    const T& back() const
    {
        return values_[(start_index_ + size_ - 1) % (N + 1)];
    }

    [[nodiscard]]
    T& back()
    {
        return values_[(start_index_ + size_ - 1) % (N + 1)];
    }

    void clear()
    {
        start_index_ = 0;
        size_ = 0;
    }

    [[nodiscard]]
    iterator begin() const
    {
        return iterator(&values_[start_index_], &values_[0]);
    }

    [[nodiscard]]
    iterator end() const
    {
        auto index = (start_index_ + size_) % (N + 1);
        return iterator(&values_[index], &values_[0]);
    }

private:
    // Value number N+1 is a sentinel value marking the end og the buffer.
    T values_[N + 1];
    size_t start_index_ = 0;
    size_t size_ = 0;
};
