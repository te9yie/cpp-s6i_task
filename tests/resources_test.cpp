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

}  // namespace
