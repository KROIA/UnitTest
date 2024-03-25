#pragma once

#include "UnitTest_base.h"
#include <string>
#include <vector>
#include <functional>
#include <chrono>

// MSVC Compiler
#ifdef _MSC_VER 
#define __PRETTY_FUNCTION__ __FUNCSIG__
typedef std::chrono::steady_clock::time_point TimePoint;
#else
typedef std::chrono::system_clock::time_point TimePoint;
#endif

namespace UnitTest
{
	class UNIT_TEST_EXPORT Test
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
			bool success = false;

			
		};
		using TestFunction = std::function<bool(TestResults&)>;

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

#define ADD_TEST(test) addTest(this, &test)

		template<typename ObjectType>
		void addTest(ObjectType* obj, bool(ObjectType::* memberFunc)(TestResults& r))
		{
			m_testFunctions.push_back(bindMember(obj, memberFunc));
		}

		//void addTest(TestFunction func);
		void addTest(Test* subTest);

		// Connects an object member function to this signal
		template<typename ObjectType>
		static TestFunction bindMember(ObjectType* obj, bool(ObjectType::* memberFunc)(TestResults& r))
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

		//static std::vector<Test*> s_tests;
	};
}

#define TEST_CLASS(className) \
	public: \
	static className instance; \
	private:

#define TEST_INSTANTIATE(className) \
	className className::instance;

#define TEST_START(res) \
	bool success = true; \
	Test::TestResults &r = res; \
	r.name = __FUNCTION__;

#define TEST_END \
	r.setSuccess(success); \
	return success;

//#define TEST_FILE_LINE_STR (getName()+":" + std::to_string(__LINE__)+" ")
#define TEST_FILE_LINE_STR ("Line [" + std::to_string(__LINE__)+"]: ")

#define TEST_COMPARE(a, b) \
	if((a) == (b)) \
	{ \
        Test::TestResult res; \
        res.state = Test::ResultState::pass; \
        res.message = TEST_FILE_LINE_STR+"Expected (" + std::string(#a) + ") to equal (" + std::string(#b) + ")"; \
		r.results.push_back(res); \
    } \
	else \
	{ \
		Test::TestResult res; \
		res.state = Test::ResultState::fail; \
		res.message = TEST_FILE_LINE_STR+"Expected (" + std::string(#a) + ") to equal (" + std::string(#b)+ ")"; \
		r.results.push_back(res); \
		success = false; \
		onFail(res.message); \
		if(doesBreakOnFail()) \
		{ \
			TEST_END; \
		} \
	}

#define TEST_ASSERT_M(condition, assertMessage) \
	if((condition)) \
	{ \
		Test::TestResult res; \
		res.state = Test::ResultState::pass; \
		res.message = TEST_FILE_LINE_STR+"Expected (" + std::string(#condition) + ") to be true"; \
		r.results.push_back(res); \
	} \
	else \
	{ \
		Test::TestResult res; \
		res.state = Test::ResultState::fail; \
		res.message = TEST_FILE_LINE_STR+assertMessage; \
		r.results.push_back(res); \
		success = false; \
		onFail(res.message); \
		if(doesBreakOnFail()) \
		{ \
			TEST_END; \
		} \
	}

#define TEST_ASSERT(condition) \
    TEST_ASSERT_M(condition, "Expected (" + std::string(#condition) + ") to be true")

#define TEST_MESSAGE(msg) \
	{ \
		Test::TestResult res; \
		res.state = Test::ResultState::none; \
		res.message = TEST_FILE_LINE_STR+msg; \
		r.results.push_back(res); \
	}

#define TEST_FAIL(reason) \
	{ \
		Test::TestResult res; \
		res.state = Test::ResultState::fail; \
		res.message = TEST_FILE_LINE_STR+reason; \
		r.results.push_back(res); \
		success = false; \
		onFail(res.message); \
		if(doesBreakOnFail()) \
		{ \
			TEST_END; \
		} \
	}