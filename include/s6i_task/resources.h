#pragma once

#include <t9_mem/prelude.h>
#include <cassert>
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

  class IResource {
   public:
    virtual ~IResource() = default;
  };

  template <typename T>
  class Resource : public IResource {
   public:
    T m_value;

    template <typename... Args>
    Resource(Args&&... args) : m_value(std::forward<Args>(args)...) {}
  };

 public:
  template <typename T>
  using Vector = std::vector<T, t9_mem::StlAllocator<T>>;

 private:
  t9_mem::IAllocator* mp_allocator = nullptr;
  Vector<void*> m_pointers;
  Vector<t9_mem::UniquePtr<IResource>> m_resources;

 public:
  Resources(t9_mem::IAllocator* allocator)
      : mp_allocator(allocator),
        m_pointers(allocator),
        m_resources(allocator) {}

  template <typename T>
  T* get_ptr() {
    auto index = TypeIndex<T>::ms_index;
    if (index == 0 || index >= m_pointers.size()) {
      return nullptr;
    }
    return static_cast<T*>(m_pointers[index]);
  }

  template <typename T>
  T* set(T&& value) {
    auto resource =
        t9_mem::make_unique<Resource<T>>(mp_allocator, std::forward<T>(value));
    assert(resource);
    auto ptr = &resource->m_value;
    m_resources.emplace_back(std::move(resource));
    return set_ptr(ptr);
  }

  template <typename T, typename... Args>
  T* emplace(Args&&... args) {
    auto resource = t9_mem::make_unique<Resource<T>>(
        mp_allocator, std::forward<Args>(args)...);
    assert(resource);
    auto ptr = &resource->m_value;
    m_resources.emplace_back(std::move(resource));
    return set_ptr(ptr);
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
