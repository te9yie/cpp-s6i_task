#pragma once

#include <t9/type_list.h>
#include <bitset>
#include <cassert>
#include <cstddef>
#include "resource_traits.h"

namespace s6i_task {

/**
 * @brief リソースに対するアクセス権限を管理するクラス
 *
 * このクラスは、型ごとのリソースに対する読み取りと書き込みの権限を
 * ビットセットを使用して管理します。各型に一意のインデックスを割り当て、
 * そのインデックスを使用して権限を設定・確認します。
 */
struct Permission {
  /**
   * @brief 型ごとのインデックスを管理する内部クラス
   *
   * @tparam T インデックスを管理する型
   */
  template <typename T>
  struct TypeIndex {
    static inline std::size_t ms_index = 0;  ///< 型に割り当てられたインデックス
  };

  /**
   * @brief 新しい型インデックスを生成する
   *
   * @return std::size_t 生成された一意のインデックス
   */
  static std::size_t gen_type_index() {
    static std::size_t index = 0;
    return ++index;
  }

  static constexpr size_t BIT_N = 128;  ///< ビットセットのサイズ

  std::bitset<BIT_N> m_read_permission;   ///< 読み取り権限のビットセット
  std::bitset<BIT_N> m_write_permission;  ///< 書き込み権限のビットセット

  /**
   * @brief 型に対する読み取り権限を設定する
   *
   * @tparam T 権限を設定する型
   */
  template <typename T>
  void set_read_permission() {
    std::size_t index = TypeIndex<T>::ms_index;
    if (index == 0) {
      index = TypeIndex<T>::ms_index = gen_type_index();
      assert(index < BIT_N);
    }
    m_read_permission.set(index);
  }

  /**
   * @brief 型に対する書き込み権限を設定する
   *
   * @tparam T 権限を設定する型
   */
  template <typename T>
  void set_write_permission() {
    std::size_t index = TypeIndex<T>::ms_index;
    if (index == 0) {
      index = TypeIndex<T>::ms_index = gen_type_index();
      assert(index < BIT_N);
    }
    m_write_permission.set(index);
  }

  /**
   * @brief 型に対する読み取り権限を確認する
   *
   * @tparam T 確認する型
   * @return bool 読み取り権限がある場合はtrue
   */
  template <typename T>
  bool test_read_permission() const {
    std::size_t index = TypeIndex<T>::ms_index;
    if (index == 0) {
      return false;
    }
    return m_read_permission.test(index);
  }

  /**
   * @brief 型に対する書き込み権限を確認する
   *
   * @tparam T 確認する型
   * @return bool 書き込み権限がある場合はtrue
   */
  template <typename T>
  bool test_write_permission() const {
    std::size_t index = TypeIndex<T>::ms_index;
    if (index == 0) {
      return false;
    }
    return m_write_permission.test(index);
  }
};

/**
 * @brief 2つのパーミッション間の競合を確認する
 *
 * 以下の場合に競合があると判断します：
 * - 一方の読み取りと他方の書き込みが競合
 * - 両者の書き込みが競合
 *
 * @param lhs 比較する1つ目のパーミッション
 * @param rhs 比較する2つ目のパーミッション
 * @return bool 競合がある場合はtrue
 */
inline bool is_conflict(const Permission& lhs, const Permission& rhs) {
  return (lhs.m_read_permission & rhs.m_write_permission).any() ||
         (lhs.m_write_permission & rhs.m_read_permission).any() ||
         (lhs.m_write_permission & rhs.m_write_permission).any();
}

namespace detail {

/**
 * @brief 型リストに含まれる全ての型に対して読み取り権限を設定する
 *
 * @tparam Ts 権限を設定する型のリスト
 * @param permission 権限を設定するPermissionオブジェクト
 * @param type_list 型リスト
 */
template <typename... Ts>
inline void set_read_permission(Permission* permission, t9::TypeList<Ts...>) {
  (permission->set_read_permission<Ts>(), ...);
}

/**
 * @brief 型リストに含まれる全ての型に対して書き込み権限を設定する
 *
 * @tparam Ts 権限を設定する型のリスト
 * @param permission 権限を設定するPermissionオブジェクト
 * @param type_list 型リスト
 */
template <typename... Ts>
inline void set_write_permission(Permission* permission, t9::TypeList<Ts...>) {
  (permission->set_write_permission<Ts>(), ...);
}

}  // namespace detail

/**
 * @brief 指定された型に対するパーミッションを生成する
 *
 * ResourceTraitsで定義された読み取りと書き込みの権限を持つ
 * Permissionオブジェクトを生成します。
 *
 * @tparam Ts パーミッションを生成する型のリスト
 * @return Permission 生成されたパーミッションオブジェクト
 */
template <typename... Ts>
inline Permission make_permission() {
  Permission permission;
  (detail::set_read_permission(&permission,
                               typename ResourceTraits<Ts>::read_permission{}),
   ...);
  (detail::set_write_permission(
       &permission, typename ResourceTraits<Ts>::write_permission{}),
   ...);
  return permission;
}

}  // namespace s6i_task
