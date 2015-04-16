#include <mbgl/util/thread.hpp>
#include <mbgl/util/run_loop.hpp>
#include <mbgl/util/std.hpp>

#include "../fixtures/util.hpp"

using namespace mbgl::util;

class TestObject {
public:
    void fn1(int val) { ASSERT_EQ(val, 1); }
    int  fn2() { return 1; }

    void transferIn(std::unique_ptr<int> val) {
        ASSERT_EQ(*val, 1);
    }

    std::unique_ptr<int> transferOut() {
        return make_unique<int>(1);
    }

    std::unique_ptr<int> transferInOut(std::unique_ptr<int> val) {
        return val;
    }
};

TEST(Thread, invoke) {
    RunLoop loop;

    loop.invoke([&] {
        Thread<TestObject> thread("Test");

        thread.invoke(&TestObject::fn1, 1);
        thread.invokeWithResult(&TestObject::fn2, [&] (int result) {
            ASSERT_EQ(result, 1);
        });

        thread.invoke(&TestObject::transferIn, make_unique<int>(1));
        thread.invokeWithResult(&TestObject::transferOut, [&] (std::unique_ptr<int> result) {
            ASSERT_EQ(*result, 1);
        });

        // Can't figure out how to get this to work.
//        thread.invokeWithResult(&TestObject::transferInOut, [&] (std::unique_ptr<int> result) {
//            ASSERT_EQ(*result, 1);
//            loop.stop();
//        }, make_unique<int>(1));

        loop.stop();
    });

    loop.run();
}
