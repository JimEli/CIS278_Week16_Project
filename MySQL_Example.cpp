/*************************************************************************
* Title: MySQL Example Project
* File: mysql_example.cpp
* Author: James Eli
* Date: 3/16/2020
*
* Note: This code is a simple demonstration of basic database access
* techniques. As such, it is vulnerable to SQL Injection. Please alter
* any derivative code to protect against injection attacks.
*
* MySQL Query to create sample database:
*
* create database students;
* use students;
* DROP TABLE IF EXISTS `STUDENT`;
* CREATE TABLE `STUDENT` (
*   `ID` tinyint(4) DEFAULT NULL,
*   `FirstName` varchar(6) DEFAULT NULL,
*   `LastName` varchar(8) DEFAULT NULL,
*   `Major` varchar(22) DEFAULT NULL,
*   `Graduation Year` smallint(6) DEFAULT NULL
* ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
* -- Dumping data for table `STUDENT`
* LOCK TABLES `STUDENT` WRITE;
* INSERT INTO `STUDENT` VALUES 
(1,'Joe','Doe','Computer Science',2018),
(2,'Emma','Smith','Electrical Engineering',2019),
(3,'Juan','Perez','Marketing',2019),
(4,'Tom','Lee','Accounting',2020),
(5,'Ella','Fenda','Finance',2019),
(6,'Oliver','Torres','Business',2018),
(7,'Lea','Martinez','Communication',2020),
(8,'Jim','Eli','Basket Weaving',2020);
* UNLOCK TABLES;
*
* DROP PROCEDURE IF EXISTS Student.delete_student;
* DELIMITER $$
* CREATE PROCEDURE delete_student(IN ident TINYINT)
* BEGIN
*   DELETE FROM student WHERE id=ident
* END $$
* DELIMITER ;
*
*************************************************************************
* Change Log:
*   3/16/2020: Initial release. JME
*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include "input.h"

#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>

// Constant data, acceptable year input.
static const int CURRENT_YEAR{ 2018 };
static const int MAX_GRAD_YEAR{ CURRENT_YEAR + 4 };

// Select and display all students in db table.
void Select(std::shared_ptr<sql::Connection> con)
{
    std::unique_ptr<sql::Statement> stmt(con->createStatement());
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM student ORDER BY id"));

    // Display header.
    std::cout << "ID  First Name Last Name Major" + std::string(19, ' ') + "Grad Year\n";
    
    std::ios oldState(nullptr);
    oldState.copyfmt(std::cout);
    while (res->next())
    {
        std::cout << std::left << std::setw(4) << res->getInt(1);
        std::cout << std::setw(11) << res->getString(2);
        std::cout << std::setw(10) << res->getString(3);
        std::cout << std::setw(24) << res->getString(4);
        std::cout << res->getInt(5) << std::endl;
    }
    std::cout.copyfmt(oldState);
}

int getEmptyId(std::shared_ptr<sql::Connection> con)
{
    size_t min{ 0 };

    // Execute SQL statement.
    std::unique_ptr<sql::Statement> stmt(con->createStatement());
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM student ORDER BY id"));
    int i = 0;

    // Check for highest & first missing id.
    while (res->next())
    {
        int n = res->getInt(1);

        if (min == 0 && ++i != n)
            min = n - 1;
    }

    if (min == 0)
        min = res->rowsCount() + 1;

    return static_cast<int>(min);
}

int getMaxId(std::shared_ptr<sql::Connection> con)
{
    // Execute SQL statement.
    std::unique_ptr<sql::Statement> stmt(con->createStatement());
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT * FROM student ORDER BY id"));

    return static_cast<int>(res->rowsCount());
}

// Delete student from db table.
void Remove(std::shared_ptr<sql::Connection> con)
{
    int id{ 0 }, maxId{ getMaxId(con) };

    // Get student id to delete.
    if (getNumber<int>("Enter student ID # to delete: ", id, 1, maxId))
    {
        // Call a stored procedure.
        std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement("CALL delete_student(?)"));
        pstmt->setInt(1, id);
        pstmt->execute();
    }
}

// Update the graduation year field of a specified record.
void Update(std::shared_ptr<sql::Connection> con)
{
    int id{ 0 };
    int year{ 0 };
    int maxId{ getMaxId(con) };

    // get a student id and new graduation year.
    if (getNumber<int>("Enter student ID # to update: ", id, 1, maxId) &&
        getNumber<int>("Enter new graduation year: ", year, CURRENT_YEAR, MAX_GRAD_YEAR))
    {
        std::unique_ptr<sql::PreparedStatement> prep_stmt(con->prepareStatement("UPDATE student SET `Graduation Year`=? WHERE id=?"));
        prep_stmt->setInt(1, year);
        prep_stmt->setInt(2, id);
        prep_stmt->execute();
    }
}

// Insert new student into db table.
void Insert(std::shared_ptr<sql::Connection> con)
{
    int year{ 0 };
    int id{ getEmptyId(con) };
    std::string fName{ "" }, lName{ "" }, major{ "" };

    // Get new student data.
    std::cout << "Enter new student information.\n";
    GetString("Enter first name: ", fName);
    GetString("Enter last name: ", lName);
    GetString("Enter major: ", major);

    if (getNumber<int>("Enter graduation year: ", year, CURRENT_YEAR, MAX_GRAD_YEAR))
    {
        // Construct the query string.
        std::string s = "INSERT INTO student VALUES( ?, ?, ?, ?, ? )";
        std::unique_ptr<sql::PreparedStatement> prep_stmt(con->prepareStatement(s));
        prep_stmt->setInt(1, id);
        prep_stmt->setString(2, fName);
        prep_stmt->setString(3, lName);
        prep_stmt->setString(4, major);
        prep_stmt->setInt(5, year);
        prep_stmt->execute();
    }
}

// Access information_schema to get db information.
void informationSchema(std::shared_ptr<sql::Connection> con)
{
    sql::Statement* stmt = con->createStatement();
    sql::ResultSet* rs = stmt->executeQuery("SELECT * FROM student");
    sql::ResultSetMetaData* res_meta = rs->getMetaData();

    int numcols = res_meta->getColumnCount();

    std::cout << "\nNumber of columns in the result set = " << numcols << std::endl;
    std::cout.width(17);
    std::cout << "Column Name/Label";
    std::cout.width(13);
    std::cout << "Column Type";
    std::cout.width(14);
    std::cout << "Column Size\n";

    for (int i = 0; i < numcols; ++i) 
    {
        std::cout.width(17);
        std::cout << res_meta->getColumnLabel(i + 1);
        std::cout.width(13);
        std::cout << res_meta->getColumnTypeName(i + 1);
        std::cout.width(13);
        std::cout << res_meta->getColumnDisplaySize(i + 1) << std::endl;
    }

    std::cout << "Column \"" << res_meta->getColumnLabel(1);
    std::cout << "\" belongs to the Table: \"" << res_meta->getTableName(1);
    std::cout << "\" which belongs to the Schema: \"" << res_meta->getSchemaName(1) << "\"" << std::endl;

    delete rs;
}

int main(void)
{
    try 
    {
        sql::Driver* driver;

        std::cout << "connecting to mysql server....";
        driver = get_driver_instance();
        std::shared_ptr<sql::Connection> con(driver->connect("tcp://127.0.0.1:3306", "admin", "password"));
        con->setSchema("students");
        std::cout << "connected" << std::endl;

        // Loop sentinel.
        bool done = false;

        // Enable user to specify action.
        while (!done)
        {
            switch (GetMenuChoice())
            {
            case Choice::OPTION_1:
            case Choice::SELECT:
                // Display all students.
                Select(con);
                break;

            case Choice::OPTION_2:
            case Choice::REMOVE:
                // Delete a student.
                Remove(con);
                break;

            case Choice::OPTION_3:
            case Choice::UPDATE:
                // Update a record.
                Update(con);
                break;

            case Choice::OPTION_4:
            case Choice::INSERT:
                // Insert new record.
                Insert(con);
                break;

            case Choice::OPTION_5:
            case Choice::QUIT:
                // Quit.
                done = true;
                break;

            case Choice::OPTION_6:
                // Information_Schema.
                informationSchema(con);
                break;

            case Choice::INVALID:
            default:
                // Display error if user does not select valid choice.
                std::cerr << "Incorrect choice\n";
                break;
            }
        }

    }
    catch (sql::SQLException & e)
    {
        std::cout << "SQL Exception: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
    catch (std::runtime_error & e)
    {
        std::cout << "Runtime_error in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "Error: " << e.what() << std::endl;
        std::cout << "not ok 1 - examples/exceptions.cpp" << std::endl;
    }

    return 0;
}
