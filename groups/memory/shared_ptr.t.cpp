// Component being tested
#include <shared_ptr.h>

// Testing framework
#include <gtest/gtest.h>

namespace ksl {

class SharedPtrTest : public ::testing::Test {
  protected:
    struct TestObject {
        int value;
        static int destructor_count;

        explicit TestObject(int v = 42) : value(v) {}
        ~TestObject() { destructor_count++; }
    };
};

int SharedPtrTest::TestObject::destructor_count = 0;

// ============================================================================
// DEFAULT AND NULLPTR CONSTRUCTOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, DefaultConstructor) {
    shared_ptr<int> ptr;
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(ptr.use_count(), 0);
}

TEST_F(SharedPtrTest, NullptrConstructor) {
    shared_ptr<int> ptr(nullptr);
    EXPECT_EQ(ptr.get(), nullptr);
    EXPECT_EQ(ptr.use_count(), 0);
}

// ============================================================================
// RAW POINTER CONSTRUCTOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, ConstructWithPointer) {
    int *raw = new int(42);
    shared_ptr<int> ptr(raw);
    EXPECT_EQ(*ptr, 42);
    EXPECT_EQ(ptr.get(), raw);
    EXPECT_EQ(ptr.use_count(), 1);
}

TEST_F(SharedPtrTest, ConstructWithObjectPointer) {
    TestObject::destructor_count = 0;
    {
        TestObject *obj = new TestObject(100);
        shared_ptr<TestObject> ptr(obj);
        EXPECT_EQ(ptr->value, 100);
        EXPECT_EQ(ptr.use_count(), 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// ============================================================================
// COPY CONSTRUCTOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, CopyConstructorBasic) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        shared_ptr<TestObject> ptr2(ptr1);
        EXPECT_EQ(ptr2->value, 42);
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(ptr2.use_count(), 2);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyConstructorMultipleOwners) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(50));
        shared_ptr<TestObject> ptr2(ptr1);
        shared_ptr<TestObject> ptr3(ptr1);
        EXPECT_EQ(ptr1.use_count(), 3);
        EXPECT_EQ(ptr3->value, 50);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyConstructorFromNullptr) {
    shared_ptr<TestObject> ptr1(nullptr);
    shared_ptr<TestObject> ptr2(ptr1);
    EXPECT_EQ(ptr2.get(), nullptr);
    EXPECT_EQ(ptr2.use_count(), 0);
}

// ============================================================================
// MOVE CONSTRUCTOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, MoveConstructorBasic) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        shared_ptr<TestObject> ptr2(std::move(ptr1));
        EXPECT_EQ(ptr2->value, 42);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(ptr1.use_count(), 0);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveConstructorChain) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(100));
        shared_ptr<TestObject> ptr2(std::move(ptr1));
        shared_ptr<TestObject> ptr3(std::move(ptr2));

        EXPECT_EQ(ptr3->value, 100);
        EXPECT_EQ(ptr3.use_count(), 1);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(ptr2.get(), nullptr);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveConstructorWithTemporary) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr = shared_ptr<TestObject>(new TestObject(99));
        EXPECT_EQ(ptr->value, 99);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// ============================================================================
// COPY ASSIGNMENT TESTS
// ============================================================================

TEST_F(SharedPtrTest, CopyAssignmentBasic) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(10));
        shared_ptr<TestObject> ptr2(new TestObject(20));
        ptr1 = ptr2;
        EXPECT_EQ(ptr1->value, 20);
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(ptr2.use_count(), 2);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, CopyAssignmentToSelf) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(42));
        ptr = ptr;
        EXPECT_EQ(ptr->value, 42);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyAssignmentReturnsSelf) {
    shared_ptr<int> ptr1(new int(1));
    shared_ptr<int> ptr2(new int(2));
    shared_ptr<int> &result = (ptr1 = ptr2);
    EXPECT_EQ(&result, &ptr1);
}

TEST_F(SharedPtrTest, CopyAssignmentFromNull) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(5));
        shared_ptr<TestObject> null_ptr(nullptr);
        ptr = null_ptr;
        EXPECT_EQ(ptr.get(), nullptr);
        EXPECT_EQ(ptr.use_count(), 0);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// ============================================================================
// MOVE ASSIGNMENT TESTS
// ============================================================================

TEST_F(SharedPtrTest, MoveAssignmentBasic) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(11));
        shared_ptr<TestObject> ptr2(new TestObject(22));
        ptr2 = std::move(ptr1);
        EXPECT_EQ(ptr2->value, 11);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(ptr1.use_count(), 0);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, MoveAssignmentToSelf) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(55));
        ptr = std::move(ptr);
        EXPECT_EQ(ptr->value, 55);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveAssignmentReturnsReference) {
    shared_ptr<int> ptr1(new int(10));
    shared_ptr<int> ptr2(new int(20));
    shared_ptr<int> &result = (ptr2 = std::move(ptr1));
    EXPECT_EQ(&result, &ptr2);
    EXPECT_EQ(*result, 10);
}

TEST_F(SharedPtrTest, MoveAssignmentWithSharedOwnership) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(50));
        shared_ptr<TestObject> ptr2(ptr1);
        shared_ptr<TestObject> ptr3(new TestObject(60));

        ptr3 = std::move(ptr1);

        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(ptr2->value, 50);
        EXPECT_EQ(ptr3->value, 50);
        EXPECT_EQ(ptr2.use_count(), 2);
        EXPECT_EQ(ptr3.use_count(), 2);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

// ============================================================================
// ACCESSOR TESTS (get, operator*, operator->)
// ============================================================================

TEST_F(SharedPtrTest, GetAccessor) {
    int *raw = new int(42);
    shared_ptr<int> ptr(raw);
    EXPECT_EQ(ptr.get(), raw);
}

TEST_F(SharedPtrTest, GetAccessorOnNullptr) {
    shared_ptr<int> ptr;
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST_F(SharedPtrTest, DereferenceOperator) {
    shared_ptr<int> ptr(new int(42));
    EXPECT_EQ(*ptr, 42);
    *ptr = 100;
    EXPECT_EQ(*ptr, 100);
}

TEST_F(SharedPtrTest, DereferenceOperatorWithObject) {
    shared_ptr<TestObject> ptr(new TestObject(77));
    EXPECT_EQ((*ptr).value, 77);
    (*ptr).value = 100;
    EXPECT_EQ((*ptr).value, 100);
}

TEST_F(SharedPtrTest, ArrowOperator) {
    shared_ptr<TestObject> ptr(new TestObject(88));
    EXPECT_EQ(ptr->value, 88);
    ptr->value = 150;
    EXPECT_EQ(ptr->value, 150);
}

// ============================================================================
// REFERENCE COUNTING TESTS
// ============================================================================

TEST_F(SharedPtrTest, UseCountBasic) {
    shared_ptr<TestObject> ptr(new TestObject(42));
    EXPECT_EQ(ptr.use_count(), 1);
}

TEST_F(SharedPtrTest, UseCountNull) {
    shared_ptr<TestObject> ptr;
    EXPECT_EQ(ptr.use_count(), 0);
}

TEST_F(SharedPtrTest, UseCountMultiple) {
    shared_ptr<TestObject> ptr1(new TestObject(42));
    shared_ptr<TestObject> ptr2(ptr1);
    shared_ptr<TestObject> ptr3(ptr1);
    EXPECT_EQ(ptr1.use_count(), 3);
    EXPECT_EQ(ptr2.use_count(), 3);
    EXPECT_EQ(ptr3.use_count(), 3);
}

TEST_F(SharedPtrTest, UseCountDecrement) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr1(new TestObject(42));
    {
        shared_ptr<TestObject> ptr2(ptr1);
        EXPECT_EQ(ptr1.use_count(), 2);
    }
    EXPECT_EQ(ptr1.use_count(), 1);
    EXPECT_EQ(TestObject::destructor_count, 0);
}

// ============================================================================
// DESTRUCTION AND CLEANUP TESTS
// ============================================================================

TEST_F(SharedPtrTest, DestructorDeletesWhenLastOwner) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(42));
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, DestructorPreservesWhenOtherOwnersExist) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr1;
    {
        shared_ptr<TestObject> ptr2(new TestObject(60));
        ptr1 = ptr2;
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 0);
    EXPECT_EQ(ptr1->value, 60);
    EXPECT_EQ(ptr1.use_count(), 1);
}

TEST_F(SharedPtrTest, DestructorAfterReleasing) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(5));
        shared_ptr<TestObject> ptr2(ptr1);
        ptr1 = shared_ptr<TestObject>(new TestObject(15));
        EXPECT_EQ(ptr1->value, 15);
        EXPECT_EQ(ptr2->value, 5);
        EXPECT_EQ(ptr1.use_count(), 1);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

// ============================================================================
// RESET METHOD TESTS
// ============================================================================

TEST_F(SharedPtrTest, ResetWithoutArguments) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(42));
        EXPECT_EQ(ptr.use_count(), 1);
        ptr.reset();
        EXPECT_EQ(ptr.get(), nullptr);
        EXPECT_EQ(ptr.use_count(), 0);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
}

TEST_F(SharedPtrTest, ResetWithoutArgumentsOnSharedOwnership) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(50));
        shared_ptr<TestObject> ptr2(ptr1);
        EXPECT_EQ(ptr1.use_count(), 2);
        ptr1.reset();
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(ptr1.use_count(), 0);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(ptr2->value, 50);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, ResetWithNewPointer) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(10));
        ptr.reset(new TestObject(20));
        EXPECT_EQ(ptr->value, 20);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, ResetWithNewPointerOnSharedOwnership) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(5));
        shared_ptr<TestObject> ptr2(ptr1);
        EXPECT_EQ(ptr1.use_count(), 2);
        ptr1.reset(new TestObject(15));
        EXPECT_EQ(ptr1->value, 15);
        EXPECT_EQ(ptr1.use_count(), 1);
        EXPECT_EQ(ptr2->value, 5);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, ResetWithNullptr) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(42));
        ptr.reset(nullptr);
        EXPECT_EQ(ptr.get(), nullptr);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, ResetNullptrPtr) {
    {
        shared_ptr<TestObject> ptr;
        ptr.reset();
        EXPECT_EQ(ptr.get(), nullptr);
        EXPECT_EQ(ptr.use_count(), 0);
    }
}

// ============================================================================
// OPERATOR BOOL TESTS
// ============================================================================

TEST_F(SharedPtrTest, OperatorBoolWithValidPointer) {
    shared_ptr<TestObject> ptr(new TestObject(42));
    EXPECT_TRUE(ptr);
    EXPECT_TRUE(static_cast<bool>(ptr));
}

TEST_F(SharedPtrTest, OperatorBoolWithNullptr) {
    shared_ptr<TestObject> ptr;
    EXPECT_FALSE(ptr);
    EXPECT_FALSE(static_cast<bool>(ptr));
}

TEST_F(SharedPtrTest, OperatorBoolAfterReset) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(42));
        EXPECT_TRUE(ptr);
        ptr.reset();
        EXPECT_FALSE(ptr);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
}

TEST_F(SharedPtrTest, OperatorBoolAfterMove) {
    shared_ptr<TestObject> ptr1(new TestObject(42));
    shared_ptr<TestObject> ptr2(std::move(ptr1));
    EXPECT_FALSE(ptr1);
    EXPECT_TRUE(ptr2);
}

TEST_F(SharedPtrTest, OperatorBoolWithSharedOwnership) {
    shared_ptr<TestObject> ptr1(new TestObject(42));
    shared_ptr<TestObject> ptr2(ptr1);
    EXPECT_TRUE(ptr1);
    EXPECT_TRUE(ptr2);
    ptr1.reset();
    EXPECT_FALSE(ptr1);
    EXPECT_TRUE(ptr2);
}

// ============================================================================
// COMPLEX OWNERSHIP SCENARIOS
// ============================================================================

TEST_F(SharedPtrTest, ComplexOwnershipChain) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1;
        shared_ptr<TestObject> ptr2;
        shared_ptr<TestObject> ptr3;
        {
            shared_ptr<TestObject> temp(new TestObject(100));
            ptr1 = temp;
            ptr2 = temp;
            ptr3 = temp;
            EXPECT_EQ(ptr1.use_count(), 4);
            EXPECT_EQ(TestObject::destructor_count, 0);
        }
        EXPECT_EQ(ptr1.use_count(), 3);
        EXPECT_EQ(TestObject::destructor_count, 0);
        EXPECT_EQ(ptr1->value, 100);
        EXPECT_EQ(ptr2->value, 100);
        EXPECT_EQ(ptr3->value, 100);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// ============================================================================
// VALUE TYPE ALIAS TEST
// ============================================================================

TEST_F(SharedPtrTest, ValueTypeAlias) {
    static_assert(std::is_same_v<shared_ptr<int>::Value_Type, int>,
                  "Value_Type should be int for shared_ptr<int>");
    static_assert(std::is_same_v<shared_ptr<TestObject>::Value_Type, TestObject>,
                  "Value_Type should be TestObject for shared_ptr<TestObject>");
}

} // namespace ksl
