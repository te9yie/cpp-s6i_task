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

// パーミッションのテスト
TEST_F(TaskFuncTest, SingleArgumentPermission) {
  // 書き込み可能な関数のパーミッション
  {
    auto task = s6i_task::make_task_func(&m_allocator, test_func1);
    ASSERT_NE(task.get(), nullptr);

    auto permission = task->permission();
    EXPECT_TRUE(permission.test_read_permission<TestType1>());
    EXPECT_TRUE(permission.test_write_permission<TestType1>());
  }

  // 読み取り専用の関数のパーミッション
  {
    auto task = s6i_task::make_task_func(&m_allocator, test_func2);
    ASSERT_NE(task.get(), nullptr);

    auto permission = task->permission();
    EXPECT_TRUE(permission.test_read_permission<TestType1>());
    EXPECT_FALSE(permission.test_write_permission<TestType1>());
  }
}

TEST_F(TaskFuncTest, MultipleArgumentsPermission) {
  // 書き込み可能な複数引数の関数のパーミッション
  {
    auto task = s6i_task::make_task_func(&m_allocator, test_func3);
    ASSERT_NE(task.get(), nullptr);

    auto permission = task->permission();
    EXPECT_TRUE(permission.test_read_permission<TestType1>());
    EXPECT_TRUE(permission.test_write_permission<TestType1>());
    EXPECT_TRUE(permission.test_read_permission<TestType2>());
    EXPECT_TRUE(permission.test_write_permission<TestType2>());
  }

  // 読み取り専用の複数引数の関数のパーミッション
  {
    auto task = s6i_task::make_task_func(&m_allocator, test_func4);
    ASSERT_NE(task.get(), nullptr);

    auto permission = task->permission();
    EXPECT_TRUE(permission.test_read_permission<TestType1>());
    EXPECT_FALSE(permission.test_write_permission<TestType1>());
    EXPECT_TRUE(permission.test_read_permission<TestType2>());
    EXPECT_FALSE(permission.test_write_permission<TestType2>());
  }
}

TEST_F(TaskFuncTest, MixedPermissions) {
  // 読み取り専用と書き込み可能な引数が混在する関数
  auto task = s6i_task::make_task_func(
      &m_allocator, [](const TestType1* data1, TestType2* data2) {
        function_called = true;
        last_value = data1->value;
        data2->value = "modified";
      });
  ASSERT_NE(task.get(), nullptr);

  auto permission = task->permission();
  EXPECT_TRUE(permission.test_read_permission<TestType1>());
  EXPECT_FALSE(permission.test_write_permission<TestType1>());
  EXPECT_TRUE(permission.test_read_permission<TestType2>());
  EXPECT_TRUE(permission.test_write_permission<TestType2>());
}

TEST_F(TaskFuncTest, PermissionConflict) {
  // 2つのタスクのパーミッション競合をテスト
  auto task1 = s6i_task::make_task_func(&m_allocator,
                                        test_func1);  // TestType1の書き込み
  auto task2 = s6i_task::make_task_func(&m_allocator,
                                        test_func1);  // TestType1の書き込み
  ASSERT_NE(task1.get(), nullptr);
  ASSERT_NE(task2.get(), nullptr);

  // 同じリソースへの書き込みアクセスは競合する
  EXPECT_TRUE(s6i_task::is_conflict(task1->permission(), task2->permission()));

  // 読み取り専用アクセスは競合しない
  auto task3 = s6i_task::make_task_func(&m_allocator,
                                        test_func2);  // TestType1の読み取り
  auto task4 = s6i_task::make_task_func(&m_allocator,
                                        test_func2);  // TestType1の読み取り
  ASSERT_NE(task3.get(), nullptr);
  ASSERT_NE(task4.get(), nullptr);

  EXPECT_FALSE(s6i_task::is_conflict(task3->permission(), task4->permission()));

  // 読み取りと書き込みは競合する
  EXPECT_TRUE(s6i_task::is_conflict(task1->permission(), task3->permission()));
}

}  // namespace
