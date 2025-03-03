#pragma once

#include <t9/func_traits.h>
#include <t9_mem/prelude.h>
#include <cassert>
#include "permission.h"
#include "resource_traits.h"
#include "resources.h"

namespace s6i_task {

/**
 * @brief タスク関数のインターフェース
 *
 * タスク関数の基底クラスです。
 * リソースを受け取って処理を実行する機能を提供します。
 * また、ローカルリソースを保持し、タスク関数内で使用できるようにします。
 */
class ITaskFunc {
 private:
  Permission m_permission;      ///< タスク関数のパーミッション
  Resources m_local_resources;  ///< タスク関数のローカルリソース

 public:
  /**
   * @brief コンストラクタ
   *
   * @param allocator ローカルリソース用のアロケータ
   */
  ITaskFunc(t9_mem::IAllocator* allocator, const Permission& permission)
      : m_permission(permission), m_local_resources(allocator) {}

  /**
   * @brief 仮想デストラクタ
   *
   * 派生クラスのデストラクタが正しく呼ばれるようにします。
   */
  virtual ~ITaskFunc() = default;

  /**
   * @brief パーミッションを取得する
   *
   * @return const Permission& パーミッション
   */
  const Permission& permission() const {
    return m_permission;
  }

  /**
   * @brief タスク関数を実行する
   *
   * 指定されたリソースとローカルリソースを使用してタスク関数を実行します。
   *
   * @param resources 使用するリソース
   */
  void exec(Resources* resources) {
    on_exec(resources, &m_local_resources);
  }

 protected:
  /**
   * @brief タスク関数の実際の処理を実行する
   *
   * 派生クラスでオーバーライドして、タスク関数の実際の処理を実装します。
   *
   * @param resources グローバルリソース
   * @param local_resources ローカルリソース
   */
  virtual void on_exec(Resources* resources, Resources* local_resources) = 0;
};

/**
 * @brief 具体的なタスク関数の実装
 *
 * 関数ポインタを保持し、リソースから引数を取得して関数を呼び出します。
 *
 * @tparam Args 関数の引数の型パック
 */
template <typename... Args>
class TaskFunc : public ITaskFunc {
 private:
  using func_type = void (*)(Args...);  ///< 関数ポインタの型

 private:
  func_type mp_func = nullptr;  ///< 関数ポインタ

 public:
  /**
   * @brief コンストラクタ
   *
   * @param allocator ローカルリソース用のアロケータ
   * @param func 呼び出す関数
   */
  TaskFunc(t9_mem::IAllocator* allocator, func_type func)
      : ITaskFunc(allocator, make_permission<Args...>()), mp_func(func) {}

 protected:
  /**
   * @brief タスク関数の実際の処理を実行する
   *
   * リソースから引数を取得して関数を呼び出します。
   *
   * @param resources グローバルリソース
   * @param local_resources ローカルリソース
   */
  void on_exec(Resources* resources, Resources* local_resources) override {
    assert(mp_func);
    mp_func(ResourceTraits<Args>::get_ptr(resources, local_resources)...);
  }
};

namespace detail {

/**
 * @brief タスク関数を作成するヘルパー関数
 *
 * 関数と引数の型リストからTaskFuncオブジェクトを作成します。
 *
 * @tparam F 関数の型
 * @tparam Args 関数の引数の型パック
 * @param allocator 使用するアロケータ
 * @param f 関数
 * @param 引数の型リスト
 * @return t9_mem::UniquePtr<ITaskFunc> 作成したTaskFuncオブジェクト
 */
template <typename F, typename... Args>
inline t9_mem::UniquePtr<ITaskFunc>
make_task_func(t9_mem::IAllocator* allocator, F&& f, t9::TypeList<Args...>) {
  return t9_mem::make_unique<TaskFunc<Args...>>(allocator, allocator,
                                                std::forward<F>(f));
}

}  // namespace detail

/**
 * @brief タスク関数を作成する
 *
 * 関数からTaskFuncオブジェクトを作成します。
 * 関数の引数の型は自動的に推論されます。
 *
 * @tparam F 関数の型
 * @param allocator 使用するアロケータ
 * @param f 関数
 * @return t9_mem::UniquePtr<ITaskFunc> 作成したTaskFuncオブジェクト
 */
template <typename F>
inline t9_mem::UniquePtr<ITaskFunc> make_task_func(
    t9_mem::IAllocator* allocator,
    F&& f) {
  using args_type_list =
      typename t9::FuncTraits<std::decay_t<F>>::args_type_list;
  return detail::make_task_func(allocator, std::forward<F>(f),
                                args_type_list{});
}

}  // namespace s6i_task
