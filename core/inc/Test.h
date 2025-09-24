#pragma once

#include "UnitTest_base.h"
#include <string>
#include <vector>
#include <functional>
#include <chrono>

/*
	Test macros
	+-------------------------------------------+---------------------------------------------------------------------------+
	|     Name	                                |                             Functionality                                 |
	+-------------------------------------------+---------------------------------------------------------------------------+
	| TEST_COMPARE(a, b)                        | Compares a == b, if false, the test fails.                                |
	| TEST_COMPARE_F(a, b, margin)              | Compares abs(a - b) <= margin, if false, the test fails.                  |
	|                                           | Use this macro to compare float or double values and set a low margin.    |
	| TEST_ASSERT(condition)                    | Checks condition == true, if the condition is false, the test fails.      |
	| TEST_ASSERT_M(condition, assertMessage)   | Same as TEST_ASSERT, but the fail message can be defined.                 |
	| TEST_FAIL(assertMessage)                  | Fails the test and uses the assertMessage as reason for the fail.         |
	| TEST_MESSAGE(msg)                         | Prints a text to the test log. This does never fail.                      |
	+-------------------------------------------+---------------------------------------------------------------------------+
*/

// MSVC Compiler
#ifdef _MSC_VER 
#define __PRETTY_FUNCTION__ __FUNCSIG__
typedef std::chrono::steady_clock::time_point TimePoint;
#else
typedef std::chrono::system_clock::time_point TimePoint;
#endif

namespace UnitTest
{
	class UNIT_TEST_API Test
	{
	public:
		enum ResultState
		{
			none,
			pass,
			fail
		};
		struct TestResult
		{
			ResultState state;
			unsigned int lineNr;
			std::string message;
		};
		struct TestResults
		{
			std::string name;
			std::vector<TestResult> results;
			std::vector<TestResults> subResults;

			bool getSuccess() const
			{
				if (!success)
					return false;
				for(const auto& res : results)
				{
					if (res.state == ResultState::fail)
					{
						return false;
					}
				}
				for(const auto& subRes : subResults)
				{
					if (!subRes.getSuccess())
					{
						return false;
					}
				}
				return true;
			}
			void setSuccess(bool success)
			{
				this->success = success;
			}
		private:
			bool success = true;

			
		};
		using TestFunction = std::function<void(TestResults&)>;

		Test(const std::string& name);
		virtual ~Test();


		bool runTests();
		const TestResults& getResults() const;

		void setBreakOnFail(bool breakOnFail);
		bool doesBreakOnFail() const;

		void printResults() const;

		const std::string& getName() const
		{
			return m_name;
		}

		static const std::vector<Test*>& getTests();
		static bool runAllTests(TestResults& results);
		static void printResults(const TestResults& results);
	protected:



		template<typename ObjectType>
		void addTest(ObjectType* obj, void(ObjectType::* memberFunc)(TestResults& r))
		{
			m_testFunctions.push_back(bindMember(obj, memberFunc));
		}

		//void addTest(TestFunction func);
		void addTest(Test* subTest);

		// Connects an object member function to this signal
		template<typename ObjectType>
		static TestFunction bindMember(ObjectType* obj, void(ObjectType::* memberFunc)(TestResults& r))
		{
			return [obj, memberFunc](TestResults& r) { return (obj->*memberFunc)(r); };
		}

		virtual void onFail(const std::string& message)
		{
			UT_UNUSED(message);
		}
		virtual void onTestsStart()
		{ }
		virtual void onTestsEnd()
		{ }

	private:
		static std::vector<Test*>& getTestsInternal();
		static void printResultsRecursive(const TestResults& results, int depth);
		static void printColored(const std::string& str, int color);
		bool runTests(TestResults& results);

		std::vector<TestFunction> m_testFunctions;
		std::vector<Test*> m_subTests;

		TestResults m_results;

		std::string m_name;
		bool m_breakTestOnFail;
	};
}

#define TEST_CLASS(className) \
	public: \
	static className instance; \
	private:

#define TEST_INSTANTIATE(className) \
	className className::instance;

#define ADD_TEST(test) addTest(this, &test)
#define TEST_FUNCTION(name) void name(TestResults& results)

#define TEST_START \
	results.name = __FUNCTION__;

#define TEST_FILE_LINE_STR ("Line [" + std::to_string(__LINE__)+"]: ")

#define TEST_COMPARE(a, b) \
	if((a) == (b)) \
	{ \
        Test::TestResult res; \
        res.state = Test::ResultState::pass; \
		res.lineNr = __LINE__; \
        res.message = "Expected (" + std::string(#a) + ") to equal (" + std::string(#b) + ")"; \
		results.results.push_back(res); \
    } \
	else \
	{ \
		TEST_FAIL("Expected (" + std::string(#a) + ") to equal (" + std::string(#b)+ ")"); \
	}

#define TEST_COMPARE_F(a, b, margin) \
	if(std::abs((a) - (b)) <= std::abs(margin)) \
	{ \
        Test::TestResult res; \
        res.state = Test::ResultState::pass; \
		res.lineNr = __LINE__; \
        res.message = "Expected (" + std::string(#a) + ") to equal (" + std::string(#b) + ") +-"+std::to_string(std::abs(margin)*0.5); \
		results.results.push_back(res); \
    } \
	else \
	{ \
		TEST_FAIL("Expected (" + std::string(#a) + ") to equal (" + std::string(#b)+ ") +-"+std::to_string(std::abs(margin)*0.5)); \
	}


#define TEST_ASSERT_M(condition, assertMessage) \
	if((condition)) \
	{ \
		Test::TestResult res; \
		res.state = Test::ResultState::pass; \
		res.lineNr = __LINE__; \
		res.message = "Expected (" + std::string(#condition) + ") to be true"; \
		results.results.push_back(res); \
	} \
	else \
	{ \
		TEST_FAIL(assertMessage); \
	}

#define TEST_ASSERT(condition) \
    TEST_ASSERT_M(condition, "Expected (" + std::string(#condition) + ") to be true")

#define TEST_MESSAGE(msg) \
	{ \
		Test::TestResult res; \
		res.state = Test::ResultState::none; \
		res.lineNr = __LINE__; \
		res.message = msg; \
		results.results.push_back(res); \
	}

#define TEST_FAIL(assertMessage) \
	{ \
		Test::TestResult res; \
		res.state = Test::ResultState::fail; \
		res.lineNr = __LINE__; \
		res.message = (assertMessage); \
		results.results.push_back(res); \
		results.setSuccess(false); \
		/*success = false;*/ \
		onFail(res.message); \
		if(doesBreakOnFail()) \
		{ \
			return; \
		} \
	}