/*************************************************************************
* Title: CIS-278 Week #16 Database Project
* File: database.cpp
* Author: James Eli
* Date: 3/01/2018
*
* Database and table classes to provide "bare-metal" C++ interfacing to
* MS Access DB via the ADO API. See header file for more information
*************************************************************************
* Change Log:
*   03/01/2018: Initial release. JME
*************************************************************************/
#include <cstdio>
#include <iostream>
#include <string>
#include <comdef.h>

#include "Database.h"

// Error reporting for db & table classes.
void ReportError(_com_error &e)
{
	std::cerr << "Error Code: " << e.Error()
		<< ", meaning: " << e.ErrorMessage()
		<< "\nSource: " << e.Source()
		<< "\nDescription: " << e.Description() << std::endl;
}

// Constructor.
Database::Database() { mConn = nullptr; }

// Attempt to close db.
void Database::Close()
{
	try
	{
		//close connection, if not yet closed
		if (mConn)
		{
			mConn->Close();
			mConn = nullptr;
		}
	}
	catch (_com_error &e)
	{
		ReportError(e);
	}
}

// Attempt to open db.
bool Database::Open(const char* userName, const char* pwd, const char* connStr)
{
	try
	{
		long result = mConn.CreateInstance(__uuidof(ADODB::Connection));
		mConn->Open(connStr, userName, pwd, NULL);
	}
	catch (_com_error &e)
	{
		ReportError(e);
		mConn = nullptr;
		throw dbException();
	}

	return true;
}

// Attempt to open a db table.
bool Database::OpenTable(const int mode, const char *command, Table& table)
{
	if (mConn == NULL)
	{
		table.SetRec(nullptr);
		std::cerr << "Invalid Connection\n";
		return false;
	}

	RecPtr tRec = nullptr;

	try
	{
		tRec.CreateInstance(__uuidof(ADODB::Recordset));
		tRec->Open(command, _variant_t((IDispatch *)mConn, true),
			ADODB::adOpenStatic, ADODB::adLockOptimistic, mode);
	}
	catch (_com_error &e)
	{
		ReportError(e);
		table.SetRec(nullptr);
		throw dbException();
	}

	table.SetRec(tRec);
	return true;
}

// Constructor.
Table::Table() { mRec = nullptr; }

// Setter for active table pointer.
void Table::SetRec(RecPtr p) { mRec = p; }

// Table recordset pointer at EOF?
int Table::IsEOF()
{
	int rs;

	if (mRec == NULL)
	{
		std::cerr << "Invalid Record\n";
		return -1;
	}
	
	try
	{
		rs = mRec->EndOfFile;
	}
	catch(_com_error &e) 
	{
		ReportError(e);
		mRec = nullptr;
		return -2;
	}

	return rs;
}

// Table get character type field data.
bool Table::Get(char* FieldName, char* FieldValue, const size_t sz)
{
	try
	{
		// Attempt convert from variant to c string.
		_variant_t  vtValue;

		vtValue = mRec->Fields->GetItem(FieldName)->GetValue();
		strcpy_s(FieldValue, sz, (_bstr_t)(vtValue.bstrVal));
	}
	catch (_com_error &e)
	{
		ReportError(e);
		std::cerr << "\n For Field: " << FieldName << std::endl;
		return false;
	}

	return true;
}

// Table get integer type field data.
bool Table::Get(char* FieldName, int& FieldValue)
{
	try
	{
		// Attempt convert from variant to int.
		_variant_t  vtValue;

		vtValue = mRec->Fields->GetItem(FieldName)->GetValue();
		FieldValue = vtValue.intVal;
	}
	catch (_com_error &e)
	{
		ReportError(e);
		std::cerr << "\n For Field: " << FieldName << std::endl;
		return false;
	}

	return true;
}

// Table get double type field data (not used in this implementation).
bool Table::Get(char* FieldName, double& FieldValue)
{
	try
	{
		// Attempt convert from variant to double.
		_variant_t vtValue;

		vtValue = mRec->Fields->GetItem(FieldName)->GetValue();
		FieldValue = vtValue.dblVal; 
	}
	catch (_com_error &e)
	{
		ReportError(e);
		std::cerr << "\n For Field: " << FieldName << std::endl;
		return false;
	}

	return true;
}

// Move table recordset pointer to next record.
long Table::MoveNext()
{
	long result;

	try
	{
		result = mRec->MoveNext();
	}
	catch(_com_error &e)
	{
		ReportError(e);
		//mRec = NULL;
		return -2;
	}

	return result;
}

// Move table recordset pointer to first record.
long Table::MoveFirst()
{
	long result;

	try
	{
		result = mRec->MoveFirst();
	}
	catch(_com_error &e)
	{
		ReportError(e);
		//mRec = NULL;
		return -2;
	}
	
	return result;
}

// Move table recordset pointer to last record.
long Table::MoveLast()
{
	long result;

	try
	{
		result = mRec->MoveLast();
	}
	catch(_com_error &e)
	{
		ReportError(e);
		//mRec = NULL;
		return -2;
	}
	
	return result;
}
