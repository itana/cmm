// cmm_string_pool.cpp

#include "std_port/std_port_os.h"
#include "cmm_string_pool.h"

namespace cmm
{

StringPool::StringPool()
{
    std_init_spin_lock(&m_lock);
}

StringPool::~StringPool()
{
    for (auto it = m_pool.begin(); it != m_pool.end(); ++it)
        XDELETE(it->second);
    std_destroy_spin_lock(&m_lock);
}
    
// Find string in pool, create new one if not found
// Once a string put in pool, it should be never deleted, so it should be
// a CONSTANT one
String *StringPool::find_or_insert(const simple::string& str)
{
    String *string_in_pool;

    std_get_spin_lock(&m_lock);
    if (!m_pool.try_get(str, &string_in_pool))
    {
        // Not found in pool, create new "CONSTANT" string
        string_in_pool = XNEW(String, str);
        string_in_pool->attrib |= (ReferenceValue::CONSTANT | ReferenceValue::SHARED);
        m_pool.put(str, string_in_pool);
    }
    std_release_spin_lock(&m_lock);

    return string_in_pool;
}

// Find only
String *StringPool::find(const simple::string& str)
{
    String *string_in_pool;
    bool is_existed;

    std_get_spin_lock(&m_lock);
    is_existed = m_pool.try_get(str, &string_in_pool);
    std_release_spin_lock(&m_lock);

    return is_existed ? string_in_pool : 0;
}

}