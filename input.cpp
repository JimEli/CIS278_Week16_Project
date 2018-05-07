/*************************************************************************
* Title: numeric, string and menu input
* File: input.cpp
* Author: James Eli
* Date: 12/29/2017
* See header file for comments.
*************************************************************************/
#ifndef _INPUT_VALIDATION_
#define _INPUT_VALIDATION_

#include "input.h"

// Capture user string input. 
void GetString(const std::string prompt, std::string &input)
{
	std::string temp("");

	do
	{
		std::cout << prompt;
		std::getline(std::cin, temp, '\n');
		std::cin.clear();
	} while (temp.size() > 22);

	// If empty, make no change.
	if (!temp.empty())
		input = temp;
}

// Enable user to input menu choice.
Choice EnterMenuChoice()
{
	char input;
	std::string const validChars{ "sduiq12345" };

	// Display available options.
	std::cout << "\nEnter your choice\n"
		<< "1 - Press (S) to select and display all students records\n"
		<< "2 - Press (D) to delete a student by ID #\n"
		<< "3 - Press (U) to update a student's graduation year\n"
		<< "4 - Press (I) to insert a new student\n"
		<< "5 - Press (Q) to quit\n";

	// Input menu selection from user.
	std::cin.get(input);

	if (std::cin.peek() != '\n')
		// More than a single char input.
		input = 0;

	// Eat any remaining chars and LF.
	std::cin.clear();

	// Undef C max MACRO before this line.
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	// Check valid menu choice and return.
	if (validChars.find_first_of(tolower(input)) != std::string::npos)
		return static_cast<Choice>(input);

	return Choice::INVALID;
}

#endif
