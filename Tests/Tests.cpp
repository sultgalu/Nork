#include "pch.h"
#include "../Nork/Source/PCH/pch.h"
#include "../Nork/Source/ThirdParty/include/glfw/glfw3.h"
#include "CppUnitTest.h"
#include "../Nork/Source/Platform/Interface/Windows.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Tests
{
	TEST_CLASS(Tests)
	{
	public:
		
		TEST_METHOD(TestMethod1)
		{
			Nork::Window<GLFWwindow> win(1920, 1080);
			Assert::IsNotNull(&win.GetData());
		}
	};
}
