//
// Created by krotk on 08.09.2018.
//

#ifndef FIXED_VECTOR_FIXED_VECTOR_H
#define FIXED_VECTOR_FIXED_VECTOR_H

#include <cstddef>

template<typename T, size_t SI = 1>
class fixed_vector {
    size_t size_;
    typename std::aligned_storage<sizeof(T), alignof(T)>::type data_[SI];
public:

    template<typename U>
    class Iterator {
    public:
        using difference_type = ptrdiff_t;
        using value_type = U;
        using pointer = U *;
        using reference = U&;
        using iterator_category = std::random_access_iterator_tag;

        friend class fixed_vector;

        Iterator() {
            pos = 0;
            vector = nullptr;
        }

        template<typename C>
        Iterator(const Iterator<C> &other) {
            pos = other.pos;
            vector = other.vector;
        }

        U& operator*() const{
            return *(const_cast<T*>(reinterpret_cast<T const *>(vector->data_ + pos)));
        }

        U* operator->() const {
            return reinterpret_cast<T const *>(vector->data_ + pos);
        }

        Iterator<U>& operator++() {
            pos++;
            if (pos == vector->size_+1) pos = 0;
            return *this;
        }

        Iterator<U> operator++(int) {
            size_t copy = pos;
            ++(*this);
            return Iterator(copy, vector);
        }

        Iterator<U>& operator--() {
            pos--;
            if (pos == -1) pos = vector->size_;
            return *this;
        }

        Iterator<U> operator--(int) {
            size_t copy = pos;
            --(*this);
            return Iterator(copy, vector);
        }

        Iterator<U>& operator+=(ptrdiff_t x) {
            pos = (pos + x) % vector->size_;
            return *this;
        }

        Iterator<U>& operator-=(ptrdiff_t x) {
            pos = (pos - x + vector->size_) % vector->size_;
            return *this;
        }

        Iterator<U>& operator+(ptrdiff_t x) const {
            return Iterator((pos + x) % vector->size_, vector);
        }

        Iterator<U>& operator-(ptrdiff_t x) const {
            return Iterator((pos - x + vector->size_) % vector->size_, vector);
        }

        friend ptrdiff_t operator-(Iterator<U> const& first, Iterator<U> const& second) {
            return first->pos - second->pos;
        }

        friend bool operator<(Iterator<U> const& first, Iterator<U> const& second) {
            return first->pos - second->pos;
        }

        friend bool operator>(Iterator<U> const& first, Iterator<U> const& second) {
            return second < first;
        }

        friend bool operator<=(Iterator<U> const& first, Iterator<U> const& second) {
            return !(first > second);
        }

        friend bool operator>=(Iterator<U> const& first, Iterator<U> const& second) {
            return !(first < second);
        }

        friend bool operator==(Iterator<U> const& first, Iterator<U> const& second) {
            return  first.pos == second.pos;
        }

        friend bool operator!=(Iterator<U> const& first, Iterator<U> const& second) {
            return first.pos != second.pos;
        }

    private:

        Iterator(size_t p, fixed_vector const * vec) {
            pos = p;
            vector = vec;
        }

        size_t pos;
        const fixed_vector* vector;
    };

    using iterator = Iterator<T>;
    using const_iterator = Iterator<T const>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    iterator begin() {
        return iterator(0, this);
    }

    iterator end() {
        return iterator(size_, this);
    }

    const_iterator begin() const {
        return const_iterator(0, this);
    }

    const_iterator end() const {
        return const_iterator(size_, this);
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rbegin() const{
        return const_reverse_iterator(end());
    }

    const_reverse_iterator rend() const{
        return const_reverse_iterator(begin());
    }

    fixed_vector() : size_(0) {}
    fixed_vector(fixed_vector<T, SI> const & other) : fixed_vector() {
        size_ = other.size_;
    }

    fixed_vector<T, SI>& operator=(fixed_vector<T, SI> const & other) {
        swap(*this, fixed_vector(other));
        return *this;
    }

    void push_back(T const & value) {
        new(data_ + size_) T(value);
        size_++;
    }

    void pop_back() {
        reinterpret_cast<T const *>(data_ + size_ - 1)->~T();
        size_--;
    }

    void push_front(T const & value) {
        for (int i = size_-1; i >= 0; i--) {
            new(data_ + i + 1) T(*reinterpret_cast<T const *>(data_ + i));
            reinterpret_cast<T const *>(data_ + i)->~T();
        }
        new(data_ + 0) T(value);
        size_++;
    }

    void pop_front() {
        for (int i = 1; i < size_; i++) {
            reinterpret_cast<T const *>(data_ + i)->~T();
            new(data_ + i - 1) T(*reinterpret_cast<T const *>(data_ + i));
            reinterpret_cast<T const *>(data_ + i)->~T();
        }
        size_--;
    }

    size_t size() const {
        return size_;
    }

    size_t capacity() const {
        return SI;
    }

    size_t max_size() const {
        return SI;
    }

    bool empty() const {
        return (size_ == 0);
    }

    T operator[](int ind) const {
        return *reinterpret_cast<T *>(data_ + ind);
    }

    T& operator[](int ind) {
        return *reinterpret_cast<T *>(data_ + ind);
    }

    T& back() {
        return *reinterpret_cast<T*>(data_ + size_ - 1);
    }

    T& front() {
        return *reinterpret_cast<T*>(data_);
    }

    void clear() {
        for (int i = 0; i < size_; i++) {
            reinterpret_cast<T const *>(data_ + i)->~T();
        }
        size_ = 0;
    }

    iterator insert(iterator ind, T const & value) {
        int p = ind.pos;
        if (size_ != 0) {
            for (int i = size_ - 1; i >= p; i--) {
                new(data_ + i + 1) T(*reinterpret_cast<T const *>(data_ + i));
                reinterpret_cast<T const *>(data_ + i)->~T();
            }
        }
        new(data_ + p) T(value);
        size_++;
        return iterator(p, this);
    }

    void erase(iterator ind) {
        size_t p = ind.pos;
        for (size_t i = p; i < size_; i++) {
            reinterpret_cast<T const *>(data_ + i)->~T();
            if (i != size_-1) new(data_ + i) T(*reinterpret_cast<T const *>(data_ + i + 1));
        }
        size_--;
    }

    friend void swap(fixed_vector<T, SI> & first, fixed_vector<T, SI> & second) {
        std::swap(first.data_, second.data_);
        std::swap(first.size_, second.size_);
    }

};

#endif //FIXED_VECTOR_FIXED_VECTOR_H
