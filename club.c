 /*
    Write a program that: 
        Can run in two different modes based on cmd line input 
            Based on cmd line parameters
            Default is Non-Admin
                -> Admin 
                    Can query the member's details 
                    Can add new members
                -> Non-Admin
                    Can only query the member's details

        Can query member details - based on id number only

        Can add new member details - one or more members
            The structure (array of struct) used to hold the member details before writing them 
            to the DB is dynamic 
        
        The program must have a logging-library (self created)

        The program must be capable of being compiled in debug-mode and this will print extra logging to the screen 
            Probably handled by the logging-library

        When the program is first executed, it outputs the current number of members in the club

        ===
        Use gdb throughout to test as we build the application

        You must then spend 40 mins debugging your program using gdb to ensure execution and the 
        code paths are as expected

 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "thirdparty/sqlite/sqlite3.h"


#define MEMBER_DB "~/Documents/cprojects/Feb2022/members.db" // TODO: This would have to be put in the installation director of the applicaiton
#define TABLE_NAME "members"

typedef struct {
    char name[100];
    char phone[100];
    int id;
} Member;

typedef enum {admin, nonadmin} Modes;

int getMemberCount();
int callback(void * notUsed, int argc, char** colData, char** colNames);
int countCallback(void * notUsed, int argc, char** colData, char** colNames);
int createTable(char *tableName);
int getMemberInfo(int memberID);
int putNewMembers(Member members[], int size);

int main (int argc, char **argv)
{
    Modes mode;

    // Set the mode of execution (admin or nonadmin)
    if(argc == 1)
    {
        mode = nonadmin;
    } 
    else 
    {
        if ( strcmp( *(argv+1),"admin") == 0 )
        {
            mode = admin;
        }
        else 
        {
            puts("You must enter a valid argument or run in default mode");
            return 0;
        }
    }

    // Create Table (if not exists)
    createTable(TABLE_NAME);

    // Output the current number of members
    printf("Current member count: %d \n", getMemberCount());

    // Admin Mode
    if (mode == admin)
    {
        // Does user want to add new members or query existing members
        int userSelection = -1;
        while (userSelection != 1 && userSelection !=2 )
        {
            fputs("Press 1 to get member information. Press 2 to add new members: ", stdout);
            char sel[2]; 
            userSelection = atoi(fgets(sel, sizeof(sel), stdin));
            printf("\nYou Selected: %d \n", userSelection);
            
        }

        // QUERY MEMBERS
        if (userSelection == 1)
        {
            char memberID[10];
            puts("QUERY MEMBERS");
            if (getMemberCount() == 0)
            {
                printf("There are currently no members registered \n");
            }
            else 
            {
                fputs("Member ID ? ", stdout);
                fgets(memberID, sizeof(memberID),stdin);  // Remember that memberID is a string at this point, and not an integer
                getMemberInfo(atoi(memberID));
            }
        }

        // ADD NEW MEMBERS
        else if (userSelection == 2)
        {
            puts("ADD NEW MEMBERS");

            {
                char newMemberCount[10];
                fflush(stdin); // TODO: Not sure WHY we need this ? 
                fputs("How many members would you like to add ? ", stdout);
                fgets(newMemberCount,sizeof(newMemberCount), stdin);    
                int newMemberNum = atoi(newMemberCount);   
        
                // Allocate the memory for that many members
                Member *members = malloc(sizeof(Member) * newMemberNum);

                if(members == NULL)
                {
                    printf("Error: NULL returned from malloc \n");
                    return 0;
                }
                
                // Get the info for that many members
                for(int j = 0; j < newMemberNum; j++)
                {
                    // Get member name
                    printf("Name %d : ", j);
                    fgets(members->name,sizeof(members->name), stdin);
                    if(members->name[strlen(members->name)-1] == '\n') 
                    {
                        members->name[strlen(members->name)-1] = '\0';
                    }
                    
                    // Get member phone number
                    printf("Phone Number %d : ", j);
                    fgets(members->phone, sizeof(members->phone), stdin);
                    if(members->phone[strlen(members->phone)-1] == '\n') 
                    {
                        members->phone[strlen(members->phone)-1] = '\0';
                    }
                    

                    // Note the ID will be added later as part of the SQL statement
                }
                

                // Write all the members to the "members table"
                putNewMembers(members, newMemberNum);


                // Free the memory that was previously allocated
                
            }

        }
        else 
        {

        }

    }
}

int getMemberCount()
{
    char *sql_member_count = "SELECT COUNT(*) FROM members;";
    sqlite3 *DB;
    char *errMessage;
    int *count = malloc(sizeof(int));

    sqlite3_open(MEMBER_DB, &DB);

    int res = sqlite3_exec(DB, sql_member_count, countCallback, count, &errMessage);

    if (res != SQLITE_OK)
    {
        /*if (strstr(errMessage,"no such table") != NULL)
        {
            return 0;
        }
        */

        printf("Error in SQL Execution \n");
        printf("SQL Statement: %s \n", sql_member_count);
        printf("Error Message: %s \n", errMessage);
        printf("Error Code: %d \n", res);
        return -1;
    }

    sqlite3_close(DB);

    int *pi = (int*) count;

    return *pi;
}

int countCallback(void * count, int argc, char** colData, char** colNames)
{
    int cnt = atoi(colData[0]);
    printf ("cnt: %d \n", cnt);

    memcpy(count,&cnt, sizeof(int));
    
    return 0;
}

int callback(void * count, int argc, char** colData, char** colNames)
{
    printf("Made it to callback \n");
    for (int i = 0; i < argc; i++)
    {
        printf("%s : %s \n", colNames[i], colData[i]);
    }

    memcpy(count,&argc, sizeof(int));

    return 0;
}

int createTable(char *tableName)
{
    sqlite3 *DB;
    // TODO: Change below to use tableName arg -> use sprintf()
    char *sql_create_table = "CREATE TABLE IF NOT EXISTS members (name text, phone text, id integer PRIMARY KEY AUTOINCREMENT);";
    char *errMessage;

    sqlite3_open(MEMBER_DB, &DB);

    int res = sqlite3_exec(DB,sql_create_table,NULL,NULL,&errMessage);

    if (res != SQLITE_OK)
    {
        printf("Error in SQL Execution \n");
        printf("SQL Statement: %s \n", sql_create_table);
        printf("Error Message: %s \n", errMessage);
        printf("Error Code: %d \n", res);
        
        sqlite3_close(DB);
        return -1;
    }

    sqlite3_close(DB);
    return 1; 
}

int getMemberInfo(int memberID)
{
    sqlite3 *DB;
    char sql_get_member_info[1024];
    char *errorMessage;

    sprintf(sql_get_member_info, "SELECT FROM %s WHERE ID = %d;", TABLE_NAME, memberID);

    sqlite3_open(MEMBER_DB, &DB);

    int res = sqlite3_exec(DB, sql_get_member_info,callback, NULL, &errorMessage);

    if(res != SQLITE_OK)
    {
        printf("Error in SQL Execution \n");
        printf("SQL Statement: %s \n", sql_get_member_info);
        printf("Error Message: %s \n", errorMessage);
        printf("Error Code: %d \n", res);
    }

    return 1;
}

int putNewMembers(Member members[], int size)
{
    
    char *errMessage;

    // Prepare the SQL Statement to Execute
    char sql[size][1000];
    char sql_statements[4096];
    for(int i = 0; i < size; i++)
    {
        /*
        sprintf(sql[i],"INSERT INTO %s name, phone VALUES(%s,%s);", TABLE_NAME, members[i].name, members[i].phone);   
        strncat(sql_statements,sql[i],sizeof(sql_statements)-1);
        printf("sql: %s \n", sql[i]);
        */

       printf("Member Name: %s, Member Phone: %s \n", members[i].name, members[i].phone);
    }

    sqlite3 *DB;

    printf("sql: %s \n", sql_statements);


    sqlite3_open(MEMBER_DB, &DB);

    sqlite3_exec(DB, sql_statements, NULL, NULL, &errMessage);


    sqlite3_close(DB);


    return 1;
}