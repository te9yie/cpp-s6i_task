#pragma once

#include <t9_mem/prelude.h>
#include <vector>

namespace s6i_task {

class Resources {
 private:
  template <typename T>
  struct TypeIndex {
    static inline std::size_t ms_index = 0;
  };

  static std::size_t gen_type_index() {
    static std::size_t index = 0;
    return ++index;
  }

 public:
  using Vector = std::vector<void*, t9_mem::StlAllocator<void*>>;

 private:
  Vector m_pointers;

 public:
  Resources(t9_mem::IAllocator* allocator) : m_pointers(allocator) {}

  template <typename T>
  T* get_ptr() {
    auto index = TypeIndex<T>::ms_index;
    if (index == 0 || index >= m_pointers.size()) {
      return nullptr;
    }
    return static_cast<T*>(m_pointers[index]);
  }

  template <typename T>
  T* set_ptr(T* ptr) {
    auto index = TypeIndex<T>::ms_index;
    if (index == 0) {
      index = TypeIndex<T>::ms_index = gen_type_index();
    }
    if (index >= m_pointers.size()) {
      m_pointers.resize(index + 1);
    }
    m_pointers[index] = ptr;
    return ptr;
  }
};

}  // namespace s6i_task
