// simple_vector.h

#ifndef _SIMPLE_VECTOR_H_
#define _SIMPLE_VECTOR_H_

#include "simple.h"
#include "std_port/std_port.h"
#include "std_port/std_port_util.h"

namespace simple
{

template <typename K>
class vector_iterator;

template <typename T>
class vector
{
public:
    typedef vector_iterator<T> iterator;
    friend iterator;

public:
    vector(const vector &vec) :
        vector(vec.m_space)
    {
        *this = vec;
    }

    // Construct from T[] with size
    vector(const T *arr, size_t count)
    {
        m_space = count;
        m_size = count;
        m_array = XNEWN(T, count);
        for (size_t i = 0; i < count; i++)
            m_array[i] = arr[i];
    }

    vector(size_t capacity = 8)
    {
        // Allocate an empty array with reserved space
        if (capacity < 1)
            capacity = 1;
        m_space = capacity;
        m_array = XNEWN(T, m_space);
        m_size = 0;
    }

    ~vector()
    {
        if (m_array)
            XDELETEN(m_array);
        STD_DEBUG_SET_NULL(m_array);
    }

    vector& operator = (const vector& vec)
    {
        if (m_space < vec.size())
        {
            // Reallocate array
            XDELETEN(m_array);
            m_space = vec.size();
            m_array = XNEWN(T, m_space);
        }

        m_size = vec.size();
        for (size_t i = 0; i < m_size; i++)
            m_array[i] = vec.m_array[i];

        return *this;
    }

    vector& operator = (vector&& vec)
    {
        if (m_array)
            XDELETEN(m_array);

        // Steal m_array from vec
        m_space = vec.m_space;
        m_size = vec.m_size;
        m_array = vec.m_array;

        return *this;
    }

    T& operator [](size_t index) const
    {
        if (index >= m_size)
            throw "Index is out of range in vector.";

        return m_array[index];
    }

    // Clear all elements
    void clear()
    {
        T zero = T();
        for (size_t i = 0; i < m_size; i++)
            m_array[i] = zero;
        m_size = 0;
    }

    // Enlarge space to double size
    void extend()
    {
        T *new_array;
        m_space *= 2;
        new_array = XNEWN(T, m_space);
        for (size_t i = 0; i < m_size; i++)
            new_array[i] = simple::move(m_array[i]);
        XDELETEN(m_array);
        m_array = new_array;
    }

    // Reserve size
    void reserve(size_t to)
    {
        if (to < m_size)
        {
            // Do nothing
            STD_ASSERT(("Reseve size is smaller than existed size.\n", 0));
            return;
        }

        if (to == m_space)
            // Size ok
            return;

        T *new_array;
        m_space = to;
        new_array = XNEWN(T, m_space);
        for (size_t i = 0; i < m_size; i++)
            new_array[i] = simple::move(m_array[i]);
        XDELETEN(m_array);
        m_array = new_array;
    }

    // Find element in vector
    iterator find(const T& element)
    {
        for (auto i = 0; i < m_size; i++)
            if (m_array[i] == element)
                return iterator(*this, i);

        // Not found
        return end();
    }

    inline void push_back(const T& element)
    {
        if (m_size >= m_space)
        {
            // Allocate new array with 2* size & move all elements
            STD_ASSERT(m_size == m_space);
            extend();
        }

        m_array[m_size++] = element;
    }

    inline void push_back(T&& element)
    {
        if (m_size >= m_space)
        {
            // Allocate new array with 2* size & move all elements
            STD_ASSERT(m_size == m_space);
            extend();
        }

        m_array[m_size++] = simple::move(element);
    }

    void push_backs(const T& e, size_t count)
    {
        STD_ASSERT(count >= 0);
        size_t new_size = m_size + count;
        // Extend size when necessary
        while (new_size > m_space)
            extend();
        // Put all values
        while (m_size < new_size)
            m_array[m_size++] = e;
    }

    // Remove an element @ index
    void remove(size_t index)
    {
        if (index >= m_size)
            throw "Element is out of range when remove from array.";
        for (size_t i = index; i < m_size; i++)
            m_array[i] = simple::move(m_array[i + 1]);
    }

    // Remove an element @ it
    void remove(iterator& it)
    {
        remove((size_t) (it->m_cursor_ptr - m_array));
    }

    // Get length of existed elements
    size_t size() const
    {
        return m_size;
    }

protected:
    // Query array for unsafe operating
    T *get_array_unsafe() const
    {
        return m_array;
    }

    // Iterator relatives
public:
    // Begin of container
    iterator begin()
    {
        return iterator(*this, 0);
    }

    // End of container
    iterator end()
    {
        iterator it(*this, m_size);
        return it;
    }

private:
    // hash table
    T     *m_array;
    size_t m_size, m_space;
};

// Iterator of container
template<typename T>
class vector_iterator
{
    typedef vector<T> vector_type;
    typedef T value_type;
    friend vector_type;

public:
    vector_iterator() :
#ifdef _DEBUG
        m_vec(0),
        m_size(0),
#endif
        m_cursor_ptr(0),
        m_index(0)
    {
    }

private:
    // Construct iterator by mapping & type (Begin, End)
    vector_iterator(vector_type& vec, size_t index)
    {
#ifdef _DEBUG
        m_vec = &vec;
        m_size = vec.size();
#endif
        m_cursor_ptr = &vec.m_array[index];
        m_index = index;
    }

public:
    size_t get_index()
    {
        return m_index;
    }

public:
    value_type& operator * ()
    {
        STD_ASSERT(m_size == m_vec->size());
        STD_ASSERT(*this < m_vec->end());
        return *m_cursor_ptr;
    }

    value_type *operator -> ()
    {
        STD_ASSERT(m_size == m_vec->size());
        STD_ASSERT(*this < m_vec->end());
        return m_cursor_ptr;
    }

    // Move to next
    vector_iterator& operator ++ ()
    {
        m_cursor_ptr++;
        m_index++;
        return *this;
    }

    vector_iterator operator ++ (int)
    {
        vector_iterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator == (const vector_iterator& it) const
    {
        return m_index == it.m_index;
    }

    bool operator < (const vector_iterator& it) const
    {
        return m_index < it.m_index;
    }

private:
    // Iterator of map
#ifdef _DEBUG
    vector_type *m_vec;
    size_t m_size;
#endif
    value_type *m_cursor_ptr;
    size_t m_index;
};

// Unsafe vector (don't do bound check when index)
template<typename T>
class unsafe_vector : public vector<T>
{
public:
    unsafe_vector(size_t capacity = 8) :
        vector<T>(capacity)
    {
    }

    T& operator [] (size_t index) const
    {
        STD_ASSERT(index < size());
        return vector<T>::get_array_unsafe()[index];
    }

    // Return the address of element index
    T *get_array_address(size_t index) const
    {
        STD_ASSERT(index <= size());
        return &vector<T>::get_array_unsafe()[index];
    }
};

} // End of namespace: simple

#endif
