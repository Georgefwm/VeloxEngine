#include <gtest/gtest.h>

#include "Arena.h"

TEST(VeloxTests, arena_construct_small)
{
    Velox::Arena arena(sizeof(int));
    SUCCEED();   
}

TEST(VeloxTests, arena_construct_large)
{
    Velox::Arena arena(sizeof(int) * 10000);
    SUCCEED();
}

TEST(VeloxTests, arena_construct_none)
{
    Velox::Arena arena(0);
    SUCCEED();
}

TEST(VeloxTests, arena_alloc_normal)
{
    Velox::Arena arena(sizeof(int));
    
    int* aPtr = arena.Alloc<int>(1);
    
    ASSERT_NE(aPtr, nullptr);
}

TEST(VeloxTests, arena_alloc_return_nullptr_when_capacity_exceeded)
{
    Velox::Arena arena(sizeof(int));
    
    int* ptrA = arena.Alloc<int>(1);
    int* ptrB = arena.Alloc<int>(1);
    
    ASSERT_EQ(ptrB, nullptr);
}

class CustomPrinter : public ::testing::TestEventListener {
public:
    explicit CustomPrinter(::testing::TestEventListener* wrapped)
        : wrapped_(wrapped) {}

    void OnTestStart(const ::testing::TestInfo&) override {
        // Suppress this output
    }

    // Forward everything else to wrapped listener
    void OnTestProgramStart(const ::testing::UnitTest& u) override { wrapped_->OnTestProgramStart(u); }
    void OnTestIterationStart(const ::testing::UnitTest& u, int i) override { wrapped_->OnTestIterationStart(u, i); }
    void OnEnvironmentsSetUpStart(const ::testing::UnitTest& u) override { wrapped_->OnEnvironmentsSetUpStart(u); }
    void OnEnvironmentsSetUpEnd(const ::testing::UnitTest& u) override { wrapped_->OnEnvironmentsSetUpEnd(u); }
    void OnTestSuiteStart(const ::testing::TestSuite& ts) override { wrapped_->OnTestSuiteStart(ts); }
    void OnTestCaseStart(const ::testing::TestCase& tc) override { wrapped_->OnTestCaseStart(tc); }  // for older gtest
    void OnTestPartResult(const ::testing::TestPartResult& tp) override { wrapped_->OnTestPartResult(tp); }
    void OnTestEnd(const ::testing::TestInfo& ti) override { wrapped_->OnTestEnd(ti); }
    void OnTestSuiteEnd(const ::testing::TestSuite& ts) override { wrapped_->OnTestSuiteEnd(ts); }
    void OnTestCaseEnd(const ::testing::TestCase& tc) override { wrapped_->OnTestCaseEnd(tc); }  // for older gtest
    void OnEnvironmentsTearDownStart(const ::testing::UnitTest& u) override { wrapped_->OnEnvironmentsTearDownStart(u); }
    void OnEnvironmentsTearDownEnd(const ::testing::UnitTest& u) override { wrapped_->OnEnvironmentsTearDownEnd(u); }
    void OnTestIterationEnd(const ::testing::UnitTest& u, int i) override { wrapped_->OnTestIterationEnd(u, i); }
    void OnTestProgramEnd(const ::testing::UnitTest& u) override { wrapped_->OnTestProgramEnd(u); }

private:
    ::testing::TestEventListener* wrapped_;
};

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);

    testing::TestEventListeners& listeners = testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new CustomPrinter(listeners.Release(listeners.default_result_printer())));

    return RUN_ALL_TESTS();
}
