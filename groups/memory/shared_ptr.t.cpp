// Component being tested
#include <shared_ptr.h>

// Testing framework
#include <gtest/gtest.h>

namespace ksl {

class SharedPtrTest : public ::testing::Test {
  protected:
    struct Base {
        virtual ~Base() = default;
    };

    struct Derived : Base {
        int value;
        static int destructor_count;

        explicit Derived(int v = 42) : value(v) {}
        ~Derived() { destructor_count++; }
    };
};

int SharedPtrTest::Derived::destructor_count = 0;

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
    Derived::destructor_count = 0;
    {
        int *raw_int = new int(42);
        Derived *raw_obj = new Derived(100);
        shared_ptr<int> ptr1(raw_int);
        shared_ptr<Derived> ptr2(raw_obj);
        EXPECT_EQ(*ptr1, 42);
        EXPECT_EQ(ptr2->value, 100);
        EXPECT_EQ(ptr1.use_count(), 1);
        EXPECT_EQ(ptr2.use_count(), 1);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

// ============================================================================
// COPY AND MOVE CONSTRUCTORS
// ============================================================================

TEST_F(SharedPtrTest, CopyConstructor) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr1(new Derived(42));
        shared_ptr<Derived> ptr2(ptr1);
        shared_ptr<Derived> ptr3(ptr1);

        EXPECT_EQ(ptr1.get(), ptr2.get());
        EXPECT_EQ(ptr1.get(), ptr3.get());
        EXPECT_EQ(ptr1.use_count(), 3);
        EXPECT_EQ(ptr2.use_count(), 3);
        EXPECT_EQ(ptr3.use_count(), 3);
        EXPECT_EQ(Derived::destructor_count, 0);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyConstructorFromNullptr) {
    shared_ptr<Derived> ptr1(nullptr);
    shared_ptr<Derived> ptr2(ptr1);
    EXPECT_EQ(ptr2.get(), nullptr);
    EXPECT_EQ(ptr2.use_count(), 0);
}

TEST_F(SharedPtrTest, MoveConstructor) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr1(new Derived(42));
        shared_ptr<Derived> ptr2(std::move(ptr1));

        EXPECT_EQ(ptr2->value, 42);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(ptr1.get(), nullptr);
        EXPECT_EQ(ptr1.use_count(), 0);
        EXPECT_FALSE(ptr1);
        EXPECT_TRUE(ptr2);
        EXPECT_EQ(Derived::destructor_count, 0);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveConstructorChain) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr1(new Derived(100));
        shared_ptr<Derived> ptr2(std::move(ptr1));
        shared_ptr<Derived> ptr3(std::move(ptr2));

        EXPECT_EQ(ptr3->value, 100);
        EXPECT_EQ(ptr3.use_count(), 1);
        EXPECT_FALSE(ptr1);
        EXPECT_FALSE(ptr2);
        EXPECT_EQ(Derived::destructor_count, 0);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

// ============================================================================
// COPY AND MOVE ASSIGNMENT
// ============================================================================

TEST_F(SharedPtrTest, CopyAssignment) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr1(new Derived(10));
        shared_ptr<Derived> ptr2(new Derived(20));

        // Basic copy assignment
        ptr1 = ptr2;
        EXPECT_EQ(ptr1->value, 20);
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(ptr2.use_count(), 2);
        EXPECT_EQ(Derived::destructor_count, 1);
    }
    EXPECT_EQ(Derived::destructor_count, 2);
}

TEST_F(SharedPtrTest, CopyAssignmentSelfAssignment) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr(new Derived(42));
        ptr = ptr;
        EXPECT_EQ(ptr->value, 42);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(Derived::destructor_count, 0);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyAssignmentReturnsReference) {
    shared_ptr<int> ptr1(new int(1));
    shared_ptr<int> ptr2(new int(2));
    shared_ptr<int> &result = (ptr1 = ptr2);
    EXPECT_EQ(&result, &ptr1);
}

TEST_F(SharedPtrTest, CopyAssignmentFromNull) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr(new Derived(5));
        shared_ptr<Derived> null_ptr(nullptr);
        ptr = null_ptr;
        EXPECT_FALSE(ptr);
        EXPECT_EQ(ptr.use_count(), 0);
        EXPECT_EQ(Derived::destructor_count, 1);
    }
}

TEST_F(SharedPtrTest, MoveAssignment) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr1(new Derived(11));
        shared_ptr<Derived> ptr2(new Derived(22));

        ptr2 = std::move(ptr1);
        EXPECT_EQ(ptr2->value, 11);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_FALSE(ptr1);
        EXPECT_EQ(ptr1.use_count(), 0);
        EXPECT_EQ(Derived::destructor_count, 1);
    }
    EXPECT_EQ(Derived::destructor_count, 2);
}

TEST_F(SharedPtrTest, MoveAssignmentSelfAssignment) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr(new Derived(55));
        ptr = std::move(ptr);
        EXPECT_EQ(ptr->value, 55);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(Derived::destructor_count, 0);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveAssignmentWithSharedOwnership) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr1(new Derived(50));
        shared_ptr<Derived> ptr2(ptr1);
        shared_ptr<Derived> ptr3(new Derived(60));

        ptr3 = std::move(ptr1);

        EXPECT_FALSE(ptr1);
        EXPECT_EQ(ptr2->value, 50);
        EXPECT_EQ(ptr3->value, 50);
        EXPECT_EQ(ptr2.use_count(), 2);
        EXPECT_EQ(ptr3.use_count(), 2);
        EXPECT_EQ(Derived::destructor_count, 1);
    }
    EXPECT_EQ(Derived::destructor_count, 2);
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

    shared_ptr<Derived> ptr_obj(new Derived(77));
    EXPECT_EQ((*ptr_obj).value, 77);
    EXPECT_EQ(ptr_obj->value, 77);
    ptr_obj->value = 150;
    EXPECT_EQ(ptr_obj->value, 150);
}

TEST_F(SharedPtrTest, OperatorBool) {
    shared_ptr<Derived> ptr1(new Derived(42));
    shared_ptr<Derived> ptr2;
    shared_ptr<Derived> ptr3(nullptr);

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
    shared_ptr<Derived> ptr1(new Derived(42));
    EXPECT_EQ(ptr1.use_count(), 1);

    {
        shared_ptr<Derived> ptr2(ptr1);
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(ptr2.use_count(), 2);

        {
            shared_ptr<Derived> ptr3(ptr1);
            EXPECT_EQ(ptr1.use_count(), 3);
        }
        EXPECT_EQ(ptr1.use_count(), 2);
    }
    EXPECT_EQ(ptr1.use_count(), 1);
}

TEST_F(SharedPtrTest, UseCountAfterMove) {
    shared_ptr<Derived> ptr1(new Derived(42));
    shared_ptr<Derived> ptr2(std::move(ptr1));

    EXPECT_EQ(ptr1.use_count(), 0);
    EXPECT_EQ(ptr2.use_count(), 1);
}

TEST_F(SharedPtrTest, UseCountAfterAssignment) {
    Derived::destructor_count = 0;
    shared_ptr<Derived> ptr1(new Derived(1));
    shared_ptr<Derived> ptr2(new Derived(2));

    EXPECT_EQ(ptr1.use_count(), 1);
    EXPECT_EQ(ptr2.use_count(), 1);

    shared_ptr<Derived> ptr3(ptr1);
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
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr1(new Derived(42));
        shared_ptr<Derived> ptr2(ptr1);

        EXPECT_EQ(ptr1.use_count(), 2);
        ptr1.reset();
        EXPECT_FALSE(ptr1);
        EXPECT_EQ(ptr1.use_count(), 0);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(ptr2->value, 42);
        EXPECT_EQ(Derived::destructor_count, 0);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, ResetWithNewPointer) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr(new Derived(10));
        ptr.reset(new Derived(20));

        EXPECT_EQ(ptr->value, 20);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(Derived::destructor_count, 1);
    }
    EXPECT_EQ(Derived::destructor_count, 2);
}

TEST_F(SharedPtrTest, ResetWithNewPointerSharedOwnership) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr1(new Derived(5));
        shared_ptr<Derived> ptr2(ptr1);

        EXPECT_EQ(ptr1.use_count(), 2);
        ptr1.reset(new Derived(15));
        EXPECT_EQ(ptr1->value, 15);
        EXPECT_EQ(ptr1.use_count(), 1);
        EXPECT_EQ(ptr2->value, 5);
        EXPECT_EQ(ptr2.use_count(), 1);
        EXPECT_EQ(Derived::destructor_count, 0);
    }
    EXPECT_EQ(Derived::destructor_count, 2);
}

// ============================================================================
// CUSTOM DELETERS
// ============================================================================

TEST_F(SharedPtrTest, CustomDeleterCalled) {
    Derived::destructor_count = 0;
    int deleter_call_count = 0;
    {
        auto custom_deleter = [&deleter_call_count](Derived *ptr) {
            deleter_call_count++;
            delete ptr;
        };
        shared_ptr<Derived> ptr(new Derived(42), custom_deleter);
        EXPECT_EQ(ptr->value, 42);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(deleter_call_count, 0);
    }
    EXPECT_EQ(deleter_call_count, 1);
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, CustomDeleterWithSharedOwnership) {
    Derived::destructor_count = 0;
    int deleter_call_count = 0;
    {
        auto custom_deleter = [&deleter_call_count](Derived *ptr) {
            deleter_call_count++;
            delete ptr;
        };
        shared_ptr<Derived> ptr1(new Derived(50), custom_deleter);
        shared_ptr<Derived> ptr2(ptr1);

        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(deleter_call_count, 0);
    }
    EXPECT_EQ(deleter_call_count, 1);
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, CustomDeleterWithReset) {
    Derived::destructor_count = 0;
    int deleter_call_count = 0;
    {
        auto custom_deleter = [&deleter_call_count](Derived *ptr) {
            deleter_call_count++;
            delete ptr;
        };
        shared_ptr<Derived> ptr(new Derived(10), custom_deleter);
        EXPECT_EQ(deleter_call_count, 0);

        ptr.reset();
        EXPECT_EQ(deleter_call_count, 1);
        EXPECT_EQ(Derived::destructor_count, 1);
        EXPECT_FALSE(ptr);
    }
}

TEST_F(SharedPtrTest, CustomDeleterWithResetNewPointer) {
    Derived::destructor_count = 0;
    int deleter_call_count = 0;
    {
        auto custom_deleter = [&deleter_call_count](Derived *ptr) {
            deleter_call_count++;
            delete ptr;
        };
        shared_ptr<Derived> ptr(new Derived(5), custom_deleter);
        ptr.reset(new Derived(15), custom_deleter);

        EXPECT_EQ(deleter_call_count, 1);
        EXPECT_EQ(ptr->value, 15);
        EXPECT_EQ(ptr.use_count(), 1);
        EXPECT_EQ(Derived::destructor_count, 1);
    }
    EXPECT_EQ(deleter_call_count, 2);
    EXPECT_EQ(Derived::destructor_count, 2);
}

TEST_F(SharedPtrTest, CustomDeleterWithFunctor) {
    struct CountingDeleter {
        int *call_count;
        explicit CountingDeleter(int *count) : call_count(count) {}
        void operator()(Derived *ptr) const {
            (*call_count)++;
            delete ptr;
        }
    };

    Derived::destructor_count = 0;
    int deleter_call_count = 0;
    {
        CountingDeleter deleter(&deleter_call_count);
        shared_ptr<Derived> ptr(new Derived(42), deleter);
        EXPECT_EQ(ptr->value, 42);
        EXPECT_EQ(deleter_call_count, 0);
    }
    EXPECT_EQ(deleter_call_count, 1);
    EXPECT_EQ(Derived::destructor_count, 1);
}

// ============================================================================
// EDGE CASES
// ============================================================================

TEST_F(SharedPtrTest, ComplexOwnershipScenario) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> ptr1;
        shared_ptr<Derived> ptr2;
        shared_ptr<Derived> ptr3;
        {
            shared_ptr<Derived> temp(new Derived(100));
            ptr1 = temp;
            ptr2 = temp;
            ptr3 = temp;
            EXPECT_EQ(ptr1.use_count(), 4);
            EXPECT_EQ(Derived::destructor_count, 0);
        }
        EXPECT_EQ(ptr1.use_count(), 3);
        EXPECT_EQ(Derived::destructor_count, 0);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, DestructorPreservesWhenOtherOwnersExist) {
    Derived::destructor_count = 0;
    shared_ptr<Derived> ptr1;
    {
        shared_ptr<Derived> ptr2(new Derived(60));
        ptr1 = ptr2;
        EXPECT_EQ(ptr1.use_count(), 2);
        EXPECT_EQ(Derived::destructor_count, 0);
    }
    EXPECT_EQ(Derived::destructor_count, 0);
    EXPECT_EQ(ptr1->value, 60);
    EXPECT_EQ(ptr1.use_count(), 1);
}

TEST_F(SharedPtrTest, ValueTypeAlias) {
    static_assert(std::is_same_v<shared_ptr<int>::Value_Type, int>,
                  "Value_Type should be int for shared_ptr<int>");
    static_assert(std::is_same_v<shared_ptr<Derived>::Value_Type, Derived>,
                  "Value_Type should be Derived for shared_ptr<Derived>");
}

// ============================================================================
// WEAK_PTR TESTS
// ============================================================================

TEST_F(SharedPtrTest, WeakPtrDefaultConstructor) {
    weak_ptr<Derived> wptr;
    EXPECT_EQ(wptr.use_count(), 0);
    EXPECT_TRUE(wptr.expired());
}

TEST_F(SharedPtrTest, WeakPtrFromSharedPtr) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> sptr(new Derived(42));
        weak_ptr<Derived> wptr(sptr);

        EXPECT_EQ(wptr.use_count(), 1);
        EXPECT_FALSE(wptr.expired());

        shared_ptr<Derived> sptr2 = wptr.lock();
        EXPECT_TRUE(sptr2);
        EXPECT_EQ(sptr2->value, 42);
        EXPECT_EQ(sptr.use_count(), 2);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, WeakPtrLockOnExpired) {
    weak_ptr<Derived> wptr;
    {
        shared_ptr<Derived> sptr(new Derived(50));
        wptr = sptr;
        EXPECT_FALSE(wptr.expired());
    }
    EXPECT_TRUE(wptr.expired());
    shared_ptr<Derived> sptr = wptr.lock();
    EXPECT_FALSE(sptr);
    EXPECT_EQ(sptr.use_count(), 0);
}

TEST_F(SharedPtrTest, WeakPtrCopyConstructor) {
    shared_ptr<Derived> sptr(new Derived(100));
    weak_ptr<Derived> wptr1(sptr);
    weak_ptr<Derived> wptr2(wptr1);

    EXPECT_EQ(wptr1.use_count(), 1);
    EXPECT_EQ(wptr2.use_count(), 1);
    EXPECT_FALSE(wptr1.expired());
    EXPECT_FALSE(wptr2.expired());
}

TEST_F(SharedPtrTest, WeakPtrMoveConstructor) {
    shared_ptr<Derived> sptr(new Derived(100));
    weak_ptr<Derived> wptr1(sptr);
    weak_ptr<Derived> wptr2(std::move(wptr1));

    EXPECT_EQ(wptr2.use_count(), 1);
    EXPECT_FALSE(wptr2.expired());
}

TEST_F(SharedPtrTest, WeakPtrCopyAssignment) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> sptr1(new Derived(10));
        shared_ptr<Derived> sptr2(new Derived(20));
        weak_ptr<Derived> wptr1(sptr1);
        weak_ptr<Derived> wptr2(sptr2);

        EXPECT_EQ(wptr1.use_count(), 1);
        EXPECT_EQ(wptr2.use_count(), 1);

        wptr1 = wptr2;
        EXPECT_EQ(wptr1.use_count(), 1);

        sptr1 = nullptr;
        EXPECT_EQ(Derived::destructor_count, 1);
    }
    EXPECT_EQ(Derived::destructor_count, 2);
}

TEST_F(SharedPtrTest, WeakPtrAssignmentFromSharedPtr) {
    shared_ptr<Derived> sptr(new Derived(75));
    weak_ptr<Derived> wptr;

    EXPECT_EQ(wptr.use_count(), 0);
    wptr = sptr;
    EXPECT_EQ(wptr.use_count(), 1);
    EXPECT_FALSE(wptr.expired());
}

TEST_F(SharedPtrTest, WeakPtrMoveAssignment) {
    shared_ptr<Derived> sptr1(new Derived(30));
    shared_ptr<Derived> sptr2(new Derived(40));
    weak_ptr<Derived> wptr1(sptr1);
    weak_ptr<Derived> wptr2(sptr2);

    wptr1 = std::move(wptr2);
    EXPECT_EQ(wptr1.use_count(), 1);
}

TEST_F(SharedPtrTest, WeakPtrReset) {
    shared_ptr<Derived> sptr(new Derived(88));
    weak_ptr<Derived> wptr(sptr);

    EXPECT_FALSE(wptr.expired());
    wptr.reset();
    EXPECT_TRUE(wptr.expired());
    EXPECT_EQ(wptr.use_count(), 0);
}

TEST_F(SharedPtrTest, WeakPtrSwap) {
    shared_ptr<Derived> sptr1(new Derived(60));
    shared_ptr<Derived> sptr2(new Derived(70));
    weak_ptr<Derived> wptr1(sptr1);
    weak_ptr<Derived> wptr2(sptr2);

    wptr1.swap(wptr2);

    EXPECT_EQ(wptr1.lock()->value, 70);
    EXPECT_EQ(wptr2.lock()->value, 60);
}

TEST_F(SharedPtrTest, SharedPtrFromWeakPtr) {
    Derived::destructor_count = 0;
    {
        shared_ptr<Derived> sptr1(new Derived(99));
        weak_ptr<Derived> wptr(sptr1);

        shared_ptr<Derived> sptr2(wptr);
        EXPECT_EQ(sptr1.use_count(), 2);
        EXPECT_EQ(sptr2->value, 99);
    }
    EXPECT_EQ(Derived::destructor_count, 1);
}

TEST_F(SharedPtrTest, SharedPtrFromExpiredWeakPtr) {
    weak_ptr<Derived> wptr;
    {
        shared_ptr<Derived> sptr(new Derived(11));
        wptr = sptr;
    }

    shared_ptr<Derived> sptr(wptr);
    EXPECT_FALSE(sptr);
    EXPECT_EQ(sptr.use_count(), 0);
}

} // namespace ksl
