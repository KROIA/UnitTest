#pragma once

#include "UnitTest.h"

class TST_simple : public UnitTest::Test
{
	TEST_CLASS(TST_simple)
public:
	TST_simple()
		: Test("TST_simple")
	{
		ADD_TEST(TST_simple::test1);
		setBreakOnFail(false);
		//ADD_TEST(TST_simple::test2);

	}

private:

	// Tests
	TEST_FUNCTION(test1)
	{
		TEST_START;

		TEST_MESSAGE("Using macro TEST_COMPARE");
		TEST_COMPARE(5, 5); // passes
		TEST_COMPARE(5, 4); // fails
		TEST_MESSAGE("");

		TEST_MESSAGE("Using macro TEST_COMPARE_F");
		TEST_COMPARE_F(5.5, 5.5, 0.001); // passes
		TEST_COMPARE_F(5.5, 5.4, 0.001); // fails
		TEST_MESSAGE("");

		TEST_MESSAGE("Using macro TEST_ASSERT");
		TEST_ASSERT(true); // passes
		TEST_ASSERT(false); // fails
		TEST_MESSAGE("");

		TEST_MESSAGE("Using macro TEST_ASSERT_M");
		TEST_ASSERT_M(true, "shuld not be visible to the console"); // passes
		TEST_ASSERT_M(false, "shuld be visible to the console"); // fails
		TEST_MESSAGE("");

		TEST_MESSAGE("Using macro TEST_FAIL");
		TEST_FAIL("Failed because of something"); // fails		
	}




	TEST_FUNCTION(test2)
	{
		TEST_START;

		int a = 0;
		TEST_ASSERT_M(a == 0, "is a == 0?");

		int b = 0;
		if (b != 0)
		{
			TEST_FAIL("b is not 0");
		}
		nastedTest(results);

		// fails if a != b
		TEST_COMPARE(a, b);
	}

	TEST_FUNCTION(nastedTest)
	{
		TEST_START;

		TEST_MESSAGE("nasted test");
		TEST_ASSERT_M(false, "Shuld fail");
	}

};

TEST_INSTANTIATE(TST_simple);