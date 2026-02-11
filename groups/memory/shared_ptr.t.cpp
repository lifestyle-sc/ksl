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
// BASIC CONSTRUCTION TESTS
// ============================================================================

TEST_F(SharedPtrTest, DefaultConstructor) {
    shared_ptr<int> ptr;
    EXPECT_EQ(ptr.operator->(), nullptr);
}

TEST_F(SharedPtrTest, ExplicitNullptrConstructor) {
    shared_ptr<int> ptr(nullptr);
    EXPECT_EQ(ptr.operator->(), nullptr);
}

TEST_F(SharedPtrTest, ConstructorWithValidIntPointer) {
    int *raw_ptr = new int(42);
    shared_ptr<int> ptr(raw_ptr);
    EXPECT_EQ(*ptr, 42);
    EXPECT_EQ(ptr.operator->(), raw_ptr);
}

TEST_F(SharedPtrTest, ConstructorWithValidObjectPointer) {
    TestObject::destructor_count = 0;
    TestObject::constructor_count = 0;
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
        // ptr2 destroyed but object should still exist
        EXPECT_EQ(TestObject::destructor_count, 0);
        EXPECT_EQ(ptr1->value, 42);
    }
    // ptr1 destroyed, object should be deleted
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyConstructorWithMultipleCopies) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(50));
        {
            shared_ptr<TestObject> ptr2(ptr1);
            {
                shared_ptr<TestObject> ptr3(ptr2);
                {
                    shared_ptr<TestObject> ptr4(ptr3);
                    EXPECT_EQ(ptr4->value, 50);
                    EXPECT_EQ(TestObject::destructor_count, 0);
                }
                EXPECT_EQ(TestObject::destructor_count, 0);
            }
            EXPECT_EQ(TestObject::destructor_count, 0);
        }
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyConstructorWithNullptr) {
    shared_ptr<int> ptr1(nullptr);
    shared_ptr<int> ptr2(ptr1);
    EXPECT_EQ(ptr2.operator->(), nullptr);
}

TEST_F(SharedPtrTest, CopyConstructorWithDifferentScopes) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr1;
    {
        shared_ptr<TestObject> ptr2(new TestObject(75));
        ptr1 = ptr2; // Copy assignment, shares ownership
        EXPECT_EQ(ptr1->value, 75);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    // ptr2 destroyed but ptr1 still holds reference
    EXPECT_EQ(TestObject::destructor_count, 0);
    EXPECT_EQ(ptr1->value, 75);
}

// ============================================================================
// COPY ASSIGNMENT TESTS
// ============================================================================

TEST_F(SharedPtrTest, CopyAssignmentSharesOwnership) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr1(new TestObject(10));
    shared_ptr<TestObject> ptr2(new TestObject(20));

    // Before assignment
    EXPECT_EQ(ptr1->value, 10);
    EXPECT_EQ(ptr2->value, 20);
    EXPECT_EQ(TestObject::destructor_count, 0);

    // Assign ptr2 to ptr1
    ptr1 = ptr2;

    // After assignment
    EXPECT_EQ(ptr1->value, 20);
    EXPECT_EQ(ptr2->value, 20);
}

TEST_F(SharedPtrTest, CopyAssignmentIncreasesRefCount) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(99));
        {
            shared_ptr<TestObject> ptr2(new TestObject(1));
            ptr2 = ptr1; // ptr2 now shares ptr1's object
            EXPECT_EQ(ptr2->value, 99);
            EXPECT_EQ(TestObject::destructor_count, 1); // original ptr2's object deleted
        }
        // ptr2 destroyed but object still alive through ptr1
        EXPECT_EQ(TestObject::destructor_count, 1);
        EXPECT_EQ(ptr1->value, 99);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, CopyAssignmentToSelf) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        ptr1 = ptr1; // Self assignment
        EXPECT_EQ(ptr1->value, 42);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, CopyAssignmentWithNullptr) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(55));
        shared_ptr<TestObject> ptr2(nullptr);
        ptr2 = ptr1;
        EXPECT_EQ(ptr2->value, 55);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
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
}

TEST_F(SharedPtrTest, DereferenceOperatorModifyObject) {
    shared_ptr<TestObject> ptr(new TestObject(100));
    (*ptr).value = 200;
    EXPECT_EQ(ptr->value, 200);
}

// ============================================================================
// ARROW OPERATOR TESTS
// ============================================================================

TEST_F(SharedPtrTest, ArrowOperatorAccess) {
    TestObject *obj = new TestObject(88);
    shared_ptr<TestObject> ptr(obj);
    EXPECT_EQ(ptr->value, 88);
}

TEST_F(SharedPtrTest, ArrowOperatorModify) {
    shared_ptr<TestObject> ptr(new TestObject(50));
    ptr->value = 150;
    EXPECT_EQ(ptr->value, 150);
}

TEST_F(SharedPtrTest, ArrowOperatorWithComplexType) {
    struct ComplexType {
        int a;
        double b;
        const char *str;
    };
    shared_ptr<ComplexType> ptr(new ComplexType{5, 3.14, "test"});
    EXPECT_EQ(ptr->a, 5);
    EXPECT_DOUBLE_EQ(ptr->b, 3.14);
    EXPECT_STREQ(ptr->str, "test");
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
    // ptr2 destroyed but object still owned by ptr1
    EXPECT_EQ(TestObject::destructor_count, 0);
    EXPECT_EQ(ptr1->value, 60);
}

// ============================================================================
// REFERENCE COUNTING TESTS
// ============================================================================

TEST_F(SharedPtrTest, ReferenceCountingThreeOwners) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(111));
        {
            shared_ptr<TestObject> ptr2(ptr1);
            {
                shared_ptr<TestObject> ptr3(ptr2);
                EXPECT_EQ(TestObject::destructor_count, 0);
            }
            EXPECT_EQ(TestObject::destructor_count, 0);
        }
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// ============================================================================
// TYPE COMPATIBILITY TESTS
// ============================================================================

TEST_F(SharedPtrTest, IntType) {
    shared_ptr<int> ptr(new int(123));
    EXPECT_EQ(*ptr, 123);
}

TEST_F(SharedPtrTest, DoubleType) {
    shared_ptr<double> ptr(new double(3.14159));
    EXPECT_DOUBLE_EQ(*ptr, 3.14159);
}

TEST_F(SharedPtrTest, CharType) {
    shared_ptr<char> ptr(new char('X'));
    EXPECT_EQ(*ptr, 'X');
}

TEST_F(SharedPtrTest, StructType) {
    struct Point {
        int x;
        int y;
    };
    shared_ptr<Point> ptr(new Point{10, 20});
    EXPECT_EQ(ptr->x, 10);
    EXPECT_EQ(ptr->y, 20);
}

TEST_F(SharedPtrTest, ComplexStructType) {
    struct Rectangle {
        int width;
        int height;
        double area;
    };
    shared_ptr<Rectangle> ptr(new Rectangle{5, 10, 50.0});
    EXPECT_EQ(ptr->width, 5);
    EXPECT_EQ(ptr->height, 10);
    EXPECT_DOUBLE_EQ(ptr->area, 50.0);
}

// ============================================================================
// EDGE CASES AND CORNER CASES
// ============================================================================

TEST_F(SharedPtrTest, SequentialCreationAndDestruction) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(1));
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
    {
        shared_ptr<TestObject> ptr2(new TestObject(2));
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

TEST_F(SharedPtrTest, ModifyValueMultipleTimes) {
    shared_ptr<int> ptr(new int(0));
    for (int i = 1; i <= 10; i++) {
        *ptr = i;
        EXPECT_EQ(*ptr, i);
    }
}

TEST_F(SharedPtrTest, CopyAssignmentChain) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr1(new TestObject(1));
    shared_ptr<TestObject> ptr2(new TestObject(2));
    shared_ptr<TestObject> ptr3(new TestObject(3));

    ptr3 = ptr2;
    ptr2 = ptr1;

    EXPECT_EQ(ptr1->value, 1);
    EXPECT_EQ(ptr2->value, 1);
    EXPECT_EQ(ptr3->value, 2);
    EXPECT_EQ(TestObject::destructor_count, 1); // Object with value 3 deleted
}

TEST_F(SharedPtrTest, ZeroValue) {
    shared_ptr<int> ptr(new int(0));
    EXPECT_EQ(*ptr, 0);
}

TEST_F(SharedPtrTest, NegativeValue) {
    shared_ptr<int> ptr(new int(-42));
    EXPECT_EQ(*ptr, -42);
}

TEST_F(SharedPtrTest, LargeValue) {
    shared_ptr<long long> ptr(new long long(9223372036854775807LL));
    EXPECT_EQ(*ptr, 9223372036854775807LL);
}

// ============================================================================
// DEFAULT CONSTRUCTOR AND NULLPTR BEHAVIOR
// ============================================================================

TEST_F(SharedPtrTest, DefaultConstructorIsNoexcept) {
    // Verify that default constructor can be called in noexcept context
    auto test = []() noexcept { shared_ptr<int> ptr; };
    test();
}

TEST_F(SharedPtrTest, NullptrConstructorIsNoexcept) {
    auto test = []() noexcept { shared_ptr<int> ptr(nullptr); };
    test();
}

TEST_F(SharedPtrTest, DefaultConstructorMultipleInstances) {
    shared_ptr<int> ptr1;
    shared_ptr<int> ptr2;
    shared_ptr<int> ptr3;
    EXPECT_EQ(ptr1.operator->(), nullptr);
    EXPECT_EQ(ptr2.operator->(), nullptr);
    EXPECT_EQ(ptr3.operator->(), nullptr);
}

// ============================================================================
// REFERENCE COUNTING WITH NULLPTR
// ============================================================================

TEST_F(SharedPtrTest, CopyConstructorWithDefaultConstructor) {
    shared_ptr<int> ptr1;
    shared_ptr<int> ptr2(ptr1);
    EXPECT_EQ(ptr2.operator->(), nullptr);
}

TEST_F(SharedPtrTest, CopyAssignmentWithDefaultConstructors) {
    shared_ptr<int> ptr1;
    shared_ptr<int> ptr2;
    ptr1 = ptr2;
    EXPECT_EQ(ptr1.operator->(), nullptr);
}

TEST_F(SharedPtrTest, CopyAssignmentFromNullptrToValidPointer) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr1(nullptr);
    shared_ptr<TestObject> ptr2(new TestObject(42));

    ptr1 = ptr2;
    EXPECT_EQ(ptr1->value, 42);
    EXPECT_EQ(TestObject::destructor_count, 0);
}

TEST_F(SharedPtrTest, CopyAssignmentFromValidPointerToNullptr) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(42));
        shared_ptr<TestObject> ptr2(nullptr);

        ptr1 = ptr2;
        EXPECT_EQ(ptr1.operator->(), nullptr);
        EXPECT_EQ(TestObject::destructor_count, 1);
    }
}

// ============================================================================
// OPERATOR CONST-CORRECTNESS
// ============================================================================

TEST_F(SharedPtrTest, DereferenceOperatorConst) {
    const shared_ptr<int> ptr(new int(42));
    EXPECT_EQ(*ptr, 42);
}

TEST_F(SharedPtrTest, ArrowOperatorConst) {
    const shared_ptr<TestObject> ptr(new TestObject(42));
    EXPECT_EQ(ptr->value, 42);
}

// ============================================================================
// COMPLEX ASSIGNMENT SCENARIOS
// ============================================================================

TEST_F(SharedPtrTest, AssignmentFromDifferentScopesToSameControlBlock) {
    TestObject::destructor_count = 0;
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
// MULTIPLE POINTER MANAGEMENT
// ============================================================================

TEST_F(SharedPtrTest, FourPointersToSameObject) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(200));
        {
            shared_ptr<TestObject> ptr2(ptr1);
            {
                shared_ptr<TestObject> ptr3(ptr2);
                {
                    shared_ptr<TestObject> ptr4(ptr3);
                    EXPECT_EQ(ptr1->value, 200);
                    EXPECT_EQ(ptr2->value, 200);
                    EXPECT_EQ(ptr3->value, 200);
                    EXPECT_EQ(ptr4->value, 200);
                }
                EXPECT_EQ(TestObject::destructor_count, 0);
            }
            EXPECT_EQ(TestObject::destructor_count, 0);
        }
        EXPECT_EQ(TestObject::destructor_count, 0);
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

// ============================================================================
// RELEASE AND CLEANUP VERIFICATION
// ============================================================================

TEST_F(SharedPtrTest, DestructorCallsReleaseCorrectly) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(1));
        shared_ptr<TestObject> ptr2(ptr1);
        shared_ptr<TestObject> ptr3(ptr1);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, ReleaseZerosRefCountBeforeDeletion) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(10));
        {
            shared_ptr<TestObject> ptr2(ptr1);
            {
                shared_ptr<TestObject> ptr3(ptr1);
                // ref_count should be 3
            }
            // After ptr3 destroyed, ref_count should be 2
            EXPECT_EQ(TestObject::destructor_count, 0);
        }
        // After ptr2 destroyed, ref_count should be 1
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    // After ptr1 destroyed, ref_count should be 0 and object deleted
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, AssignmentReturnValue) {
    TestObject::destructor_count = 0;
    shared_ptr<TestObject> ptr1(new TestObject(1));
    shared_ptr<TestObject> ptr2(new TestObject(2));

    shared_ptr<TestObject> &result = (ptr1 = ptr2);
    EXPECT_EQ(result.operator->(), ptr1.operator->());
    EXPECT_EQ(result->value, 2);
}

// ============================================================================
// EDGE CASES WITH DIFFERENT DATA TYPES
// ============================================================================

TEST_F(SharedPtrTest, FunctionPointerType) {
    auto func = [](int x) { return x * 2; };
    shared_ptr<decltype(func)> ptr(new decltype(func)(func));
    EXPECT_EQ((**ptr)(5), 10);
}

TEST_F(SharedPtrTest, PointerToPointer) {
    int *raw_ptr = new int(42);
    {
        shared_ptr<int *> ptr(new int *(raw_ptr));
        EXPECT_EQ(**ptr, 42);
    }
    delete raw_ptr;
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
        // EXPECT_EQ(ptr1.operator->(), nullptr); // undefined state after move
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
    EXPECT_EQ(ptr2.operator->(), nullptr);
    // EXPECT_EQ(ptr1.operator->(), nullptr); // undefined state after move
}

TEST_F(SharedPtrTest, MoveConstructorWithComplexType) {
    struct Complex {
        int x;
        int y;
        double z;
    };
    Complex *obj = new Complex{10, 20, 3.14};
    shared_ptr<Complex> ptr1(obj);
    shared_ptr<Complex> ptr2(std::move(ptr1));

    EXPECT_EQ(ptr2->x, 10);
    EXPECT_EQ(ptr2->y, 20);
    EXPECT_DOUBLE_EQ(ptr2->z, 3.14);
    // EXPECT_EQ(ptr1.operator->(), nullptr); // undefined state
}

TEST_F(SharedPtrTest, MoveConstructorMultipleChain) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(100));
        shared_ptr<TestObject> ptr2(std::move(ptr1));
        shared_ptr<TestObject> ptr3(std::move(ptr2));

        EXPECT_EQ(ptr3->value, 100);
        // EXPECT_EQ(ptr1.operator->(), nullptr); // undefined state
        // EXPECT_EQ(ptr2.operator->(), nullptr); // undefined state
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

TEST_F(SharedPtrTest, MoveConstructorInFunction) {
    TestObject::destructor_count = 0;
    auto create_ptr = []() -> shared_ptr<TestObject> {
        return shared_ptr<TestObject>(new TestObject(77));
    };

    shared_ptr<TestObject> ptr = create_ptr();
    EXPECT_EQ(ptr->value, 77);
    EXPECT_EQ(TestObject::destructor_count, 0);
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
        // EXPECT_EQ(ptr1.operator->(), nullptr); // undefined behaviour
        EXPECT_EQ(TestObject::destructor_count, 1); // Object with value 22 deleted
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
        // EXPECT_EQ(ptr1.operator->(), nullptr); // undefined state
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
        EXPECT_EQ(ptr1.operator->(), nullptr);
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
            EXPECT_EQ(TestObject::destructor_count, 1); // Original ptr1 object deleted
        }
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

// ============================================================================
// MOVE SEMANTICS WITH SHARED OWNERSHIP
// ============================================================================

TEST_F(SharedPtrTest, MoveAssignmentWithSharedOwnership) {
    TestObject::destructor_count = 0;
    {
        shared_ptr<TestObject> ptr1(new TestObject(50));
        shared_ptr<TestObject> ptr2(ptr1); // ptr1 and ptr2 share ownership
        shared_ptr<TestObject> ptr3(new TestObject(60));

        ptr3 = std::move(ptr1); // Move ptr1's ownership to ptr3

        // EXPECT_EQ(ptr1.operator->(), nullptr); // undefined state
        EXPECT_EQ(ptr2->value, 50); // ptr2 still owns the object
        EXPECT_EQ(ptr3->value, 50);
        EXPECT_EQ(TestObject::destructor_count, 1); // Object with value 60 deleted
    }
    EXPECT_EQ(TestObject::destructor_count, 2);
}

// ============================================================================
// VALUE TYPE ALIAS TEST
// ============================================================================

TEST_F(SharedPtrTest, ValueTypeAlias) {
    // Verify that Value_Type alias is correctly defined
    static_assert(std::is_same_v<shared_ptr<int>::Value_Type, int>,
                  "Value_Type should be int for shared_ptr<int>");
    static_assert(std::is_same_v<shared_ptr<TestObject>::Value_Type, TestObject>,
                  "Value_Type should be TestObject for shared_ptr<TestObject>");
}

} // namespace ksl
