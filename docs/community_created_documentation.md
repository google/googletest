# Community-Created Documentation

The following is a list, in no particular order, of links to documentation
created by the Googletest community.

* [Googlemock Insights](https://github.com/ElectricRCAircraftGuy/eRCaGuy_dotfiles/blob/master/googletest/insights.md), by [Gabriel Staples](https://www.linkedin.com/in/gabriel-staples/) ([ElectricRCAircraftGuy](https://github.com/ElectricRCAircraftGuy))

    This covers some of the more nuanced features such as using multiple `EXPECT_CALL()`s, and using `.WillRepeatedly(InvokeWithoutArgs([&callCounter](){ callCounter++; }));` to count the number of times a mock function is called between each call when doing successive calls of the function under test. 
