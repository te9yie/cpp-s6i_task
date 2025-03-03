#include <t9_mem/prelude.h>

namespace {

// テスト用の型
struct TestType1 {
  int value;
  TestType1(int v) : value(v) {}
};

struct TestType2 {
  std::string value;
  TestType2(const std::string& v) : value(v) {}
};

// 関数呼び出しを追跡するための変数
static bool function_called = false;
static int last_value = 0;
static std::string last_string = "";

// テスト前に追跡用の変数をクリアする
void ClearTrackingData() {
  function_called = false;
  last_value = 0;
  last_string = "";
}

// テスト用の関数
void test_func1(TestType1* data) {
  function_called = true;
  last_value = data->value;
}

void test_func2(const TestType1* data) {
  function_called = true;
  last_value = data->value;
}

void test_func3(TestType1* data1, TestType2* data2) {
  function_called = true;
  last_value = data1->value;
  last_string = data2->value;
}

void test_func4(const TestType1* data1, const TestType2* data2) {
  function_called = true;
  last_value = data1->value;
  last_string = data2->value;
}

class TaskFuncTest : public ::testing::Test {
 protected:
  t9_mem::DefaultAllocator m_allocator;

  void SetUp() override {
    ClearTrackingData();
  }
};

// 単一引数の関数テスト（書き込み可能）
TEST_F(TaskFuncTest, SingleArgument) {
  // リソースの準備
  s6i_task::Resources resources(&m_allocator);
  auto* data = resources.emplace<TestType1>(42);
  ASSERT_NE(data, nullptr);

  // TaskFuncの作成
  auto task = s6i_task::make_task_func(&m_allocator, test_func1);
  ASSERT_NE(task.get(), nullptr);

  // 実行
  task->exec(&resources);

  // 関数が呼ばれたことを確認
  EXPECT_TRUE(function_called);
  EXPECT_EQ(last_value, 42);
}

// 単一引数の関数テスト（読み取り専用）
TEST_F(TaskFuncTest, SingleArgumentConst) {
  // リソースの準備
  s6i_task::Resources resources(&m_allocator);
  auto* data = resources.emplace<TestType1>(42);
  ASSERT_NE(data, nullptr);

  // TaskFuncの作成
  auto task = s6i_task::make_task_func(&m_allocator, test_func2);
  ASSERT_NE(task.get(), nullptr);

  // 実行
  task->exec(&resources);

  // 関数が呼ばれたことを確認
  EXPECT_TRUE(function_called);
  EXPECT_EQ(last_value, 42);
}

// 複数引数の関数テスト（書き込み可能）
TEST_F(TaskFuncTest, MultipleArguments) {
  // リソースの準備
  s6i_task::Resources resources(&m_allocator);
  auto* data1 = resources.emplace<TestType1>(42);
  auto* data2 = resources.emplace<TestType2>("hello");
  ASSERT_NE(data1, nullptr);
  ASSERT_NE(data2, nullptr);

  // TaskFuncの作成
  auto task = s6i_task::make_task_func(&m_allocator, test_func3);
  ASSERT_NE(task.get(), nullptr);

  // 実行
  task->exec(&resources);

  // 関数が呼ばれたことを確認
  EXPECT_TRUE(function_called);
  EXPECT_EQ(last_value, 42);
  EXPECT_EQ(last_string, "hello");
}

// 複数引数の関数テスト（読み取り専用）
TEST_F(TaskFuncTest, MultipleArgumentsConst) {
  // リソースの準備
  s6i_task::Resources resources(&m_allocator);
  auto* data1 = resources.emplace<TestType1>(42);
  auto* data2 = resources.emplace<TestType2>("hello");
  ASSERT_NE(data1, nullptr);
  ASSERT_NE(data2, nullptr);

  // TaskFuncの作成
  auto task = s6i_task::make_task_func(&m_allocator, test_func4);
  ASSERT_NE(task.get(), nullptr);

  // 実行
  task->exec(&resources);

  // 関数が呼ばれたことを確認
  EXPECT_TRUE(function_called);
  EXPECT_EQ(last_value, 42);
  EXPECT_EQ(last_string, "hello");
}

// ラムダ式を使用したテスト
TEST_F(TaskFuncTest, LambdaFunction) {
  // リソースの準備
  s6i_task::Resources resources(&m_allocator);
  auto* data = resources.emplace<TestType1>(42);
  ASSERT_NE(data, nullptr);

  // ラムダ式を使用したTaskFuncの作成
  auto task = s6i_task::make_task_func(&m_allocator, [](TestType1* data) {
    function_called = true;
    last_value = data->value;
  });
  ASSERT_NE(task.get(), nullptr);

  // 実行
  task->exec(&resources);

  // 関数が呼ばれたことを確認
  EXPECT_TRUE(function_called);
  EXPECT_EQ(last_value, 42);
}

// リソースが存在しない場合のテスト
TEST_F(TaskFuncTest, MissingResource) {
  // リソースの準備（TestType1は設定しない）
  s6i_task::Resources resources(&m_allocator);

  // TaskFuncの作成
  auto task = s6i_task::make_task_func(&m_allocator, test_func1);
  ASSERT_NE(task.get(), nullptr);

  // 実行（アサーションが発生するはずなので、DEATHテストを使用）
  EXPECT_DEATH(task->exec(&resources), "");
}

}  // namespace
