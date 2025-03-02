#include <s6i_task/resources.h>
#include <t9_mem/prelude.h>

namespace {

// 構築と破棄の順序を追跡するための静的変数
static std::vector<int> construction_order;
static std::vector<int> destruction_order;

// テスト前に追跡用の変数をクリアする
void ClearTrackingData() {
  construction_order.clear();
  destruction_order.clear();
}

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

// 構築と破棄を追跡するための型
struct TrackableType {
  int id;

  TrackableType(int id_val) : id(id_val) {
    construction_order.push_back(id);
  }

  ~TrackableType() {
    destruction_order.push_back(id);
  }
};

class ResourcesTest : public ::testing::Test {
 protected:
  t9_mem::DefaultAllocator m_allocator;

  void SetUp() override {
    ClearTrackingData();
  }
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

TEST_F(ResourcesTest, DestructionOrder) {
  ClearTrackingData();

  // スコープを作成して、Resourcesのデストラクタが呼ばれるようにする
  {
    s6i_task::Resources resources(&m_allocator);

    // 順番にオブジェクトを構築
    resources.emplace<TrackableType>(1);
    resources.emplace<TrackableType>(2);
    resources.emplace<TrackableType>(3);
    resources.emplace<TrackableType>(4);
    resources.emplace<TrackableType>(5);

    // 構築順序を確認
    ASSERT_EQ(construction_order.size(), 5);
    EXPECT_EQ(construction_order[0], 1);
    EXPECT_EQ(construction_order[1], 2);
    EXPECT_EQ(construction_order[2], 3);
    EXPECT_EQ(construction_order[3], 4);
    EXPECT_EQ(construction_order[4], 5);

    // この時点では破棄されていないことを確認
    EXPECT_TRUE(destruction_order.empty());
  }

  // Resourcesのデストラクタが呼ばれた後、オブジェクトが逆順に破棄されたことを確認
  ASSERT_EQ(destruction_order.size(), 5);
  EXPECT_EQ(destruction_order[0], 5);  // 最後に追加したものが最初に破棄される
  EXPECT_EQ(destruction_order[1], 4);
  EXPECT_EQ(destruction_order[2], 3);
  EXPECT_EQ(destruction_order[3], 2);
  EXPECT_EQ(destruction_order[4], 1);  // 最初に追加したものが最後に破棄される
}

TEST_F(ResourcesTest, MoveConstructor) {
  // 元のResourcesオブジェクトを作成
  s6i_task::Resources original(&m_allocator);

  // リソースを追加
  auto* ptr1 = original.emplace<TestType1>(42);
  auto* ptr2 = original.emplace<TestType2>("test");

  // ムーブコンストラクタでコピー
  s6i_task::Resources moved(std::move(original));

  // 移動先のオブジェクトからリソースが取得できること
  EXPECT_NE(moved.get_ptr<TestType1>(), nullptr);
  EXPECT_EQ(moved.get_ptr<TestType1>(), ptr1);
  EXPECT_EQ(moved.get_ptr<TestType1>()->value, 42);

  EXPECT_NE(moved.get_ptr<TestType2>(), nullptr);
  EXPECT_EQ(moved.get_ptr<TestType2>(), ptr2);
  EXPECT_EQ(moved.get_ptr<TestType2>()->value, "test");

  // 移動元のオブジェクトからはリソースが取得できないこと
  // 注意: ムーブ後の状態は未定義なので、このテストは実装依存
  // EXPECT_EQ(original.get_ptr<TestType1>(), nullptr);
  // EXPECT_EQ(original.get_ptr<TestType2>(), nullptr);
}

TEST_F(ResourcesTest, MoveAssignment) {
  // 元のResourcesオブジェクトを作成
  s6i_task::Resources original(&m_allocator);

  // リソースを追加
  auto* ptr1 = original.emplace<TestType1>(42);
  auto* ptr2 = original.emplace<TestType2>("test");

  // 移動先のオブジェクトを作成し、別のリソースを追加
  s6i_task::Resources target(&m_allocator);
  target.emplace<TestType3>(10, 3.14);

  // ムーブ代入
  target = std::move(original);

  // 移動先のオブジェクトから元のリソースが取得できること
  EXPECT_NE(target.get_ptr<TestType1>(), nullptr);
  EXPECT_EQ(target.get_ptr<TestType1>(), ptr1);
  EXPECT_EQ(target.get_ptr<TestType1>()->value, 42);

  EXPECT_NE(target.get_ptr<TestType2>(), nullptr);
  EXPECT_EQ(target.get_ptr<TestType2>(), ptr2);
  EXPECT_EQ(target.get_ptr<TestType2>()->value, "test");

  // 移動先の元のリソースは破棄されているはず
  EXPECT_EQ(target.get_ptr<TestType3>(), nullptr);

  // 移動元のオブジェクトからはリソースが取得できないこと
  // 注意: ムーブ後の状態は未定義なので、このテストは実装依存
  // EXPECT_EQ(original.get_ptr<TestType1>(), nullptr);
  // EXPECT_EQ(original.get_ptr<TestType2>(), nullptr);
}

TEST_F(ResourcesTest, MoveConstructorNoDestruction) {
  ClearTrackingData();

  // スコープを作成
  {
    // 元のResourcesオブジェクトを作成
    s6i_task::Resources original(&m_allocator);

    // TrackableTypeのオブジェクトを追加
    original.emplace<TrackableType>(1);
    original.emplace<TrackableType>(2);
    original.emplace<TrackableType>(3);

    // 構築順序を確認
    ASSERT_EQ(construction_order.size(), 3);
    EXPECT_EQ(construction_order[0], 1);
    EXPECT_EQ(construction_order[1], 2);
    EXPECT_EQ(construction_order[2], 3);

    // この時点では破棄されていないことを確認
    EXPECT_TRUE(destruction_order.empty());

    // スコープを作成して、ムーブ後に元のオブジェクトが破棄されるようにする
    {
      // ムーブコンストラクタでコピー
      s6i_task::Resources moved(std::move(original));

      // この時点でもオブジェクトは破棄されていないことを確認
      EXPECT_TRUE(destruction_order.empty());
    }

    // 移動先のオブジェクトが破棄された後、オブジェクトが逆順に破棄されたことを確認
    ASSERT_EQ(destruction_order.size(), 3);
    EXPECT_EQ(destruction_order[0], 3);  // 最後に追加したものが最初に破棄される
    EXPECT_EQ(destruction_order[1], 2);
    EXPECT_EQ(destruction_order[2], 1);  // 最初に追加したものが最後に破棄される

    // 元のオブジェクトのデストラクタが呼ばれても、オブジェクトが二重に破棄されないことを確認
    ClearTrackingData();  // 破棄記録をクリア
  }

  // 元のオブジェクトのデストラクタが呼ばれても、新たな破棄は発生しないことを確認
  EXPECT_TRUE(destruction_order.empty());
}

TEST_F(ResourcesTest, MoveAssignmentNoDestruction) {
  ClearTrackingData();

  // スコープを作成
  {
    // 元のResourcesオブジェクトを作成
    s6i_task::Resources original(&m_allocator);

    // TrackableTypeのオブジェクトを追加
    original.emplace<TrackableType>(1);
    original.emplace<TrackableType>(2);
    original.emplace<TrackableType>(3);

    // 移動先のオブジェクトを作成
    s6i_task::Resources target(&m_allocator);

    // 別のTrackableTypeのオブジェクトを追加
    target.emplace<TrackableType>(4);
    target.emplace<TrackableType>(5);

    // 構築順序を確認
    ASSERT_EQ(construction_order.size(), 5);
    EXPECT_EQ(construction_order[0], 1);
    EXPECT_EQ(construction_order[1], 2);
    EXPECT_EQ(construction_order[2], 3);
    EXPECT_EQ(construction_order[3], 4);
    EXPECT_EQ(construction_order[4], 5);

    // この時点では破棄されていないことを確認
    EXPECT_TRUE(destruction_order.empty());

    // ムーブ代入
    target = std::move(original);

    // 移動先の元のオブジェクトが破棄されたことを確認
    ASSERT_EQ(destruction_order.size(), 2);
    EXPECT_EQ(destruction_order[0],
              5);  // 移動先の最後のオブジェクトが最初に破棄される
    EXPECT_EQ(destruction_order[1],
              4);  // 移動先の最初のオブジェクトが次に破棄される

    ClearTrackingData();  // 破棄記録をクリア

    // この時点ではまだオブジェクトは破棄されていない
    EXPECT_TRUE(destruction_order.empty());
  }

  // 移動先のオブジェクトが破棄された後、元のオブジェクトが逆順に破棄されたことを確認
  ASSERT_EQ(destruction_order.size(), 3);
  EXPECT_EQ(destruction_order[0], 3);  // 最後に追加したものが最初に破棄される
  EXPECT_EQ(destruction_order[1], 2);
  EXPECT_EQ(destruction_order[2], 1);  // 最初に追加したものが最後に破棄される

  // 元のオブジェクトのデストラクタが呼ばれても、オブジェクトが二重に破棄されないことを確認
  ClearTrackingData();  // 破棄記録をクリア
}

}  // namespace
