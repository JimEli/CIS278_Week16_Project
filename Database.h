/*************************************************************************
* Title: CIS-278 Week #16 Database Project
* File: database.h
* Author: James Eli
* Date: 3/01/2018
*
* Database and table classes that encapsulate the ADO API C++ interface 
* to MS Access DB. This is "bare-metal" style, as ADO support in MSVS 2017 
* is somewhat deprecated, it's inclusion is primarily to support legacy 
* code. These classes incorporate only the most basic functionality needed 
* to complete the course project.
*
* Notes:
*  (1) ADO API is included via the #import directive (see below).
*  (2) COM must be initialized before using this class.
*  (3) Realize all below is optimized for this particular implementaion.
*  (4) Compiled with MS Visual Studio 2017 Community (v141).
*  (5) Submitted in partial fulfillment of the requirements of PCC CIS-278.
*************************************************************************
* Change Log:
*   03/01/2018: Initial release. JME
*************************************************************************/
#ifndef _DATABASE_TABLE_H_
#define _DATABASE_TABLE_H_

// Import ADO class information from msado15.dll.
#import "c:\Program Files\Common Files\system\ADO\msado15.dll" rename("EOF", "EndOfFile")

// A dervired exception class for our DB access failures.
class dbException : public std::exception
{
public:
	virtual char const * what() const { return "ADODB class exception"; }
};

// ADO DB connection and recordset pointers.
typedef ADODB::_RecordsetPtr RecPtr;
typedef ADODB::_ConnectionPtr ConnPtr; 
// Instantiate our classes.
class Database;
class Table;

// Minimal functionality database class. 
class Database
{
public:
	Database();
	bool Open(const char*, const char*, const char*);
	void Close();
	bool OpenTable(const int, const char*, Table&);
private:
	// Pointer to db connection object.
	ConnPtr mConn;
};

// Minimal functionality DB table class. 
class Table {
public:
	Table();
	void SetRec(RecPtr);
	int IsEOF();
	long MoveNext();
	long MoveFirst();
	long MoveLast();
	bool Get(char*, char*, const size_t);
	bool Get(char*, int&);
	bool Get(char*, double&);
private:
	// Pointer to recordset object.
	RecPtr mRec;
};

#endif
