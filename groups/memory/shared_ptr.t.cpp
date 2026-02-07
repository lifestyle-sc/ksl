#include <gtest/gtest.h>
#include <shared_ptr.h>

namespace ksl {

class SharedPtrTest : public ::testing::Test {
  protected:
    struct TestObject {
        int value;
        static int destructor_count;

        TestObject(int v = 42) : value(v) {}
        ~TestObject() { destructor_count++; }
    };
};

int SharedPtrTest::TestObject::destructor_count = 0;

// Basic Construction Tests
TEST_F(SharedPtrTest, ConstructorWithNullptr) {
    shared_ptr<int> ptr(nullptr);
    EXPECT_TRUE(ptr.operator->() == nullptr);
}

TEST_F(SharedPtrTest, ConstructorWithValidPointer) {
    int *raw_ptr = new int(42);
    shared_ptr<int> ptr(raw_ptr);
    EXPECT_EQ(*ptr, 42);
    EXPECT_EQ(ptr.operator->(), raw_ptr);
}

TEST_F(SharedPtrTest, ConstructorWithObject) {
    TestObject::destructor_count = 0;
    TestObject *obj = new TestObject(100);
    shared_ptr<TestObject> ptr(obj);
    EXPECT_EQ(ptr->value, 100);
    EXPECT_EQ((*ptr).value, 100);
}

// Dereference operator tests
TEST_F(SharedPtrTest, DereferenceOperator) {
    shared_ptr<int> ptr(new int(55));
    EXPECT_EQ(*ptr, 55);
    *ptr = 77;
    EXPECT_EQ(*ptr, 77);
}

TEST_F(SharedPtrTest, ArrowOperator) {
    TestObject *obj = new TestObject(200);
    shared_ptr<TestObject> ptr(obj);
    EXPECT_EQ(ptr->value, 200);
    ptr->value = 300;
    EXPECT_EQ(ptr->value, 300);
}

// Destructor Tests
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

TEST_F(SharedPtrTest, MultiplePointersToSameObject) {
    TestObject::destructor_count = 0;
    TestObject *obj = new TestObject(42);
    {
        shared_ptr<TestObject> ptr1(obj);
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// Reference Counting Tests
TEST_F(SharedPtrTest, ReferenceCountWithMultipleSharedPtrs) {
    TestObject::destructor_count = 0;
    TestObject *obj = new TestObject(42);
    shared_ptr<TestObject> ptr1(obj);
    {
        shared_ptr<TestObject> ptr2(obj);
        EXPECT_EQ(TestObject::destructor_count, 0);
        {
            shared_ptr<TestObject> ptr3(obj);
            EXPECT_EQ(TestObject::destructor_count, 0);
        }
        EXPECT_EQ(TestObject::destructor_count, 0);
    }
    EXPECT_EQ(TestObject::destructor_count, 0);
    ptr1.~shared_ptr();
    EXPECT_EQ(TestObject::destructor_count, 1);
}

// Edge cases
TEST_F(SharedPtrTest, IntPtr) {
    shared_ptr<int> ptr(new int(123));
    EXPECT_EQ(*ptr, 123);
}

TEST_F(SharedPtrTest, DoublePtr) {
    shared_ptr<double> ptr(new double(3.14));
    EXPECT_DOUBLE_EQ(*ptr, 3.14);
}

TEST_F(SharedPtrTest, StructPtr) {
    struct Point {
        int x, y;
    };
    shared_ptr<Point> ptr(new Point{10, 20});
    EXPECT_EQ(ptr->x, 10);
    EXPECT_EQ(ptr->y, 20);
}

TEST_F(SharedPtrTest, ModifyThroughPointer) {
    shared_ptr<int> ptr(new int(5));
    *ptr = 10;
    EXPECT_EQ(*ptr, 10);
}

TEST_F(SharedPtrTest, SequentialConstruction) {
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

} // namespace ksl
