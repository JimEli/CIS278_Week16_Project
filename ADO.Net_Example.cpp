/*************************************************************************
* Title: CIS278 Week16 C++/CLI ADO.Net OleDb Example
* File: main.cpp
* Author: James Eli
* Date: 5/3/2018
*
* Notes:
*  (1) Place the student.accdb file inside the project folder.
*  (2) Add to project properties>General:
*       Common Langauge Runtime Support: (/clr)
*       .NET Target Framework Version: v4.5
*  (3) In the Solution Explorer>References Add:
*       System
*       System.Data
*  (4) Compiled with MS Visual Studio 2017 Community (v141).
*  (5) Additional information can be found here:
*      https://www.codeproject.com/Articles/8477/Using-ADO-NET-for-beginners
*************************************************************************
* Change Log:
*   05/03/2018: Initial release. JME
*************************************************************************/
#include <iostream>
#include <string>
#include <msclr\marshal_cppstd.h>

using namespace std;

// ADO.NET definitions.
#using <System.dll>
#using <System.Data.dll>
using namespace System::Data;
using namespace System::Data::Common;
using namespace msclr::interop;

int main()
{
	// Get the factory object for OleDb provider type.
	DbProviderFactory^ fac = DbProviderFactories::GetFactory("System.Data.OleDb");
	DbConnection^ conn = nullptr;

	try
	{
		// Open db connection.
		conn = fac->CreateConnection();
		conn->ConnectionString = "Provider=Microsoft.ACE.OLEDB.12.0;Data Source=student.accdb";
		conn->Open();
		cout << "Connection opened.\n";

		// Create and populate a command.
		DbCommand^ cmd = fac->CreateCommand();
		cmd->CommandText = "SELECT * FROM student ORDER BY id";
		cmd->CommandType = CommandType::Text;
		cmd->Connection = conn;

		// Using a data adapter.

		// Create a DataAdapter.
		DbDataAdapter^ adapter = fac->CreateDataAdapter();
		// Use the command with the adapter.
		adapter->SelectCommand = cmd;
		
		// Create and fill a DataSet.
		DataSet ^ds = gcnew DataSet("student");
		adapter->Fill(ds, "studs");

		//DataRowCollection^ rc;
		//rc = table->Rows;
		//System::Console::WriteLine("Row count: {0}", rc->Count);

		// Dump the schema.
		for (int c = 0; c < ds->Tables["studs"]->Columns->Count; c++)
			System::Console::Write("{0} ", ds->Tables["studs"]->Columns[c]->ColumnName->ToString());
		std::cout << std::endl;

		// Dump the data.
		for (int r = 0; r < ds->Tables[0]->Rows->Count; r++) {
			DataRow^ row = ds->Tables[0]->Rows[r];
			for (int c = 0; c < ds->Tables["studs"]->Columns->Count; c++)
				System::Console::Write("{0} ", row[c]->ToString());
			std::cout << std::endl;

		}
/*
		// Using a data reader.
		
		// Count the entries.
		DbCommand^ cmd = fac->CreateCommand();
		cmd->CommandText = "SELECT COUNT(*) FROM student";
		cmd->CommandType = CommandType::Text;
		cmd->Connection = conn;
		int numberOfEntries = (int)cmd->ExecuteScalar();
		cout << "Number of entries is " << numberOfEntries << endl;

		// Insert a new entry.
		//cmd->CommandText = "INSERT INTO student VALUES ( 10, 'Donald', 'Trump', 'President', '2020')";
		//int rowsAffected = cmd->ExecuteNonQuery();
		//cout << "Added " << rowsAffected << " rows.\n";
		// Count the entries.
		//cmd->CommandText = "SELECT COUNT(*) FROM student";
		//numberOfEntries = (int)cmd->ExecuteScalar();
		//cout << "Number of entries is " << numberOfEntries << endl;

		// Get the entries.
		cmd->CommandText = "SELECT * FROM student ORDER BY id";
		DbDataReader^ reader = cmd->ExecuteReader();
		while (reader->Read())
			//cout << (int)reader->GetInt32(0) << ": " << marshal_as<string>(reader->GetString(1)) << " " << marshal_as<string>(reader->GetString(2)) << endl;
			System::Console::WriteLine("{0}: {1} {2}", reader->GetInt32(0).ToString(), reader->GetString(1), reader->GetString(2));
			reader->Close();
*/
	}
	
	catch (System::Exception^ ex)
	{
		System::Console::WriteLine("Error: {0}", ex->Message);
	}
	
	finally
	{
		if (conn != nullptr)
			conn->Close();
		cout << "Connection closed.\n";
	}

	return 0;
}
