/*************************************************************************
* Title: SQLite3 Example Project
* File: sqlite3_Example.cpp
* Author: James Eli
* Date: 3/16/2020
*
* Note: This code is a simple demonstration of basic database access 
* techniques. As such, it is vulnerable to SQL Injection. Please alter 
* any derivative code to protect against injection attacks.
*************************************************************************
* Change Log:
*   3/16/2020: Initial release. JME
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <memory>
#include <climits>
#include <vector>
#include "sqlite3.h"
#include "input.h"

// Constant data, acceptable year input.
static const int CURRENT_YEAR{ 2018 };
static const int MAX_GRAD_YEAR{ CURRENT_YEAR + 4 };

/*
static int selectCallback(void* data, int argc, char** argv, char** azColName)
{
    // Loop through entire table of students.
    for (int id{ 0 }; id < argc; id++)
    {
        // Grab fields for each record.
        if (!strcmp(azColName[id], "ID"))
            std::cout << std::left << std::setw(4) << argv[id];
        if (!strcmp(azColName[id], "FirstName"))
            std::cout << std::setw(11) << argv[id];
        if (!strcmp(azColName[id], "LastName"))
            std::cout << std::setw(10) << argv[id];
        if (!strcmp(azColName[id], "Major"))
            std::cout << std::setw(24) << argv[id];
        if (!strcmp(azColName[id], "Graduation Year"))
            std::cout << argv[id] << std::endl;
    }
    return 0;
}
*/

// Select and display all students in db table.
void Select(sqlite3* db)
{
/*
    char* zErrMsg = 0;

    // Attempt to run sql query grabbing all students records.
    const char* sql = "select * from STUDENT order by ID";

    // Display header.
    std::cout << "ID  First Name Last Name Major" + std::string(19, ' ') + "Grad Year\n";

    // Execute SQL statement.
    int rc = sqlite3_exec(db, sql, selectCallback, (void*)sql, &zErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);
    }
*/
    char** result = nullptr;
    int nrows, ncols;
    char* zErr = 0;

    // Execute SQL statement.

    int rc = sqlite3_get_table(db, (char*)"select * from STUDENT order by ID;", &result, &nrows, &ncols, &zErr);

    for (int i = 0; i <= nrows; i++)
    {
        std::cout << std::left << std::setw(4) << result[i * ncols];
        std::cout << std::setw(11) << result[i * ncols + 1];
        std::cout << std::setw(10) << result[i * ncols + 2];
        std::cout << std::setw(24) << result[i * ncols + 3];
        std::cout << result[i * ncols + 4] << std::endl;
    }

    // Free memory.
    sqlite3_free_table(result);
}

// Get highest/lowest id number in db.
std::pair<int, int> getMinMaxId(sqlite3* db)
{
    char** result = NULL;
    int nrows, ncols;
    char* zErr = 0;
    int min{ 0 }, max{ 0 };

    // Execute SQL statement.
    int rc = sqlite3_get_table(db, (char*)"select * from STUDENT order by ID;", &result, &nrows, &ncols, &zErr);

    std::vector<int> n;

    // Check for highest id.
    for (int i = 1; i <= nrows; i++)
    {
        int r = atoi(result[i * ncols]);
        n.push_back(r);

        if (r > max)
            max = r;
    }

    // Determine first missing number.
    for (int i = 1; i < n.size(); i++)
        if (std::find(n.begin(), n.end(), i) == n.end())
            min = i;
    if (min == 0)
        min = max + 1;

    // Free memory.
    sqlite3_free_table(result);

    return std::make_pair(min, max);
}

int getMinId(sqlite3* db) { return getMinMaxId(db).first; }
int getMaxId(sqlite3* db) { return getMinMaxId(db).second; }

// Delete student from db table.
void Remove(sqlite3* db)
{
    int id{ 0 }, maxId{ getMaxId(db) };

    // Get student id to delete.
    if (getNumber<int>("Enter student ID # to delete: ", id, 1, maxId)) 
    {
        // Construct a query string & convert to C style string.
        std::string s = "DELETE from STUDENT where ID='" + std::to_string(id) + "'";
        char* err = 0;

        int rc = sqlite3_exec(db, s.c_str(), NULL, NULL, &err);

        if (rc != SQLITE_OK) 
        {
            if (err != NULL) 
            {
                fprintf(stderr, "SQL error: %s\n", err);
                sqlite3_free(err);
            }
        }
        else
        {
            std::cout << "sqlite3_changes() = " << sqlite3_changes(db) << std::endl;
            std::cout << "Record #" << std::to_string(id) << " has been deleted." << std::endl;
        }
    }
}

// Update the graduation year field of a specified record.
void Update(sqlite3* db)
{
    int id{ 0 };
    int year{ 0 };
    int maxId{ getMaxId(db) };

    // get a student id and new graduation year.
    if (getNumber<int>("Enter student ID # to update: ", id, 1, maxId) &&
        getNumber<int>("Enter new graduation year: ", year, CURRENT_YEAR, MAX_GRAD_YEAR))
    {
        char* err = 0;

        // Construct a query string & convert to C style string.
        std::string s = "update STUDENT set [Graduation Year]='" + std::to_string(year) + "' where ID='" + std::to_string(id) + "'";

        int rc = sqlite3_exec(db, s.c_str(), NULL, NULL, &err);

        if (rc != SQLITE_OK)
        {
            if (err != NULL)
            {
                fprintf(stderr, "SQL error: %s\n", err);
                sqlite3_free(err);
            }
        }
        else
        {
            std::cout << "sqlite3_changes() = " << sqlite3_changes(db) << std::endl;
            std::cout << "Update of record #" << std::to_string(id) << " completed." << std::endl;
        }
    }
}

// Insert new student into db table.
void Insert(sqlite3* db)
{
    int year{ 0 };
    int id{ getMinId(db) };
    std::string fName{ "" }, lName{ "" }, major{ "" };

    // Get new student data.
    std::cout << "Enter new student information.\n";
    GetString("Enter first name: ", fName);
    GetString("Enter last name: ", lName);
    GetString("Enter major: ", major);
    if (getNumber<int>("Enter graduation year: ", year, CURRENT_YEAR, MAX_GRAD_YEAR))
    {
        char* err = 0;

        // Construct a query string.
        std::string s = "insert into STUDENT values( '" + std::to_string(id)
            + "', '" + fName  
            + "', '" + lName 
            + "', '" + major  
            + "', '" + std::to_string(year) + "' )";

        // Insert new student.
        int rc = sqlite3_exec(db, s.c_str(), NULL, NULL, &err);

        if (rc != SQLITE_OK)
        {
            if (err != NULL)
            {
                fprintf(stderr, "SQL error: %s\n", err);
                sqlite3_free(err);
            }
        }
        else
        {
            std::cout << "Record #" << std::to_string(id) << " is in database." << std::endl;
        }
    }
}

int main(void)
{
    std::shared_ptr<sqlite3> spDB(nullptr);
    sqlite3* db = NULL;
    spDB.reset(db, sqlite3_close);

    // Loop sentinel.
    bool done = false;

    // Attempt to open db (no user id & pwd).
    int rc = sqlite3_open("students_sqlite.db", &db);

    if (rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }
    else
        fprintf(stderr, "Opened database successfully\n");

    // Report sqlite version.
    std::cout << sqlite3_libversion() << std::endl;

    // Enable user to specify action.
    while (!done) 
    {
        switch (GetMenuChoice()) 
        {
        case Choice::OPTION_1:
        case Choice::SELECT:
            // Display all students.
            Select(db);
            break;
            
        case Choice::OPTION_2:
        case Choice::REMOVE:
            // Delete a student.
            Remove(db);
            break;
            
        case Choice::OPTION_3:
        case Choice::UPDATE:
            // Update a record.
            Update(db);
            break;
            
        case Choice::OPTION_4:
        case Choice::INSERT:
            // Insert new record.
            Insert(db);
            break;
            
        case Choice::OPTION_5:
        case Choice::QUIT:
            // Quit.
            done = true;
            sqlite3_close(db);
            break;

        case Choice::INVALID:
        default:
            // Display error if user does not select valid choice.
            std::cerr << "Incorrect choice\n";
            break;
        }
    }

    return 0;
}
