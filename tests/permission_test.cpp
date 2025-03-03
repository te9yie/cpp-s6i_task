#include <gtest/gtest.h>

namespace {

struct ResourceA {};
struct ResourceB {};
struct ResourceC {};

TEST(PermissionTest, SetAndTestPermission) {
  s6i_task::Permission permission;

  // 初期状態ではパーミッションは設定されていない
  EXPECT_FALSE(permission.test_read_permission<ResourceA>());
  EXPECT_FALSE(permission.test_write_permission<ResourceA>());

  // 読み取りパーミッションの設定
  permission.set_read_permission<ResourceA>();
  EXPECT_TRUE(permission.test_read_permission<ResourceA>());
  EXPECT_FALSE(permission.test_write_permission<ResourceA>());

  // 書き込みパーミッションの設定
  permission.set_write_permission<ResourceB>();
  EXPECT_TRUE(permission.test_write_permission<ResourceB>());
  EXPECT_FALSE(permission.test_read_permission<ResourceB>());
}

TEST(PermissionTest, ConflictCheck) {
  s6i_task::Permission p1, p2;

  // 読み取り vs 書き込みの競合
  p1.set_read_permission<ResourceA>();
  p2.set_write_permission<ResourceA>();
  EXPECT_TRUE(s6i_task::is_conflict(p1, p2));
  EXPECT_TRUE(s6i_task::is_conflict(p2, p1));

  // 書き込み vs 書き込みの競合
  s6i_task::Permission p3, p4;
  p3.set_write_permission<ResourceB>();
  p4.set_write_permission<ResourceB>();
  EXPECT_TRUE(s6i_task::is_conflict(p3, p4));
  EXPECT_TRUE(s6i_task::is_conflict(p4, p3));

  // 競合しないケース
  s6i_task::Permission p5, p6;
  p5.set_read_permission<ResourceA>();
  p6.set_read_permission<ResourceB>();
  EXPECT_FALSE(s6i_task::is_conflict(p5, p6));
  EXPECT_FALSE(s6i_task::is_conflict(p6, p5));
}

TEST(PermissionTest, MakePermission) {
  // ResourceA: 読み取りのみ
  auto p1 = s6i_task::make_permission<const ResourceA*>();
  EXPECT_TRUE(p1.test_read_permission<ResourceA>());
  EXPECT_FALSE(p1.test_write_permission<ResourceA>());

  // ResourceB: 読み取り（A, B）と書き込み（B）
  auto p2 = s6i_task::make_permission<const ResourceA*, ResourceB*>();
  EXPECT_TRUE(p2.test_read_permission<ResourceA>());
  EXPECT_TRUE(p2.test_read_permission<ResourceB>());
  EXPECT_TRUE(p2.test_write_permission<ResourceB>());
}

}  // namespace
