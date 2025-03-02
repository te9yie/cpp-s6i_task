#include <s6i_task/resources.h>
#include <t9_mem/prelude.h>

namespace {

struct TestType1 {
  int value;
  TestType1(int v) : value(v) {}
};

struct TestType2 {
  std::string value;
  TestType2(const std::string& v) : value(v) {}
};

struct TestType3 {
  int x;
  double y;
  TestType3(int x_val, double y_val) : x(x_val), y(y_val) {}
};

class ResourcesTest : public ::testing::Test {
 protected:
  t9_mem::DefaultAllocator m_allocator;
};

TEST_F(ResourcesTest, InitialState) {
  s6i_task::Resources resources(&m_allocator);

  // 初期状態では型に対応するポインタはnullptrであること
  EXPECT_EQ(resources.get_ptr<TestType1>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType2>(), nullptr);
}

TEST_F(ResourcesTest, SetAndGetPointer) {
  s6i_task::Resources resources(&m_allocator);

  // ポインタを設定
  auto* ptr1 = new TestType1(42);
  resources.set_ptr(ptr1);

  // 設定したポインタが取得できること
  EXPECT_EQ(resources.get_ptr<TestType1>(), ptr1);
  EXPECT_EQ(resources.get_ptr<TestType1>()->value, 42);

  // 別の型のポインタはnullptrのまま
  EXPECT_EQ(resources.get_ptr<TestType2>(), nullptr);

  // 別の型のポインタを設定
  auto* ptr2 = new TestType2("test");
  resources.set_ptr(ptr2);

  // 両方の型のポインタが正しく取得できること
  EXPECT_EQ(resources.get_ptr<TestType1>(), ptr1);
  EXPECT_EQ(resources.get_ptr<TestType1>()->value, 42);
  EXPECT_EQ(resources.get_ptr<TestType2>(), ptr2);
  EXPECT_EQ(resources.get_ptr<TestType2>()->value, "test");

  // リソースの後片付け
  delete ptr1;
  delete ptr2;
}

TEST_F(ResourcesTest, OverwritePointer) {
  s6i_task::Resources resources(&m_allocator);

  // 最初のポインタを設定
  auto* ptr1 = new TestType1(42);
  resources.set_ptr(ptr1);
  EXPECT_EQ(resources.get_ptr<TestType1>(), ptr1);

  // 同じ型の新しいポインタで上書き
  auto* ptr2 = new TestType1(100);
  resources.set_ptr(ptr2);
  EXPECT_EQ(resources.get_ptr<TestType1>(), ptr2);
  EXPECT_EQ(resources.get_ptr<TestType1>()->value, 100);

  // リソースの後片付け
  delete ptr1;
  delete ptr2;
}

TEST_F(ResourcesTest, Set) {
  s6i_task::Resources resources(&m_allocator);

  // set()メソッドを使用して値を設定
  auto* ptr1 = resources.set(TestType1(42));

  // 設定した値が取得できること
  EXPECT_NE(resources.get_ptr<TestType1>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType1>(), ptr1);
  EXPECT_EQ(resources.get_ptr<TestType1>()->value, 42);

  // 別の型の値を設定
  auto* ptr2 = resources.set(TestType2("test"));

  // 両方の型の値が正しく取得できること
  EXPECT_NE(resources.get_ptr<TestType1>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType1>()->value, 42);
  EXPECT_NE(resources.get_ptr<TestType2>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType2>(), ptr2);
  EXPECT_EQ(resources.get_ptr<TestType2>()->value, "test");

  // 同じ型の新しい値で上書き
  auto* ptr3 = resources.set(TestType1(100));

  // 新しい値が取得できること
  EXPECT_NE(resources.get_ptr<TestType1>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType1>(), ptr3);
  EXPECT_EQ(resources.get_ptr<TestType1>()->value, 100);
}

TEST_F(ResourcesTest, Emplace) {
  s6i_task::Resources resources(&m_allocator);

  // emplace()メソッドを使用して値を構築
  auto* ptr1 = resources.emplace<TestType1>(42);

  // 構築した値が取得できること
  EXPECT_NE(resources.get_ptr<TestType1>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType1>(), ptr1);
  EXPECT_EQ(resources.get_ptr<TestType1>()->value, 42);

  // 別の型の値を構築
  auto* ptr2 = resources.emplace<TestType2>("test");

  // 両方の型の値が正しく取得できること
  EXPECT_NE(resources.get_ptr<TestType1>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType1>()->value, 42);
  EXPECT_NE(resources.get_ptr<TestType2>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType2>(), ptr2);
  EXPECT_EQ(resources.get_ptr<TestType2>()->value, "test");

  // 複数の引数を取る型をemplaceで構築
  auto* ptr3 = resources.emplace<TestType3>(10, 3.14);

  // 構築した値が正しく取得できること
  EXPECT_NE(resources.get_ptr<TestType3>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType3>(), ptr3);
  EXPECT_EQ(resources.get_ptr<TestType3>()->x, 10);
  EXPECT_DOUBLE_EQ(resources.get_ptr<TestType3>()->y, 3.14);

  // 同じ型の新しい値で上書き
  auto* ptr4 = resources.emplace<TestType1>(100);

  // 新しい値が取得できること
  EXPECT_NE(resources.get_ptr<TestType1>(), nullptr);
  EXPECT_EQ(resources.get_ptr<TestType1>(), ptr4);
  EXPECT_EQ(resources.get_ptr<TestType1>()->value, 100);
}

}  // namespace
