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
// CONSTRUCTORS: DEFAULT, NULLPTR, AND RAW POINTER
// ============================================================================

TEST_F(SharedPtrTest, DefaultAndNullptrConstructor) {
    shared_ptr<int> ptr1;
    shared_ptr<int> ptr2(nullptr);
    EXPECT_EQ(ptr1.get(), nullptr);
    EXPECT_EQ(ptr2.get(), nullptr);
    EXPECT_EQ(ptr1.use_count(), 0);
    EXPECT_EQ(ptr2.use_count(), 0);
    EXPECT_FALSE(ptr1);
    EXPECT_FALSE(ptr2);
}

TEST_F(SharedPtrTest, ConstructWithRawPointer) {
    TestObject::destructor_count = 0;
    {
        int *raw_int = new int(42);
        TestObject *raw_obj = new TestObject(100);
        shared_ptr<int> ptr1(raw_int);
        shared_ptr<TestObject> ptr2(raw_obj);
        EXPECT_EQ(*ptr1, 42);
        EXPECT_EQ(ptr2->value, 100);
        EXPECT_EQ(ptr1.use_count(), 1);
        EXPECT_EQ(ptr2.use_count(), 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// ============================================================================
// COPY AND MOVE CONSTRUCTORS
// ============================================================================

TEST_F(SharedPtrTest, CopyConstructor) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        shared_ptr<TestObject> ptr2(ptr1);
        shared_ptr<TestObject> ptr3(ptr1);

        EXPECT_EQ(ptr1.get(), ptr2.get());
        EXPECT_EQ(ptr1.get(), ptr3.get());
        EXPECT_EQ(ptr1.use_count(), 3);
        EXPECT_EQ(ptr2.use_count(), 3);
        EXPECT_EQ(ptr3.use_count(), 3);
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

TEST_F(SharedPtrTest, MoveConstructor) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        shared_ptr<TestObject> ptr2(std::move(ptr1));

        EXPECT_EQ(ptr2->value, 42);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(ptr1.use_count(), 0);
        EXPECT_FALSE(ptr1);
        EXPECT_TRUE(ptr2);
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
        EXPECT_FALSE(ptr1);
        EXPECT_FALSE(ptr2);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// ============================================================================
// COPY AND MOVE ASSIGNMENT
// ============================================================================

TEST_F(SharedPtrTest, CopyAssignment) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(10));
        shared_ptr<TestObject> ptr2(new TestObject(20));

        // Basic copy assignment
        ptr1 = ptr2;
        EXPECT_EQ(ptr1->value, 20);
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(ptr2.use_count(), 2);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, CopyAssignmentSelfAssignment) {
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

TEST_F(SharedPtrTest, CopyAssignmentReturnsReference) {
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
        EXPECT_FALSE(ptr);
        EXPECT_EQ(ptr.use_count(), 0);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
}

TEST_F(SharedPtrTest, MoveAssignment) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(11));
        shared_ptr<TestObject> ptr2(new TestObject(22));

        ptr2 = std::move(ptr1);
        EXPECT_EQ(ptr2->value, 11);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_FALSE(ptr1);
        EXPECT_EQ(ptr1.use_count(), 0);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, MoveAssignmentSelfAssignment) {
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

TEST_F(SharedPtrTest, MoveAssignmentWithSharedOwnership) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(50));
        shared_ptr<TestObject> ptr2(ptr1);
        shared_ptr<TestObject> ptr3(new TestObject(60));

        ptr3 = std::move(ptr1);

        EXPECT_FALSE(ptr1);
        EXPECT_EQ(ptr2->value, 50);
        EXPECT_EQ(ptr3->value, 50);
        EXPECT_EQ(ptr2.use_count(), 2);
        EXPECT_EQ(ptr3.use_count(), 2);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

// ============================================================================
// ACCESSORS AND OPERATORS
// ============================================================================

TEST_F(SharedPtrTest, GetOperator) {
    int *raw = new int(42);
    shared_ptr<int> ptr1(raw);
    shared_ptr<int> ptr2;

    EXPECT_EQ(ptr1.get(), raw);
    EXPECT_EQ(ptr2.get(), nullptr);
}

TEST_F(SharedPtrTest, DereferenceAndArrowOperators) {
    shared_ptr<int> ptr_int(new int(42));
    EXPECT_EQ(*ptr_int, 42);
    *ptr_int = 100;
    EXPECT_EQ(*ptr_int, 100);

    shared_ptr<TestObject> ptr_obj(new TestObject(77));
    EXPECT_EQ((*ptr_obj).value, 77);
    EXPECT_EQ(ptr_obj->value, 77);
    ptr_obj->value = 150;
    EXPECT_EQ(ptr_obj->value, 150);
}

TEST_F(SharedPtrTest, OperatorBool) {
    shared_ptr<TestObject> ptr1(new TestObject(42));
    shared_ptr<TestObject> ptr2;
    shared_ptr<TestObject> ptr3(nullptr);

    EXPECT_TRUE(ptr1);
    EXPECT_FALSE(ptr2);
    EXPECT_FALSE(ptr3);

    ptr1.reset();
    EXPECT_FALSE(ptr1);
}

// ============================================================================
// REFERENCE COUNTING AND USE COUNT
// ============================================================================

TEST_F(SharedPtrTest, UseCount) {
    shared_ptr<TestObject> ptr1(new TestObject(42));
    EXPECT_EQ(ptr1.use_count(), 1);

    {
        shared_ptr<TestObject> ptr2(ptr1);
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(ptr2.use_count(), 2);

        {
            shared_ptr<TestObject> ptr3(ptr1);
            EXPECT_EQ(ptr1.use_count(), 3);
        }
        EXPECT_EQ(ptr1.use_count(), 2);
    }
    EXPECT_EQ(ptr1.use_count(), 1);
}

TEST_F(SharedPtrTest, UseCountAfterMove) {
    shared_ptr<TestObject> ptr1(new TestObject(42));
    shared_ptr<TestObject> ptr2(std::move(ptr1));

    EXPECT_EQ(ptr1.use_count(), 0);
    EXPECT_EQ(ptr2.use_count(), 1);
}

TEST_F(SharedPtrTest, UseCountAfterAssignment) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr1(new TestObject(1));
    shared_ptr<TestObject> ptr2(new TestObject(2));

    EXPECT_EQ(ptr1.use_count(), 1);
    EXPECT_EQ(ptr2.use_count(), 1);

    shared_ptr<TestObject> ptr3(ptr1);
    EXPECT_EQ(ptr1.use_count(), 2);
    EXPECT_EQ(ptr3.use_count(), 2);

    ptr3 = ptr2;
    EXPECT_EQ(ptr1.use_count(), 1);
    EXPECT_EQ(ptr2.use_count(), 2);
    EXPECT_EQ(ptr3.use_count(), 2);
}

// ============================================================================
// RESET METHOD
// ============================================================================

TEST_F(SharedPtrTest, ResetWithoutArguments) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        shared_ptr<TestObject> ptr2(ptr1);

        EXPECT_EQ(ptr1.use_count(), 2);
        ptr1.reset();
        EXPECT_FALSE(ptr1);
        EXPECT_EQ(ptr1.use_count(), 0);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(ptr2->value, 42);
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

TEST_F(SharedPtrTest, ResetWithNewPointerSharedOwnership) {
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

// ============================================================================
// CUSTOM DELETERS
// ============================================================================

TEST_F(SharedPtrTest, CustomDeleterCalled) {
    TestObject::destructor_count = 0;
    int deleter_call_count = 0;
    {
        auto custom_deleter = [&deleter_call_count](TestObject *ptr) {
            deleter_call_count++;
            delete ptr;
        };
        shared_ptr<TestObject> ptr(new TestObject(42), custom_deleter);
        EXPECT_EQ(ptr->value, 42);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(deleter_call_count, 0);
    }
    EXPECT_EQ(deleter_call_count, 1);
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CustomDeleterWithSharedOwnership) {
    TestObject::destructor_count = 0;
    int deleter_call_count = 0;
    {
        auto custom_deleter = [&deleter_call_count](TestObject *ptr) {
            deleter_call_count++;
            delete ptr;
        };
        shared_ptr<TestObject> ptr1(new TestObject(50), custom_deleter);
        shared_ptr<TestObject> ptr2(ptr1);

        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(deleter_call_count, 0);
    }
    EXPECT_EQ(deleter_call_count, 1);
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CustomDeleterWithReset) {
    TestObject::destructor_count = 0;
    int deleter_call_count = 0;
    {
        auto custom_deleter = [&deleter_call_count](TestObject *ptr) {
            deleter_call_count++;
            delete ptr;
        };
        shared_ptr<TestObject> ptr(new TestObject(10), custom_deleter);
        EXPECT_EQ(deleter_call_count, 0);

        ptr.reset();
        EXPECT_EQ(deleter_call_count, 1);
        EXPECT_EQ(TestObject::destructor_count, 1);
        EXPECT_FALSE(ptr);
    }
}

TEST_F(SharedPtrTest, CustomDeleterWithResetNewPointer) {
    TestObject::destructor_count = 0;
    int deleter_call_count = 0;
    {
        auto custom_deleter = [&deleter_call_count](TestObject *ptr) {
            deleter_call_count++;
            delete ptr;
        };
        shared_ptr<TestObject> ptr(new TestObject(5), custom_deleter);
        ptr.reset(new TestObject(15), custom_deleter);

        EXPECT_EQ(deleter_call_count, 1);
        EXPECT_EQ(ptr->value, 15);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(deleter_call_count, 2);
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, CustomDeleterWithFunctor) {
    struct CountingDeleter {
        int *call_count;
        explicit CountingDeleter(int *count) : call_count(count) {}
        void operator()(TestObject *ptr) const {
            (*call_count)++;
            delete ptr;
        }
    };

    TestObject::destructor_count = 0;
    int deleter_call_count = 0;
    {
        CountingDeleter deleter(&deleter_call_count);
        shared_ptr<TestObject> ptr(new TestObject(42), deleter);
        EXPECT_EQ(ptr->value, 42);
        EXPECT_EQ(deleter_call_count, 0);
    }
    EXPECT_EQ(deleter_call_count, 1);
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// ============================================================================
// EDGE CASES
// ============================================================================

TEST_F(SharedPtrTest, ComplexOwnershipScenario) {
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

TEST_F(SharedPtrTest, ValueTypeAlias) {
    static_assert(std::is_same_v<shared_ptr<int>::Value_Type, int>,
                  "Value_Type should be int for shared_ptr<int>");
    static_assert(std::is_same_v<shared_ptr<TestObject>::Value_Type, TestObject>,
                  "Value_Type should be TestObject for shared_ptr<TestObject>");
}

// ============================================================================
// WEAK_PTR TESTS
// ============================================================================

TEST_F(SharedPtrTest, WeakPtrDefaultConstructor) {
    weak_ptr<TestObject> wptr;
    EXPECT_EQ(wptr.use_count(), 0);
    EXPECT_TRUE(wptr.expired());
}

TEST_F(SharedPtrTest, WeakPtrFromSharedPtr) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> sptr(new TestObject(42));
        weak_ptr<TestObject> wptr(sptr);

        EXPECT_EQ(wptr.use_count(), 1);
        EXPECT_FALSE(wptr.expired());

        shared_ptr<TestObject> sptr2 = wptr.lock();
        EXPECT_TRUE(sptr2);
        EXPECT_EQ(sptr2->value, 42);
        EXPECT_EQ(sptr.use_count(), 2);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, WeakPtrLockOnExpired) {
    weak_ptr<TestObject> wptr;
    {
        shared_ptr<TestObject> sptr(new TestObject(50));
        wptr = sptr;
        EXPECT_FALSE(wptr.expired());
    }
    EXPECT_TRUE(wptr.expired());
    shared_ptr<TestObject> sptr = wptr.lock();
    EXPECT_FALSE(sptr);
    EXPECT_EQ(sptr.use_count(), 0);
}

TEST_F(SharedPtrTest, WeakPtrCopyConstructor) {
    shared_ptr<TestObject> sptr(new TestObject(100));
    weak_ptr<TestObject> wptr1(sptr);
    weak_ptr<TestObject> wptr2(wptr1);

    EXPECT_EQ(wptr1.use_count(), 1);
    EXPECT_EQ(wptr2.use_count(), 1);
    EXPECT_FALSE(wptr1.expired());
    EXPECT_FALSE(wptr2.expired());
}

TEST_F(SharedPtrTest, WeakPtrMoveConstructor) {
    shared_ptr<TestObject> sptr(new TestObject(100));
    weak_ptr<TestObject> wptr1(sptr);
    weak_ptr<TestObject> wptr2(std::move(wptr1));

    EXPECT_EQ(wptr2.use_count(), 1);
    EXPECT_FALSE(wptr2.expired());
}

TEST_F(SharedPtrTest, WeakPtrCopyAssignment) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> sptr1(new TestObject(10));
        shared_ptr<TestObject> sptr2(new TestObject(20));
        weak_ptr<TestObject> wptr1(sptr1);
        weak_ptr<TestObject> wptr2(sptr2);

        EXPECT_EQ(wptr1.use_count(), 1);
        EXPECT_EQ(wptr2.use_count(), 1);

        wptr1 = wptr2;
        EXPECT_EQ(wptr1.use_count(), 1);

        sptr1 = nullptr;
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, WeakPtrAssignmentFromSharedPtr) {
    shared_ptr<TestObject> sptr(new TestObject(75));
    weak_ptr<TestObject> wptr;

    EXPECT_EQ(wptr.use_count(), 0);
    wptr = sptr;
    EXPECT_EQ(wptr.use_count(), 1);
    EXPECT_FALSE(wptr.expired());
}

TEST_F(SharedPtrTest, WeakPtrMoveAssignment) {
    shared_ptr<TestObject> sptr1(new TestObject(30));
    shared_ptr<TestObject> sptr2(new TestObject(40));
    weak_ptr<TestObject> wptr1(sptr1);
    weak_ptr<TestObject> wptr2(sptr2);

    wptr1 = std::move(wptr2);
    EXPECT_EQ(wptr1.use_count(), 1);
}

TEST_F(SharedPtrTest, WeakPtrReset) {
    shared_ptr<TestObject> sptr(new TestObject(88));
    weak_ptr<TestObject> wptr(sptr);

    EXPECT_FALSE(wptr.expired());
    wptr.reset();
    EXPECT_TRUE(wptr.expired());
    EXPECT_EQ(wptr.use_count(), 0);
}

TEST_F(SharedPtrTest, WeakPtrSwap) {
    shared_ptr<TestObject> sptr1(new TestObject(60));
    shared_ptr<TestObject> sptr2(new TestObject(70));
    weak_ptr<TestObject> wptr1(sptr1);
    weak_ptr<TestObject> wptr2(sptr2);

    wptr1.swap(wptr2);

    EXPECT_EQ(wptr1.lock()->value, 70);
    EXPECT_EQ(wptr2.lock()->value, 60);
}

TEST_F(SharedPtrTest, SharedPtrFromWeakPtr) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> sptr1(new TestObject(99));
        weak_ptr<TestObject> wptr(sptr1);

        shared_ptr<TestObject> sptr2(wptr);
        EXPECT_EQ(sptr1.use_count(), 2);
        EXPECT_EQ(sptr2->value, 99);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, SharedPtrFromExpiredWeakPtr) {
    weak_ptr<TestObject> wptr;
    {
        shared_ptr<TestObject> sptr(new TestObject(11));
        wptr = sptr;
    }

    shared_ptr<TestObject> sptr(wptr);
    EXPECT_FALSE(sptr);
    EXPECT_EQ(sptr.use_count(), 0);
}

} // namespace ksl
