/*
create database students;
use students;
-- Table structure for table `STUDENT`
DROP TABLE IF EXISTS `STUDENT`;
CREATE TABLE `STUDENT` (
  `ID` tinyint(4) DEFAULT NULL,
  `FirstName` varchar(6) DEFAULT NULL,
  `LastName` varchar(8) DEFAULT NULL,
  `Major` varchar(22) DEFAULT NULL,
  `Graduation Year` smallint(6) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
-- Dumping data for table `STUDENT`
LOCK TABLES `STUDENT` WRITE;
INSERT INTO `STUDENT` VALUES (1,'Joe','Doe','Computer Science',2018),(2,'Emma','Smith','Electrical Engineering',2019),(3,'Juan','Perez','Marketing',2019),(4,'Tom','Lee','Accounting',2020),(5,'Ella','Fenda','Finance',2019),(6,'Oliver','Torres','Business',2018),(7,'Lea','Martinez','Communication',2020),(8,'Jim','Eli','Basket Weaving',2020);
UNLOCK TABLES;
*/
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

using namespace sql;

// Visual Leak Detector.
#include "C:\Program Files (x86)\Visual Leak Detector\include\vld.h"

// Constant data, acceptable year input.
static const int CURRENT_YEAR{ 2018 };
static const int MAX_GRAD_YEAR{ CURRENT_YEAR + 4 };

// Select and display all students in db table.
void Select(sql::Connection* con)
{
    try 
    {
        sql::Statement* stmt;
        sql::ResultSet* res;

        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT * FROM student ORDER BY id");

        // Display header.
        std::cout << "ID  First Name Last Name Major" + std::string(19, ' ') + "Grad Year\n";

        while (res->next()) 
        {
            std::cout << std::left << std::setw(4) << res->getInt(1);
            std::cout << std::setw(11) << res->getString(2);
            std::cout << std::setw(10) << res->getString(3);
            std::cout << std::setw(24) << res->getString(4);
            std::cout << res->getInt(5) << std::endl;
        }

        delete res;
        delete stmt;
    }
    catch (sql::SQLException & e)
    {
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

// Get highest (last) id number in db.
std::pair<int, int> getMinMaxId(sql::Connection* con)
{
    int min{ 0 }, max{ 0 };

    // Execute SQL statement.
    try
    {
        sql::Statement* stmt;
        sql::ResultSet* res;
        int i = 0;

        stmt = con->createStatement();
        res = stmt->executeQuery("SELECT * FROM student ORDER BY id");

        // Check for highest id.
        while (res->next())
        {
            int n = res->getInt(1);

            if (min == 0 && ++i != n)
                min = n;

            if (n > max)
                max = n;
        }

        if (min == 0)
            min = max + 1;

        delete res;
        delete stmt;
    }
    catch (sql::SQLException & e)
    {
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }

    return std::make_pair(min, max);
}
int getMinId(sql::Connection* con) { return getMinMaxId(con).first; }
int getMaxId(sql::Connection* con) { return getMinMaxId(con).second; }

// Delete student from db table.
void Remove(sql::Connection* con)
{
    int id{ 0 }, maxId{ getMaxId(con) };

    // Get student id to delete.
    if (getNumber<int>("Enter student ID # to delete: ", id, 1, maxId))
    {
        try 
        {
            sql::PreparedStatement* prep_stmt;

            prep_stmt = con->prepareStatement("DELETE FROM student WHERE id=?");
            prep_stmt->setInt(1, id);
            prep_stmt->execute();

            delete prep_stmt;
        }
        catch (sql::SQLException & e) {
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
    }
}

// Update the graduation year field of a specified record.
void Update(sql::Connection* con)
{
    int id{ 0 };
    int year{ 0 };
    int maxId{ getMaxId(con) };

    // get a student id and new graduation year.
    if (getNumber<int>("Enter student ID # to update: ", id, 1, maxId) &&
        getNumber<int>("Enter new graduation year: ", year, CURRENT_YEAR, MAX_GRAD_YEAR))
    {
        try 
        {
            PreparedStatement* prep_stmt;

            prep_stmt = con->prepareStatement("UPDATE student SET `Graduation Year`=? WHERE id=?");
            prep_stmt->setInt(1, year);
            prep_stmt->setInt(2, id);
            prep_stmt->execute();

            delete prep_stmt;
        }
        catch (sql::SQLException & e) 
        {
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
    }
}

// Insert new student into db table.
void Insert(sql::Connection* con)
{
    int year{ 0 };
    int id{ getMinId(con) };
    std::string fName{ "" }, lName{ "" }, major{ "" };

    // Get new student data.
    std::cout << "Enter new student information.\n";
    GetString("Enter first name: ", fName);
    GetString("Enter last name: ", lName);
    GetString("Enter major: ", major);
    if (getNumber<int>("Enter graduation year: ", year, CURRENT_YEAR, MAX_GRAD_YEAR))
    {
        try
        {
            // Construct a query string.
            std::string s = "INSERT INTO student VALUES(" + std::to_string(id) + ",'" + fName + "','" + lName + "','" + major + "'," + std::to_string(year) + ")";
            PreparedStatement* prep_stmt;

            prep_stmt = con->prepareStatement(s);
            prep_stmt->execute();

            delete prep_stmt;
        }
        catch (sql::SQLException & e)
        {
            std::cout << "# ERR: " << e.what();
            std::cout << " (MySQL error code: " << e.getErrorCode();
            std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
        }
    }
}

int main(void)
{
    try {
        sql::Driver* driver;
        sql::Connection* con;

        std::cout << "connecting to mysql server....";
        driver = get_driver_instance();
        con = driver->connect("tcp://127.0.0.1:3306", "admin", "password");
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
                delete con;
                done = true;
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
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
    
    return 0;
}

#if 0

// See url for more info: http://www.cyberciti.biz/tips/linux-unix-connect-mysql-c-api-program.html 
#include <stdio.h>
#include <mysql.h>

int main(void)
{
    MYSQL* conn;
    MYSQL_RES* res;
    MYSQL_ROW row;

    /* Change me */
    const char* server = "localhost";
    const char* user = "root";
    const char* password = "1qaz!QAZ";
    const char* database = "employeedb";

    conn = mysql_init(NULL);

    /* Connect to database */
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    /* send SQL query */
    if (mysql_query(conn, "show tables"))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    res = mysql_use_result(conn);

    /* output table name */
    printf("MySQL Tables in mysql database:\n");
    while ((row = mysql_fetch_row(res)) != NULL)
        printf("%s \n", row[0]);

    /* close connection */
    mysql_free_result(res);
    mysql_close(conn);

    return 0;
}
//#endif
//#if 0

// mySQL script: create database test;

/* Standard C++ includes */
#include <stdlib.h>
#include <iostream>

/*
  Include directly the different
  headers from cppconn/ and mysql_driver.h + mysql_util.h
  (and mysql_connection.h). This will reduce your build time!
*/
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <cppconn/prepared_statement.h>

using namespace std;

int main(void)
{
    cout << endl;
    cout << "Let's have MySQL count from 10 to 1..." << endl;

    try
    {
        sql::Driver* driver;
        sql::Connection* con;
        sql::Statement* stmt;
        sql::ResultSet* res;
        sql::PreparedStatement* pstmt;

        /* Create a connection */
        driver = get_driver_instance();
        //con = driver->connect("tcp://127.0.0.1:3306", "root", "root");
        con = driver->connect("tcp://127.0.0.1:3306", "admin", "password");
        /* Connect to the MySQL test database */
        con->setSchema("test");

        stmt = con->createStatement();
        stmt->execute("DROP TABLE IF EXISTS test");
        stmt->execute("CREATE TABLE test(id INT)");
        delete stmt;

        /* '?' is the supported placeholder syntax */
        pstmt = con->prepareStatement("INSERT INTO test(id) VALUES (?)");
        for (int i = 1; i <= 10; i++)
        {
            pstmt->setInt(1, i);
            pstmt->executeUpdate();
        }
        delete pstmt;

        /* Select in ascending order */
        pstmt = con->prepareStatement("SELECT id FROM test ORDER BY id ASC");
        res = pstmt->executeQuery();

        /* Fetch in reverse = descending order! */
        res->afterLast();
        while (res->previous())
            cout << "\t... MySQL counts: " << res->getInt("id") << endl;
        delete res;

        delete pstmt;
        delete con;

    }
    catch (sql::SQLException & e)
    {
        cout << "# ERR: SQLException in " << __FILE__;
        cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
        cout << "# ERR: " << e.what();
        cout << " (MySQL error code: " << e.getErrorCode();
        cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }

    cout << endl;

    return EXIT_SUCCESS;
}
#endif

