/*************************************************************************
* Title: ADO MS Access DB Example
* File: ADO_Example.cpp
* Author: James Eli
* Date: 3/01/2018
*
* Note, ADO support in MSVS 2017 is somewhat deprecated, it's inclusion 
* is primarily to support legacy code.
*************************************************************************
* Change Log:
*   03/01/2018: Initial release. JME
*************************************************************************/
#include <iostream> // console input/output
#include <string>   // string handling
#include <memory>   // unique pointers

// Import ADO class information from msado15.dll.
#import "c:\Program Files\Common Files\system\ADO\msado15.dll" rename("EOF", "EndOfFile")

int main()
{
	// Pointer to db connection object.
	ADODB::_ConnectionPtr mConn = nullptr;
	// Pointer to recordset object.
	ADODB::_RecordsetPtr mRec = nullptr;

	try
	{
		// Initalize COM.
		if (CoInitialize(NULL))
			exit(EXIT_FAILURE);

		// Attempt to open db (no user id & pwd) using MS ACE OLEDB driver to access db.
		long result = mConn.CreateInstance(__uuidof(ADODB::Connection));
		mConn->Open("Provider=Microsoft.ACE.OLEDB.12.0;Data Source=student.accdb", "", "", NULL);
		if (mConn != NULL)
		{
			// Display all students.
			mRec.CreateInstance(__uuidof(ADODB::Recordset));
			mRec->Open("select * from STUDENT", _variant_t((IDispatch *)mConn, true),
				ADODB::adOpenStatic, ADODB::adLockOptimistic, ADODB::adCmdText);
			if (mRec != NULL)
			{
				// Loop through entire table of students.
				while (!mRec->EndOfFile)
				{
					// Alloc temp storage for student record data (255 chars + null terminator).
					auto temp = std::unique_ptr<char[]>{ new char[256] };
					_variant_t vtValue = mRec->Fields->GetItem("FirstName")->GetValue();
					strcpy_s(temp.get(), 255, (_bstr_t)(vtValue.bstrVal));
					std::cout << temp.get();
					vtValue = mRec->Fields->GetItem("LastName")->GetValue();
					strcpy_s(temp.get(), 255, (_bstr_t)(vtValue.bstrVal));
					std::cout << " " << temp.get() << std::endl;

					// Advance to next record.
					mRec->MoveNext();
				}
			}
			//close connection.
			if (mConn)
				mConn->Close();
		}
		else
			std::cout << "Could not open db.\n";
	}
	catch (...)
	{
		std::cout << "Caught error.\n";
	}

	CoUninitialize();
	return 0;
}
