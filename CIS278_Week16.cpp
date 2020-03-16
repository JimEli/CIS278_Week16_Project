/*************************************************************************
* Title: Database Project
* File: CIS278_Week16.cpp
* Author: James Eli
* Date: 3/01/2018
*
* Develop a C++ program to connect to a MS Access database
* ("Student.accdb" – provided on D2L) and use SQL commands to manipulate
* its records.
* The user will be given a menu to choose from the following options:
*   1 - Press (S) to select and display all students records
*   2 - Press (D) to delete a student by ID#
*   3 - Press (U) to update a student's graduation year
*   4 - Press (I) to insert a new student
*   5 - Press (Q) to quit
* For user option #1, first display the header and then display every record
* on a separate line. See example below:
* Student ID  First Name Last Name  Major                   Graduation Year
* 1           Joe        Doe        Computer Science        2018
* 2           Emma       Smith      Electrical Engineering  2019
* 3           Juan       Perez      Marketing               2019
* For user option #2, confirm back to the user whether the deletion was
* successful or the ID provided was not found in the students records.
* For user option #3, prompt the user to enter a student ID, validate if
* the student ID exists in the database. If it does not exist, then display
* a message to user that the student ID does not exist. If it exists, then
* prompt the user to enter the new graduation year. The new graduation year
* should be no later than 4 years from the current year. For example, if
* the user enters “2035”, you should display a message to the user saying
* invalid graduation year. Then prompt again to enter a valid year.
* For user option #4, prompt the user to enter the first name, last name,
* major, and graduation year. Note that the student ID will automatically
* be generated. DO NOT PROMPT FOR STUDENT ID. Graduation year rules are the
* same as described in user option #3 above.
* After every operation completion, you need to re-display the menu until
* the user selects option #5 to quit the program.
*
* Technical Tips
* C++ language does not have a standard library to connect to and interact
* with a database management system. Such application is domain and vendor
* specific. Every major database management system in the market (MS Access,
* SQL Server, Oracle, etc.) has its own libraries and interfaces. Developers
* rely on such interfaces as well as the documentation supplied by the
* vendor when developing applications that interact with a database on the
* back-end.
* For this project, the database is MS Access. Over the years, Microsoft has
* developed various database communication protocols that were provided in
* libraries and interfaces (i.e. Dynamic Link Libraries). It is highly
* recommended that you visit and read the contents of this site
* https://msdn.microsoft.com/en-us/library/cc811599(v=office.12).aspx
* before you start working on this project. It is also recommended that you
* use Direct Data Access Object (Direct DAO) for this project.
*
* Notes:
*  (1) Requires MS Access 2007 driver, located here: 
*      https://www.microsoft.com/en-us/download/details.aspx?id=23734
*  (2) Uses the archaic MS ActiveX Data Object (ADO) API.
*  (3) Initally wanted to bind DB recordset to a C++ class, but found that 
*      difficult, inefficient and lacking good documentation or examples. 
*      However, without it the program is much simplified.
*  (4) Program load/init time is excessive, I couldn't determine the cause.
*  (5) Needs more input validation.
*  (6) This code is a simple demonstration of basic database access 
*      techniques. As such, it is vulnerable to SQL Injection. Please alter 
*      any derivative code to protect against injection attacks.
*  (7) Compiled with MS Visual Studio 2017 Community (v141).
*  (8) Submitted in partial fulfillment of the requirements of PCC CIS-278.
*************************************************************************
* Change Log:
*   03/01/2018: Initial release. JME
*************************************************************************/
#include <iostream> // console input/output
#include <iomanip>  // setprecision
#include <string>   // string handling
#include <limits>   // numeric limits
#include <memory>   // unique pointers

// Undef C max MACRO, otherwise C++ version not used. 
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Visual Leak Detector.
#include "C:\Program Files (x86)\Visual Leak Detector\include\vld.h"

#include "database.h"
#include "input.h"

using namespace std;

// Constant data, acceptable year input.
static const int CURRENT_YEAR{ 2018 };
static const int MAX_GRAD_YEAR{ CURRENT_YEAR + 4 };

// Run SQL query.
bool RunQuery(Database& db, Table& tb, const string s)
{
	// Attmept to run query.
	if (db.OpenTable(ADODB::adCmdText, s.c_str(), tb))
		return true;

	return false;
}

// Get highest (last) id number in db.
int GetHighestId(Database db, Table tb)
{
	// Attempt to run sql query grabbing all students records.
	if (!RunQuery(db, tb,"select * from STUDENT order by ID"))
		return 0;

	// Set pointer to end of recordset.
	tb.MoveLast();

	// Get the id.
	int id{ 0 };
	tb.Get("ID", id);

	return id;
}

// Check ids (if student deleted, id will not exist).
bool CheckResult(Database db, Table tb, const int id)
{
	// Attempt to run sql query grabbing all students records.
	string s = "select * from STUDENT where ID=" + to_string(id);
	if (!RunQuery(db, tb, s))
		return false;

	// Grab id field of returned record.
	int dbId{ 0 };
	if (!tb.Get("ID", dbId))
		return false;

	// Return results.
	return (id == dbId);
}

// Select and display all students.
void Select(Database db, Table tb)
{
	// Attempt to run sql query grabbing all students records.
	if (!RunQuery(db, tb, "select * from STUDENT order by ID"))
		return;

	// Set pointer to start of recordset.
	if (!tb.IsEOF())
		tb.MoveFirst();

	// Display header.
	cout << "ID  First Name Last Name Major" + string(19, ' ') + "Grad Year\n";

	// Loop through entire table of students.
	while (!tb.IsEOF())
	{
		// Alloc temp storage for student record data.
		// Max char field is 255 chars + null terminator.
		auto temp = unique_ptr<char[]>{ new char[256] };
		int id{ 0 };

		// Grab fields for each record.
		if (tb.Get("ID", id))
			cout << left << setw(4) << id;
		else
			break;
		if (tb.Get("FirstName", temp.get(), 255))
			cout << setw(11) << temp.get();
		if (tb.Get("LastName", temp.get(), 255))
			cout << setw(10) << temp.get();
		if (tb.Get("Major", temp.get(), 255))
			cout << setw(24) << temp.get();
		if (tb.Get("Graduation Year", temp.get(), 255))
			cout << temp.get();
		cout << endl;

		// Advance to next record.
		tb.MoveNext();
	}
}

// Delete student.
void Remove(Database db, Table tb)
{
	int id{ 0 }, maxId{ GetHighestId(db, tb) };

	// Get student id to delete.
	if (getNumber<int>("Enter student ID # to delete: ", id, 1, maxId))
	{
		// Construct a query string & convert to C style string.
		string s = "delete from STUDENT where ID=" + to_string(id);
		
		// Delete record.
		if (RunQuery(db, tb, s) && !CheckResult(db, tb, id))
			cout << "Record #" << to_string(id) << " has been deleted." << endl;
	}
}

// Insert new student.
void Insert(Database db, Table tb)
{
	int year{ 0 }, id{ GetHighestId(db, tb) + 1 };
	string fName{ "" }, lName{ "" }, major{ "" };
	
	// Get new student data.
	cout << "Enter new student information.\n";
	GetString("Enter first name: ", fName);
	GetString("Enter last name: ", lName);
	GetString("Enter major: ", major);
	if (getNumber<int>("Enter graduation year: ", year, CURRENT_YEAR, MAX_GRAD_YEAR))
	{
		// Construct a query string & convert to C style string.
		string s = "insert into STUDENT values( " + to_string(id) + ", "
		"'" + fName + "', " + "'" + lName + "', " + "'" + major + "', " + to_string(year) + " )";
		
		// Insert new student into db, and increment record count.
		if (RunQuery(db, tb, s) && CheckResult(db, tb, id))
			cout << "Record #" << to_string(id) << " is in database." << endl;
	}
}

// Update the graduation year field of a specified record.
void Update(Database db, Table tb)
{
	int id{ 0 }, year{ 0 }, maxId{ GetHighestId(db, tb) };

	// get a student id and new graduation year.
	if (getNumber<int>("Enter student ID # to update: ", id, 1, maxId) &&
		getNumber<int>("Enter new graduation year: ", year, CURRENT_YEAR, MAX_GRAD_YEAR))
	{ 
		// Construct a query string & convert to C style string.
		string s = "update STUDENT set [Graduation Year]='" + to_string(year) + "' where ID=" + to_string(id);

		// insert into db.
		if (RunQuery(db, tb, s))
			cout << "Update of record #" << to_string(id) << " completed." << endl;
	}
}

// Contains main program loop.
int main()
{
	try 
	{
		Database db;
		Table table;

		// Initalize COM.
		if ( FAILED( CoInitialize(NULL) ) )
			exit(EXIT_FAILURE);

		// Attempt to open db (no user id & pwd) using MS ACE OLEDB driver to access db.
		db.Open("", "", "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=student.accdb");

		// Loop sentinel.
		bool done = false;

		// Enable user to specify action.
		while (!done)
		{
			switch (EnterMenuChoice())
			{
			case Choice::SELECT:
			case Choice::_1:
				// Display all students.
				Select(db, table);
				break;

			case Choice::REMOVE:
			case Choice::_2:
				// Delete a student.
				Remove(db, table);
				break;

			case Choice::UPDATE:
			case Choice::_3:
				// Update a record.
				Update(db, table);
				break;

			case Choice::INSERT:
			case Choice::_4:
				// Insert new record.
				Insert(db, table);
				break;

			case Choice::QUIT:
			case Choice::_5:
				// Quit.
				done = true;
				db.Close();
				CoUninitialize();
				break;

			case Choice::INVALID:
			default:
				// Display error if user does not select valid choice.
				cerr << "Incorrect choice\n";
				break;
			}
		}
	}
	catch (dbException)
	{
		CoUninitialize();
	}

	return 0;
}
