#pragma once

#include <t9_mem/prelude.h>
#include <cassert>
#include <utility>
#include <vector>

namespace s6i_task {

/**
 * @brief 型ごとのポインタを保持して取り出すことができるリソース管理クラス
 *
 * このクラスは型ごとに1つのポインタを保持し、型情報を使って取り出すことができます。
 * また、リソースの所有権を管理し、デストラクタで逆順に解放します。
 */
class Resources final {
 private:
  /// コピーコンストラクタは削除（コピー禁止）
  Resources(const Resources&) = delete;
  /// コピー代入演算子は削除（コピー禁止）
  Resources& operator=(const Resources&) = delete;

 private:
  /**
   * @brief 型ごとのインデックスを保持する構造体
   *
   * 各型に一意のインデックスを割り当てるために使用します。
   * 静的変数ms_indexに型ごとのインデックスを保持します。
   *
   * @tparam T インデックスを割り当てる型
   */
  template <typename T>
  struct TypeIndex {
    static inline std::size_t ms_index = 0;
  };

  /**
   * @brief 新しい型インデックスを生成する
   *
   * 呼び出されるたびに一意のインデックスを返します。
   *
   * @return std::size_t 新しい型インデックス
   */
  static std::size_t gen_type_index() {
    static std::size_t index = 0;
    return ++index;
  }

  /**
   * @brief リソースのインターフェースクラス
   *
   * すべてのリソースの基底クラスとして使用されます。
   * 仮想デストラクタを持ち、派生クラスのデストラクタが正しく呼ばれるようにします。
   */
  class IResource {
   public:
    virtual ~IResource() = default;
  };

  /**
   * @brief 型Tのリソースを保持するクラス
   *
   * IResourceを継承し、実際の値を保持します。
   *
   * @tparam T 保持する値の型
   */
  template <typename T>
  class Resource : public IResource {
   public:
    T m_value;  ///< 保持する値

    /**
     * @brief コンストラクタ
     *
     * 可変引数テンプレートを使用して、任意の引数で値を構築します。
     *
     * @tparam Args 引数の型パック
     * @param args 値の構築に使用する引数
     */
    template <typename... Args>
    Resource(Args&&... args) : m_value(std::forward<Args>(args)...) {}
  };

 public:
  /**
   * @brief アロケータを使用するベクター型
   *
   * t9_mem::StlAllocatorを使用するstd::vectorの別名です。
   *
   * @tparam T ベクターの要素の型
   */
  template <typename T>
  using Vector = std::vector<T, t9_mem::StlAllocator<T>>;

 private:
  t9_mem::IAllocator* mp_allocator = nullptr;  ///< アロケータへのポインタ
  Vector<void*> m_pointers;  ///< 型ごとのポインタを保持するベクター
  Vector<t9_mem::UniquePtr<IResource>>
      m_resources;  ///< リソースの所有権を保持するベクター

 public:
  /**
   * @brief コンストラクタ
   *
   * @param allocator 使用するアロケータへのポインタ
   */
  Resources(t9_mem::IAllocator* allocator)
      : mp_allocator(allocator),
        m_pointers(allocator),
        m_resources(allocator) {}

  /**
   * @brief デストラクタ
   *
   * リソースを逆順に解放します。
   */
  ~Resources() {
    while (!m_resources.empty()) {
      m_resources.pop_back();
    }
  }

  /**
   * @brief ムーブコンストラクタ
   *
   * 他のResourcesオブジェクトからリソースを移動します。
   *
   * @param other 移動元のResourcesオブジェクト
   */
  Resources(Resources&& other) noexcept
      : mp_allocator(std::move(other.mp_allocator)),
        m_pointers(std::move(other.m_pointers)),
        m_resources(std::move(other.m_resources)) {}

  /**
   * @brief ムーブ代入演算子
   *
   * 他のResourcesオブジェクトからリソースを移動します。
   *
   * @param other 移動元のResourcesオブジェクト
   * @return Resources& *thisへの参照
   */
  Resources& operator=(Resources&& other) noexcept {
    Resources(std::move(other)).swap(*this);
    return *this;
  }

  /**
   * @brief 型Tのポインタを取得する
   *
   * 型Tに対応するポインタを返します。
   * 型Tのポインタが設定されていない場合はnullptrを返します。
   *
   * @tparam T 取得するポインタの型
   * @return T* 型Tのポインタ、または設定されていない場合はnullptr
   */
  template <typename T>
  T* get_ptr() {
    auto index = TypeIndex<T>::ms_index;
    if (index == 0 || index >= m_pointers.size()) {
      return nullptr;
    }
    return static_cast<T*>(m_pointers[index]);
  }

  /**
   * @brief 値を設定する
   *
   * 値のコピーを作成し、所有権を取得します。
   * 同じ型の値が既に設定されている場合は上書きします。
   *
   * @tparam T 設定する値の型
   * @param value 設定する値
   * @return T* 設定した値へのポインタ
   */
  template <typename T>
  T* set(T&& value) {
    auto resource =
        t9_mem::make_unique<Resource<T>>(mp_allocator, std::forward<T>(value));
    assert(resource);
    auto ptr = &resource->m_value;
    m_resources.emplace_back(std::move(resource));
    return set_ptr(ptr);
  }

  /**
   * @brief 値をその場で構築する
   *
   * 引数から直接値を構築し、所有権を取得します。
   * 同じ型の値が既に設定されている場合は上書きします。
   *
   * @tparam T 構築する値の型
   * @tparam Args コンストラクタ引数の型パック
   * @param args 値の構築に使用する引数
   * @return T* 構築した値へのポインタ
   */
  template <typename T, typename... Args>
  T* emplace(Args&&... args) {
    auto resource = t9_mem::make_unique<Resource<T>>(
        mp_allocator, std::forward<Args>(args)...);
    assert(resource);
    auto ptr = &resource->m_value;
    m_resources.emplace_back(std::move(resource));
    return set_ptr(ptr);
  }

  /**
   * @brief ポインタを設定する
   *
   * 型Tのポインタを設定します。
   * 同じ型のポインタが既に設定されている場合は上書きします。
   * このメソッドは所有権を取得しません。
   *
   * @tparam T 設定するポインタの型
   * @param ptr 設定するポインタ
   * @return T* 設定したポインタ
   */
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

  /**
   * @brief 他のResourcesオブジェクトと内容を交換する
   *
   * @param other 交換先のResourcesオブジェクト
   */
  void swap(Resources& other) {
    using std::swap;
    swap(mp_allocator, other.mp_allocator);
    swap(m_pointers, other.m_pointers);
    swap(m_resources, other.m_resources);
  }

  /**
   * @brief 2つのResourcesオブジェクトの内容を交換する
   *
   * @param lhs 左辺のResourcesオブジェクト
   * @param rhs 右辺のResourcesオブジェクト
   */
  friend void swap(Resources& lhs, Resources& rhs) {
    lhs.swap(rhs);
  }
};

}  // namespace s6i_task
