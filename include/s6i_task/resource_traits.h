#pragma once

#include <t9/type_list.h>
#include "resources.h"

namespace s6i_task {

/**
 * @brief リソースの特性を定義するトレイト
 *
 * このテンプレート構造体は、リソースの型に関する特性を定義します。
 * 特殊化によって、ポインタ型に対する振る舞いを定義します。
 *
 * @tparam T リソースの型
 */
template <typename T>
struct ResourceTraits;

/**
 * @brief 書き込み可能なポインタに対するリソース特性
 *
 * 書き込み可能なポインタ型に対するリソース特性を定義します。
 * 読み取りと書き込みの両方の権限を持ちます。
 *
 * @tparam T ポインタの参照先の型
 */
template <typename T>
struct ResourceTraits<T*> {
  using resource_type_list = t9::TypeList<T>;  ///< リソースの型リスト
  using read_permission = t9::TypeList<T>;     ///< 読み取り権限のある型リスト
  using write_permission = t9::TypeList<T>;    ///< 書き込み権限のある型リスト

  /**
   * @brief リソースからポインタを取得する
   *
   * 指定されたリソースから型Tのポインタを取得します。
   *
   * @param resources グローバルリソース
   * @param local_resources ローカルリソース（使用しない）
   * @return T* 型Tのポインタ
   */
  static T* get_ptr(Resources* resources, Resources* /*local_resources*/) {
    return resources->get_ptr<T>();
  }
};

/**
 * @brief 読み取り専用ポインタに対するリソース特性
 *
 * 読み取り専用（const）ポインタ型に対するリソース特性を定義します。
 * 読み取り権限のみを持ち、書き込み権限はありません。
 *
 * @tparam T ポインタの参照先の型
 */
template <typename T>
struct ResourceTraits<const T*> {
  using resource_type_list = t9::TypeList<T>;  ///< リソースの型リスト
  using read_permission = t9::TypeList<T>;     ///< 読み取り権限のある型リスト
  using write_permission =
      t9::TypeList<>;  ///< 書き込み権限のある型リスト（空）

  /**
   * @brief リソースから読み取り専用ポインタを取得する
   *
   * 指定されたリソースから型Tの読み取り専用ポインタを取得します。
   *
   * @param resources グローバルリソース
   * @param local_resources ローカルリソース（使用しない）
   * @return const T* 型Tの読み取り専用ポインタ
   */
  static const T* get_ptr(Resources* resources,
                          Resources* /*local_resources*/) {
    return resources->get_ptr<T>();
  }
};

}  // namespace s6i_task
