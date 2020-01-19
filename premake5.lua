project "gtest"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"googletest/include/gtest/gtest.h",
		"googletest/src/gtest-internal-inl.h",
		"googletest/src/gtest-all.cc"
		
	}

    includedirs
    {
		"googletest/include",
		"googletest"
    }

	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

		files
		{
		}

		defines
		{
		}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		files
		{
		}

		defines 
		{ 
			--"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
