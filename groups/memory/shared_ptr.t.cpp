// Component being tested
#include <shared_ptr.h>

// Testing framework
#include <gtest/gtest.h>

namespace ksl {

class SharedPtrTest : public ::testing::Test {
  protected:
    struct TestObject {
        int value;
        static int constructor_count;
        static int destructor_count;

        TestObject(int v = 42) : value(v) { constructor_count++; }
        ~TestObject() { destructor_count++; }
    };
};

int SharedPtrTest::TestObject::constructor_count = 0;
int SharedPtrTest::TestObject::destructor_count = 0;

// ============================================================================
// CONSTRUCTION TESTS
// ============================================================================

TEST_F(SharedPtrTest, DefaultConstructor) {
    shared_ptr<int> ptr;
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST_F(SharedPtrTest, NullptrConstructor) {
    shared_ptr<int> ptr(nullptr);
    EXPECT_EQ(ptr.get(), nullptr);
}

TEST_F(SharedPtrTest, ConstructWithPointer) {
    int *raw = new int(42);
    shared_ptr<int> ptr(raw);
    EXPECT_EQ(*ptr, 42);
    EXPECT_EQ(ptr.get(), raw);
}

TEST_F(SharedPtrTest, ConstructWithObjectPointer) {
    TestObject::destructor_count = 0;
    TestObject *obj = new TestObject(100);
    shared_ptr<TestObject> ptr(obj);
    EXPECT_EQ(ptr->value, 100);
    EXPECT_EQ((*ptr).value, 100);
}

// ============================================================================
// COPY CONSTRUCTOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, CopyConstructorSharesOwnership) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        {
            shared_ptr<TestObject> ptr2(ptr1);
            EXPECT_EQ(ptr2->value, 42);
            EXPECT_EQ(TestObject::destructor_count, 0);
        }
        EXPECT_EQ(TestObject::destructor_count, 0);
        EXPECT_EQ(ptr1->value, 42);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyConstructorWithMultipleOwners) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(50));
        shared_ptr<TestObject> ptr2(ptr1);
        shared_ptr<TestObject> ptr3(ptr1);
        EXPECT_EQ(ptr3->value, 50);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyConstructorWithNullptr) {
    shared_ptr<int> ptr1(nullptr);
    shared_ptr<int> ptr2(ptr1);
    EXPECT_EQ(ptr2.get(), nullptr);
}

// ============================================================================
// COPY ASSIGNMENT TESTS
// ============================================================================

TEST_F(SharedPtrTest, CopyAssignmentSharesOwnership) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(10));
        shared_ptr<TestObject> ptr2(new TestObject(20));
        EXPECT_EQ(ptr1->value, 10);
        EXPECT_EQ(ptr2->value, 20);
        EXPECT_EQ(TestObject::destructor_count, 0);

        ptr1 = ptr2;
        EXPECT_EQ(ptr1->value, 20);
        EXPECT_EQ(ptr2->value, 20);
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
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyAssignmentFromNullptrToValid) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(nullptr);
        shared_ptr<TestObject> ptr2(new TestObject(42));
        ptr1 = ptr2;
        EXPECT_EQ(ptr1->value, 42);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyAssignmentFromValidToNullptr) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        shared_ptr<TestObject> ptr2(nullptr);
        ptr1 = ptr2;
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
}

TEST_F(SharedPtrTest, CopyAssignmentReturnsSelf) {
    shared_ptr<int> ptr1(new int(1));
    shared_ptr<int> ptr2(new int(2));
    shared_ptr<int> &result = (ptr1 = ptr2);
    EXPECT_EQ(&result, &ptr1);
}

// ============================================================================
// DEREFERENCE OPERATOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, DereferenceOperatorRead) {
    shared_ptr<int> ptr(new int(42));
    EXPECT_EQ(*ptr, 42);
}

TEST_F(SharedPtrTest, DereferenceOperatorWrite) {
    shared_ptr<int> ptr(new int(10));
    *ptr = 20;
    EXPECT_EQ(*ptr, 20);
}

TEST_F(SharedPtrTest, DereferenceOperatorWithObject) {
    shared_ptr<TestObject> ptr(new TestObject(77));
    EXPECT_EQ((*ptr).value, 77);
    (*ptr).value = 100;
    EXPECT_EQ(ptr->value, 100);
}

TEST_F(SharedPtrTest, ConstDereference) {
    const shared_ptr<int> ptr(new int(42));
    EXPECT_EQ(*ptr, 42);
}

// ============================================================================
// ARROW OPERATOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, ArrowOperatorAccess) {
    shared_ptr<TestObject> ptr(new TestObject(88));
    EXPECT_EQ(ptr->value, 88);
}

TEST_F(SharedPtrTest, ArrowOperatorWrite) {
    shared_ptr<TestObject> ptr(new TestObject(50));
    ptr->value = 150;
    EXPECT_EQ(ptr->value, 150);
}

TEST_F(SharedPtrTest, ConstArrow) {
    const shared_ptr<TestObject> ptr(new TestObject(42));
    EXPECT_EQ(ptr->value, 42);
}

// ============================================================================
// DESTRUCTOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, DestructorDeletesObjectWhenLastOwner) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(42));
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, DestructorWithNullptr) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(nullptr);
    }
    EXPECT_EQ(TestObject::destructor_count, 0);
}

TEST_F(SharedPtrTest, DestructorDoesNotDeleteWhenOtherOwnersExist) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr1;
    {
        shared_ptr<TestObject> ptr2(new TestObject(60));
        ptr1 = ptr2;
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 0);
    EXPECT_EQ(ptr1->value, 60);
}

// ============================================================================
// REFERENCE COUNTING AND CLEANUP TESTS
// ============================================================================

TEST_F(SharedPtrTest, ReferenceCountingMultipleOwners) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(111));
        {
            shared_ptr<TestObject> ptr2(ptr1);
            {
                shared_ptr<TestObject> ptr3(ptr1);
                EXPECT_EQ(TestObject::destructor_count, 0);
            }
            EXPECT_EQ(TestObject::destructor_count, 0);
        }
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, MultipleOwnersPointToSameObject) {
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
            EXPECT_EQ(TestObject::destructor_count, 0);
        }
        EXPECT_EQ(TestObject::destructor_count, 0);
        EXPECT_EQ(ptr1->value, 100);
        EXPECT_EQ(ptr2->value, 100);
        EXPECT_EQ(ptr3->value, 100);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, SwitchingBetweenDifferentObjects) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(new TestObject(1));
        EXPECT_EQ(ptr->value, 1);

        ptr = shared_ptr<TestObject>(new TestObject(2));
        EXPECT_EQ(ptr->value, 2);
        EXPECT_EQ(TestObject::destructor_count, 1);

        ptr = shared_ptr<TestObject>(new TestObject(3));
        EXPECT_EQ(ptr->value, 3);
        EXPECT_EQ(TestObject::destructor_count, 2);
    }
    EXPECT_EQ(TestObject::destructor_count, 3);
}

TEST_F(SharedPtrTest, RepeatedAssignmentsSameObject) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr(new TestObject(50));
    shared_ptr<TestObject> ptr2(nullptr);

    ptr2 = ptr;
    ptr2 = ptr;
    ptr2 = ptr;

    EXPECT_EQ(ptr2->value, 50);
    EXPECT_EQ(TestObject::destructor_count, 0);
}

// ============================================================================
// MOVE CONSTRUCTOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, MoveConstructorTransfersOwnership) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        shared_ptr<TestObject> ptr2(std::move(ptr1));
        EXPECT_EQ(ptr2->value, 42);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveConstructorWithTemporary) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr = shared_ptr<TestObject>(new TestObject(99));
        EXPECT_EQ(ptr->value, 99);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveConstructorFromNullptr) {
    shared_ptr<int> ptr1(nullptr);
    shared_ptr<int> ptr2(std::move(ptr1));
    EXPECT_EQ(ptr2.get(), nullptr);
    EXPECT_EQ(ptr1.get(), nullptr);
}

TEST_F(SharedPtrTest, MoveConstructorChain) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(100));
        shared_ptr<TestObject> ptr2(std::move(ptr1));
        shared_ptr<TestObject> ptr3(std::move(ptr2));

        EXPECT_EQ(ptr3->value, 100);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(ptr2.get(), nullptr);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// ============================================================================
// MOVE ASSIGNMENT TESTS
// ============================================================================

TEST_F(SharedPtrTest, MoveAssignmentTransfersOwnership) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(11));
        shared_ptr<TestObject> ptr2(new TestObject(22));

        ptr2 = std::move(ptr1);
        EXPECT_EQ(ptr2->value, 11);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, MoveAssignmentToNullptr) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(33));
        shared_ptr<TestObject> ptr2(nullptr);

        ptr2 = std::move(ptr1);
        EXPECT_EQ(ptr2->value, 33);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveAssignmentFromNullptr) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(44));
        shared_ptr<TestObject> ptr2(nullptr);

        ptr1 = std::move(ptr2);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
}

TEST_F(SharedPtrTest, MoveAssignmentToSelf) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(55));
        ptr1 = std::move(ptr1);
        EXPECT_EQ(ptr1->value, 55);
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

TEST_F(SharedPtrTest, MoveAssignmentWithTemporary) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr(nullptr);
        ptr = shared_ptr<TestObject>(new TestObject(88));
        EXPECT_EQ(ptr->value, 88);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveAssignmentReleasesOldObject) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(5));
        {
            shared_ptr<TestObject> ptr2(new TestObject(10));
            ptr1 = std::move(ptr2);
            EXPECT_EQ(ptr1->value, 10);
            EXPECT_EQ(TestObject::destructor_count, 1);
        }
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
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
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
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
