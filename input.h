/*************************************************************************
* Title: numeric, string and menu input validation
* File: input.h
* Author: James Eli
* Date: 12/29/2017
*
* Integer/float console input validation. Checks string input for valid
* characters, and range. Will repeat input prompting for specified
* number of attempts. Can be expanded to additional types. Invlid input
* throws an exception, all of which are caught inside the getNumber
* function.
*
* Notes:
*  (1) Requires C++11 or greater.
*  (2) Compiled with MS Visual Studio 2017 Community (v141), and/or
*      Eclipse GCC 5.3.0.
*  (2) Floating point input rejects scientific notation.
*
* Submitted in partial fulfillment of the requirements of PCC CIS-278.
*************************************************************************
* Change Log:
*   12/29/2017: Initial release. JME
*   01/22/2018: Add type_traits check, static_assert and constants. JME
*   02/07/2017: Removed assertions. JME
*   02/21/2018: Removed "using namespace std". JME
*   03/03/2018: Added header file, string and menu input. JME
*************************************************************************/
#ifndef _INPUT_VALIDATION_H_
#define _INPUT_VALIDATION_H_

#include <iostream>    // console input/output
#include <iomanip>     // setprecision
#include <string>      // string handling
#include <limits>      // numeric limits
#include <type_traits> // is_arithmetic

// Menu choice enumerations (ascii char codes).
enum class Choice 
{
	INVALID = 0,
	SELECT = 115, REMOVE = 100, UPDATE = 117, INSERT = 105, QUIT = 113,
	/* SELECT */_1 = 49, /* REMOVE */_2 = 50, /*UPDATE */_3 = 51, /* INSERT */_4 = 52, /* QUIT */_5 = 53
};

// Maximum input attempts.
const unsigned int MAX_INPUT_ATTEMPTS{ 3 };

// Maximum characters allowed in input string.
const size_t MAX_INPUT_SIZE{ 16 };

// Capture user string input. 
void GetString(const std::string prompt, std::string &input);

// Enable user to input menu choice.
Choice EnterMenuChoice();

// Individual type validations.
template<typename T>
inline const bool validateNumber(T& t, const std::string s)
{
	return false; // Fail for types other than int, long & double (see below).
}

// Handle integer type.
template<>
inline const bool validateNumber<int>(int& i, const std::string s)
{
	std::string::size_type pos = 0;

	i = stoi(s, &pos);
	if (pos != s.length())
		throw std::runtime_error("Trailing characters");

	return true;
}

// Handle unsigned integer type.
template<>
inline const bool validateNumber<unsigned>(unsigned& i, const std::string s)
{
	std::string::size_type pos = 0;

	i = stoi(s, &pos);
	if (pos != s.length())
		throw std::runtime_error("Trailing characters");

	return true;
}

// Handle long type.
template<>
inline const bool validateNumber<long>(long& l, const std::string s)
{
	std::string::size_type pos = 0;

	l = stol(s, &pos);
	if (pos != s.length())
		throw std::runtime_error("Trailing characters");

	return true;
}

// Handle double type.
template<>
inline const bool validateNumber<double>(double& d, const std::string s)
{
	std::string::size_type pos = 0;

	d = stod(s, &pos);
	if (pos != s.length())
		throw std::runtime_error("Trailing characters");

	return true;
}

// Validate string characters and attempt conversion.
template<typename T>
static const bool isNumber(T& n, std::string& s)
{
	static_assert(std::is_arithmetic<T>::value, "T must be numeric");

	// Remove trailing ws, leading ws is ignored by stoX functions.
	size_t p;
	if ((p = s.find_last_not_of(" \f\n\r\t\v")) != std::string::npos)
		s.erase(p + 1);

	if (!s.empty())
	{
		std::string validChars(" +-1234567890");

		if (!std::numeric_limits<T>::is_integer)
			// Add decimal point for floats.
			validChars += ".";

		// Throw exception if invalid chars found.
		if (s.find_first_not_of(validChars) != std::string::npos)
			throw std::runtime_error("Invalid characters");

		// Attempt to validate number by type.
		if (validateNumber<T>(n, s))
			return true;
	}

	return false;
}

// Input loop.
template<
	typename T,
	typename = typename std::enable_if_t<std::is_arithmetic<T>::value, T>>
	const bool getNumber(const std::string prompt, T& n, const T min, const T max)
{
	unsigned int attempts = MAX_INPUT_ATTEMPTS;

	while (attempts--)
	{
		// Temporary holds input.
		std::string buffer;

		// Prompt and get input.
		if (prompt.empty())
			std::cout << "Enter a number: ";
		else
			std::cout << prompt;
		std::getline(cin, buffer, '\n');

		try
		{
			// Limit buffer length to reasonable amount.
			if (buffer.size() > MAX_INPUT_SIZE)
				throw std::length_error("Exceeded max_input_size");

			if (isNumber<T>(n, buffer)) {
				// Check if value between min/max.
				if (n >= min && n <= max)
					return true;
				else
					throw std::out_of_range("Outside min/max");
			}
		}
		catch (std::out_of_range&)
		{
			std::cout << "Input outside range (" << std::fixed << std::setprecision(1)
				<< min << " to " << max << ")\n";
		}
		catch (std::length_error&)
		{
			std::cout << "Too many characters input\n";
		}
		catch (...)
		{
			// all other exceptions caught here.
			std::cout << "Invalid input: " << buffer << std::endl;
		}
	}

	return false;
}

#endif
