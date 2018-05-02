/*************************************************************************
* Title: DAO Database Access EXample
* File: CIS278_Week16_DAO.cpp
* Author: James Eli
* Date: 3/01/2018
*
* Direct Data Access Object (Direct DAO) example.
*
* Notes:
*  (1) Requires MS Access 2007 driver, located here:
*      https://www.microsoft.com/en-us/download/details.aspx?id=23734
*  (2) Note/update location of ACEDAO.dll and sample student.accdb files.
*  (3) Compiled with MS Visual Studio 2017 Community (v141).
*************************************************************************
* Change Log:
*   03/01/2018: Initial release. JME
*************************************************************************/
#include <iostream>
#include <comutil.h>

using namespace std;

// Windows 10 location of ACEDAO.dll on my computer.
#import <C:\\Program Files (x86)\\Common Files\\microsoft shared\\OFFICE12\\ACEDAO.dll> rename( "EOF", "AdoNSEOF" )

int main()
{
	// Hardcoded location of the database.
	_bstr_t bstrConnect = "C:\\Users\\Jim\\source\\repos\\CIS278_Week16_DAO\\CIS278_Week16_DAO\\student.accdb";

	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr))
	{
		cout << ": Failed to CoInitialize COM." << endl;
		return hr;
	}

	// Create an instance of the engine.
	DAO::_DBEngine* pEngine = NULL;
	// The CoCreateInstance helper function provides a convenient shortcut by connecting 
	// to the class object associated with the specified CLSID, creating an 
	// uninitialized instance, and releasing the class object. 
	hr = CoCreateInstance(__uuidof(DAO::DBEngine), NULL, CLSCTX_ALL, IID_IDispatch, (LPVOID*)&pEngine);
	if (SUCCEEDED(hr) && pEngine)
	{
		// COM errors are handled by the C++ try/catch block.
		try
		{
			DAO::DatabasePtr pDbPtr = NULL;
			pDbPtr = pEngine->OpenDatabase(bstrConnect);
			if (pDbPtr)
			{
				cout << "Successfully connected to database. Data source name:\n  " << pDbPtr->GetName() << endl;

				// Prepare SQL query.
				_bstr_t query = "select * from STUDENT order by ID";
				cout << "SQL query:\n  " << query << endl;

				// Excecute the query and create a record set.
				DAO::RecordsetPtr pRS = NULL;
				pRS = pDbPtr->OpenRecordset(query, _variant_t(DAO::dbOpenDynaset));
				if (pRS && 0 < pRS->RecordCount)
				{
					cout << "Retrieve schema info for the given result set: " << endl;

					DAO::FieldsPtr pFields = NULL;
					pFields = pRS->GetFields();
					if (pFields && pFields->Count > 0)
					{
						for (short nIndex = 0; nIndex < pFields->Count; nIndex++)
							cout << " | " << pFields->GetItem(nIndex)->GetName();
						cout << endl;
					}
					else
						cout << "Error: Number of fields in the result set is 0." << endl;

					cout << "Fetch the actual data: " << endl;
					// Loop through the rows in the result set.
					while (!pRS->AdoNSEOF)
					{
						for (short nIndex = 0; nIndex < pFields->Count; nIndex++)
							cout << " | " << _bstr_t(pFields->GetItem(nIndex)->GetValue());
						cout << endl;
						pRS->MoveNext();
					}
					cout << "Total Row Count: " << pRS->RecordCount << endl;
				}

				// Close record set and database.
				pRS->Close();
				pDbPtr->Close();
				pDbPtr = NULL;
			}
			else
				cout << "Unable to connect to data source: " << bstrConnect << endl;
		}
		catch (_com_error& e)
		{
			cout << "_com_error: " << e.Description() << endl;
		}

		pEngine->Release();
		pEngine = NULL;
		cout << "Cleanup. Done." << endl;
	}
	else
		cout << ": Cannot instantiate DBEngine object." << endl;

	return 0;
}
