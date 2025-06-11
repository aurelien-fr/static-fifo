/**
 * @file test_fifo.cpp
 * @author aurelien.dhiver@outlook.fr
 * 
 * Copyright (c) 2025 aurelien.dhiver@outlook.fr
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include "../FIFO.hpp"

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"


TEST_CASE("test_fifo_initial_state") {
    static constexpr auto FIFO_SIZE{5U};
    int value;

    Fifo<int, FIFO_SIZE> fifo{};
    CHECK_MESSAGE(fifo.getCount() == 0, "FIFO should be empty on initialization");
    CHECK_FALSE_MESSAGE(fifo.pop(&value),
                  "Read elements from empty FIFO should not be possible");
    CHECK_MESSAGE(fifo.read(&value, 1) == 0, "Read elements from empty FIFO should not be possible");
}

TEST_CASE("test_fifo_init_with_list") {
    static constexpr auto FIFO_SIZE{5U};

    Fifo<int, FIFO_SIZE> fifo{1, 2, 3};
    CHECK(fifo.getCount() == 3);
    CHECK(fifo[2] == 3);
}

TEST_CASE("test_fifo_push_and_pop") {
    static constexpr auto FIFO_SIZE{5U};
    Fifo<int, FIFO_SIZE> fifo{};
    int value;

    CHECK_MESSAGE(fifo.push(42), "Push should succeed");
    CHECK_MESSAGE(fifo.getCount() == 1, "FIFO count should be 1 after push");

    CHECK_MESSAGE(fifo.pop(&value), "Pop should succeed");
    CHECK_MESSAGE(value == 42, "Popped value should match pushed value");
    CHECK_MESSAGE(fifo.getCount() == 0, "FIFO should be empty after popping");
}

TEST_CASE("test_fifo_overflow") {
    static constexpr auto FIFO_SIZE{5U};

    Fifo<int, FIFO_SIZE> fifo{};
    std::array<int, FIFO_SIZE + 1> values = {1, 2, 3, 4, 5, 6};

    auto pushed = fifo.push(values, true);
    CHECK_MESSAGE(pushed, FIFO_SIZE, "Should push exactly FIFO_SIZE elements");
    CHECK_MESSAGE(fifo.getCount() == FIFO_SIZE, "FIFO should be full");
    CHECK_MESSAGE(fifo[0] == 2, "Second element shall be at first FIFO index");
    CHECK_MESSAGE(fifo[FIFO_SIZE - 1] == 6, "Last element shall be at first FIFO index");

    pushed = fifo.push(1984, false);
    CHECK_MESSAGE(pushed == 0, "Push should fail when FIFO is full and overwrite is false");
}

TEST_CASE("test_fifo_overwrite") {
    static constexpr auto FIFO_SIZE{5U};
    Fifo<int, FIFO_SIZE> fifo{};
    std::array<int, FIFO_SIZE> values = {1, 2, 3, 4, 5};
    std::array<int, FIFO_SIZE> values_2 = {11, 12, 13, 14, 15};
    int value;

    fifo.push(values, false);
    fifo.push(values_2, true);

    CHECK_MESSAGE(fifo.getCount() == FIFO_SIZE, "FIFO should remain full after overwrite");
    fifo.pop(&value);
    CHECK_MESSAGE(value == 11, "All elements should have been overwritten");
    fifo.pop(&value);
    CHECK_MESSAGE(value == 12, "Try another pop");
    CHECK_MESSAGE(fifo.getCount() == FIFO_SIZE - 2, "Check get count");

    Fifo<int, 5> expected = {13, 14, 15};
    CHECK_MESSAGE(fifo == expected, "fifo != expected");
}

TEST_CASE("test_fifo_reset") {
    static constexpr auto FIFO_SIZE{5U};
    Fifo<int, FIFO_SIZE> fifo{1, 2, 3};
    int val;

    fifo.reset();
    CHECK_MESSAGE(fifo.getCount() == 0, "FIFO should be empty after reset");
    CHECK_FALSE_MESSAGE(fifo.pop(&val),
                  "Read elements from empty FIFO should not be possible");
    CHECK_MESSAGE(fifo.read(&val, 1) == 0, "Read elements from empty FIFO should not be possible");
}

TEST_CASE("test_fifo_push") {
    static constexpr auto FIFO_SIZE{5U};
    Fifo<int, FIFO_SIZE> fifo{};
    std::array<int, 3> values = { 1, 2, 3};
    int singleValue = 45;

    CHECK_MESSAGE(fifo.push(values), "Push in empty FIFO");
    CHECK_MESSAGE(fifo.push(singleValue), "Push in FIFO that still has some space");
    CHECK_MESSAGE(fifo.push(-46), "Push again in FIFO that still has some space");
    CHECK_FALSE_MESSAGE(fifo.push(singleValue), "FIFO is full");
    CHECK_MESSAGE(fifo.getCount() == FIFO_SIZE, "FIFO should be full");
}

TEST_CASE("test_inequality") {
    Fifo<int, 4> fifo_0 = {1, 2, 3};
    Fifo<int, 4> fifo_1 = {0, 1, 2, 3};
    Fifo<int, 4> fifo_2 = {1, 2, 3, 0};
    Fifo<int, 4> fifo_3 = {-1, 2, 3};
    Fifo<int, 4> fifo_4 = {1, 2, -3};

    CHECK(fifo_0 != fifo_1);
    CHECK(fifo_0 != fifo_2);
    CHECK(fifo_0 != fifo_3);
    CHECK(fifo_0 != fifo_4);
}

TEST_CASE("test_equality") {
    Fifo<int, 5> fifo_0 = {1, 2, 3};
    Fifo<int, 5> fifo_1 = {1, 2, 3};
    Fifo<int, 5> fifo_2 = {0, 1, 2, 3};
    Fifo<int, 5> fifo_3 = {0, 0, 0, 0};

    CHECK(fifo_0 == fifo_1);

    int value;
    fifo_2.pop(&value);
    CHECK(fifo_0 == fifo_2);

    fifo_3.push({1, 2, 3}, true);
    std::array<int, 3> dummy{};
    fifo_3.pull(dummy.begin(), 2);

    CHECK(fifo_0 == fifo_3);
}

TEST_CASE("test_drop") {
    Fifo<int, 5> fifo = {0, 1, 2, 3};

    Fifo<int, 5> expected = {0, 1, 2, 3};
    fifo.drop(0);
    CHECK(fifo == expected);
    
    fifo.drop(1);
    CHECK(fifo.getCount() == 3);
    expected = {1, 2, 3};
    CHECK(fifo == expected);

    fifo.drop(10);
    CHECK(fifo.getCount() == 0);
    expected = {};
    CHECK(fifo == expected);

    fifo.drop(4);
    CHECK(fifo.getCount() == 0);
    expected = {};
    CHECK(fifo == expected);
}

TEST_CASE("test_pull") {
    Fifo<int, 7> fifo = {0, 1, 2, 3};
    auto expected = fifo;
    std::array<int, 10> buffer{};

    auto ret = fifo.pull(buffer.begin(), 0);
    CHECK(fifo == expected);
    CHECK(fifo.getCount() == 4);
    CHECK(ret == 0);

    ret = fifo.pull(buffer.begin(), 2);
    expected = {2, 3};
    CHECK(fifo == expected);
    CHECK(fifo.getCount() == 2);
    CHECK(ret == 2);
    CHECK(buffer[0] == 0);
    CHECK(buffer[1] == 1);


    ret = fifo.pull(buffer.begin(), 4);
    CHECK(ret == 2);
    CHECK(fifo.getCount() == 0);
    CHECK(buffer[0] == 2);
    CHECK(buffer[1] == 3);
}

TEST_CASE("test_pull_2") {
    Fifo<int, 4> fifo = {0, 0, 0, 0};
    fifo.push({1, 2, 3}, true);
    
    auto expected = Fifo<int, 4>{0, 1, 2, 3};
    std::array<int, 10> buffer{0xAA};

    auto ret = fifo.pull(buffer.begin(), 0);
    CHECK(fifo == expected);
    CHECK(fifo.getCount() == 4);
    CHECK(ret == 0);

    ret = fifo.pull(buffer.begin(), 1);
    expected = {1, 2, 3};
    CHECK(fifo == expected);
    CHECK(fifo.getCount() == 3);
    CHECK(ret == 1);
    CHECK(buffer[0] == 0);

    ret = fifo.pull(buffer.begin(), 4);
    CHECK(ret == 3);
    CHECK(fifo.getCount() == 0);
    CHECK(buffer[0] == 1);
    CHECK(buffer[1] == 2);
    CHECK(buffer[2] == 3);
}

TEST_CASE("test_read") {
    Fifo<int, 4> fifo = {0, 0, 0, 0};
    fifo.push({1, 2, 3}, true);
    
    auto expected = Fifo<int, 4>{0, 1, 2, 3};
    std::array<int, 10> buffer{0xAA};

    auto ret = fifo.read(buffer.begin(), 0);
    CHECK(fifo == expected);
    CHECK(fifo.getCount() == 4);
    CHECK(ret == 0);

    ret = fifo.read(buffer.begin(), 1);
    CHECK(fifo == expected);
    CHECK(fifo.getCount() == 4);
    CHECK(ret == 1);
    CHECK(buffer[0] == 0);

    ret = fifo.read(buffer.begin(), 10);
    CHECK(ret == 4);
    CHECK(fifo.getCount() == 4);
    std::array<int, 10> result = {0, 1, 2, 3};
    CHECK(buffer == result);
}
