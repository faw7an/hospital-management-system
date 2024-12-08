#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <cstdlib> // Include the cstdlib library for the exit function
#include "sqlite3.h"
#include <sstream>

using namespace std;

// Function prototypes
bool authenticateAdmin(const string& username, const string& password);
void adminMenu(sqlite3* db);
void doctorMenu(sqlite3* db);
void receptionistMenu(sqlite3* db);
void registerPatient(sqlite3* db);
void staffReg(sqlite3* db);
void printMenu(const vector<string>& menu);
string mainMenu(int choice, sqlite3* db);
void viewStaffRecords(sqlite3* db);

// padding center
void printCentered(const string& text, int consoleWidth) {
    int textLength = text.length();
    int padding = (consoleWidth - textLength - 4) / 2; // Subtract 4 for the "||" borders
    int extraPadding = (consoleWidth - textLength - 4) % 2; // Handle odd/even length differences
    cout << "||" << string(padding, ' ') << text << string(padding + extraPadding, ' ') << "||" << endl;
}

// Border on the system
void printWithBorder(const vector<string>& lines) {
    int consoleWidth = 80; // Assuming a console width of 80 characters
    string border(consoleWidth, '=');

    cout << border << endl;
    for (const string& line : lines) {
        printCentered(line, consoleWidth);
    }
    cout << border << endl;
}

void printMenu(const vector<string>& menu) {
    system("cls"); // Clear the console
    cout << "\033[1m"; // Set text to bold
    printWithBorder(menu);
    cout << "\033[0m"; // Reset text formatting
}

void printRecords(const vector<string>& menu) {
    system("cls"); // Clear the console
    cout << "\033[1m"; // Set text to bold
    printWithBorder(menu);
    cout << "\033[0m"; // Reset text formatting
}

// Doctor's code:
void doctorMenu(sqlite3* db) {
    vector<string> doctorMenuOptions = {
        "Doctor's Appointments:",
        "1. Access Patients Records",
        "2. Appointments",
        "3. Back to Main Menu"
    };

    int choice = 0;
    string lastChoice; // Variable to store the last selected option
    string errorMessage;

    while (choice != 3) {
        printMenu(doctorMenuOptions);

        if (!errorMessage.empty()) {
            cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Print error message in red
        }

        if (!lastChoice.empty()) {
            cout << lastChoice << endl;
        }

        cin >> choice;

        if (cin.fail()) { // Check if the input is invalid
            errorMessage = "Your choice is invalid";
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
            choice = 0; // Reset choice to continue the loop
        } else if (choice < 1 || choice > 3) {
            errorMessage = "Invalid choice. Please choose 1, 2, or 3.";
        } else {
            errorMessage.clear(); // Clear the error message if the input is valid
            // Store the last valid choice

            switch (choice) {
                case 1:
                    // Call function for Option 1
                    lastChoice = "Doctor Option 1 selected";
                    break;
                case 2:
                    // Call function for Option 2
                    lastChoice = "Doctor Option 2 selected";
                    break;
                case 3:
                    // Go back to main menu
                    lastChoice = "Returning to main menu";
                    return;
            }
        }
    }
}
// Receptionist code:
void receptionistMenu(sqlite3* db) {
    vector<string> receptionistMenuOptions = {
        "Receptionist Menu:",
        "1. Patient registration",
        "2. Appointment Scheduling",
        "3. Billing & Payments",
        "4. Back to Main Menu"
    };

    int choice = 0;
    string lastChoice; // Variable to store the last selected option
    string errorMessage;

    while (choice != 4) {
        printMenu(receptionistMenuOptions);

        if (!errorMessage.empty()) {
            cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Print error message in red
        }

        if (!lastChoice.empty()) {
            cout << lastChoice << endl;
        }

        cin >> choice;

        if (cin.fail()) { // Check if the input is invalid
            errorMessage = "Your choice is invalid";
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
            choice = 0; // Reset choice to continue the loop
        } else if (choice < 1 || choice > 4) {
            errorMessage = "Invalid choice. Please choose 1, 2, 3, or 4.";
        } else {
            errorMessage.clear(); // Clear the error message if the input is valid
            // Store the last valid choice

            switch (choice) {
                case 1:
                    registerPatient(db);
                    lastChoice = "Patient registration";
                    break;
                case 2:
                    // Other cases...
                    break;
                case 3:
                    // Other cases...
                    break;
                case 4:
                    // Go back to main menu
                    lastChoice = "Returning to main menu";
                    return;
            }
        }
    }
}

void registerPatient(sqlite3* db) {
    char* errMsg = 0;
    string sql = "CREATE TABLE IF NOT EXISTS Patients("
                 "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "fName TEXT NOT NULL,"
                 "lName TEXT NOT NULL,"
                 "Age INTEGER NOT NULL,"
                 "Sex TEXT NOT NULL,"
                 "Allergies TEXT,"
                 "NextOfKinTelNo TEXT NOT NULL,"
                 "PatientTelNo TEXT NOT NULL,"
                 "Location TEXT NOT NULL,"
                 "registrationDate TEXT DEFAULT CURRENT_TIMESTAMP"
                 ");";

    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (table creation): " << errMsg << endl;
        sqlite3_free(errMsg);
        return;
    }

    string fName, lName, sex, allergies, patientTelNo, nextOfKinTelNo, location;
    int age;

    cout << "Enter patient first name: ";
    cin.ignore();
    getline(cin, fName);
    cout << "Enter patient last name: ";
    getline(cin, lName);
    cout << "Enter patient age: ";
    cin >> age;
    cout << "Enter patient sex: ";
    cin >> sex;
    cout << "Enter patient allergies: ";
    cin.ignore();
    getline(cin, allergies);
    cout << "Enter patient telephone number: ";
    getline(cin, patientTelNo);
    cout << "Enter next of kin telephone number: ";
    getline(cin, nextOfKinTelNo);
    cout << "Enter patient location: ";
    getline(cin, location);

    sql = "INSERT INTO Patients (fName, lName, Age, Sex, Allergies, NextOfKinTelNo, PatientTelNo, Location) "
          "VALUES ('" + fName + "', '" + lName + "', " + to_string(age) + ", '" + sex + "', '" + allergies + "', '" + nextOfKinTelNo + "', '" + patientTelNo + "', '" + location + "');";

    rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (insertion): " << errMsg << endl;
        sqlite3_free(errMsg);
    } else {
        cout << "Patient registered successfully!" << endl;
    }
}

// Admin code:
void adminMenu(sqlite3* db) {
    vector<string> adminMenuOptions = {
        "Admin Menu:",
        "1. Register Staff",
        "2. Staff Records",
        "3. Patient Records",
        "4. Financial Analysis",
        "5. Back to Main Menu"
    };

    int choice = 0;
    string lastChoice; // Variable to store the last selected option
    string errorMessage;

    while (choice != 5) {
        printMenu(adminMenuOptions);

        if (!errorMessage.empty()) {
            cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Print error message in red
        }

        if (!lastChoice.empty()) {
            cout << lastChoice << endl;
        }

        cin >> choice;

        if (cin.fail()) { // Check if the input is invalid
            errorMessage = "Your choice is invalid";
            cin.clear(); // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
            choice = 0; // Reset choice to continue the loop
        } else if (choice < 1 || choice > 5) {
            errorMessage = "Invalid choice. Please choose 1, 2, 3, 4, or 5.";
        } else {
            errorMessage.clear(); // Clear the error message if the input is valid
            // Store the last valid choice

            switch (choice) {
                case 1:
                    staffReg(db);
                    break;
                case 2:
                    // Call function for Option 2
                    viewStaffRecords(db);
                    break;
                case 3:
                    // Call function for Option 3
                    lastChoice = "Admin Option 3 selected";
                    break;
                case 4:
                    // Call function for Option 4
                    lastChoice = "Admin Option 4 selected";
                    break;
                case 5:
                    // Go back to main menu
                    lastChoice = "Returning to main menu";
                    return;
            }
        }
    }
}


// Admin auth:
bool authenticateAdmin(const string& username, const string& password) {
    const string adminUser = "admin";
    const string adminPass = "admin";
    return (username == adminUser && password == adminPass);
}

// Staff regestration function DB:
void staffReg(sqlite3* db) {
    // Create table if it doesn't exist
    const char* createTableSQL = R"sql(
        CREATE TABLE IF NOT EXISTS staff (
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,
            password TEXT NOT NULL,
            fName TEXT NOT NULL,
            lName TEXT NOT NULL,
            specialization TEXT NOT NULL,
            salary REAL NOT NULL,
            idNumber INTEGER NOT NULL,
            age INTEGER NOT NULL,
            telNo TEXT NOT NULL,
            location TEXT NOT NULL,
            HireDate TEXT DEFAULT CURRENT_TIMESTAMP
        );
    )sql";

    char* errMsg = 0;
    int rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        cerr << "SQL error (table creation): " << errMsg << endl;
        sqlite3_free(errMsg);
        return;
    } else {
        cout << "Table created successfully" << endl;
    }

    // Collect user input
    string username, password, fName, lName, specialization, location, telNo;
    int age, idNumber;
    float salary;

    cout << "Enter username: ";
    cin.ignore();
    getline(cin, username);
    cout << "Enter password: ";
    getline(cin, password);
    cout << "Enter first name: ";
    getline(cin, fName);
    cout << "Enter last name: ";
    getline(cin, lName);
    cout << "Enter specialization: ";
    getline(cin, specialization);
    cout << "Enter salary: ";
    cin >> salary;
    cout << "Enter ID number: ";
    cin >> idNumber;
    cout << "Enter age: ";
    cin >> age;
    cout << "Enter telephone number: ";
    cin.ignore(); // Clear buffer before reading telNo
    getline(cin, telNo);
    cout << "Enter location: ";
    getline(cin, location);

    // Use parameterized SQL query for insertion
    const char* insertSQL = R"sql(
        INSERT INTO staff (username, password, fName, lName, specialization, salary, idNumber, age, telNo, location)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )sql";

    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (prepare statement): " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Bind values to the prepared statement
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, fName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, lName.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, specialization.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 6, salary);
    sqlite3_bind_int(stmt, 7, idNumber);
    sqlite3_bind_int(stmt, 8, age);
    sqlite3_bind_text(stmt, 9, telNo.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, location.c_str(), -1, SQLITE_TRANSIENT);

    // Execute the statement
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL error (insertion): " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Staff registered successfully!" << endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
}


// View staff records:

void viewStaffRecords(sqlite3* db) {
    const char* sql = "SELECT * FROM staff";
    sqlite3_stmt* stmt;
    vector<string> records;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (retrieving records): " << sqlite3_errmsg(db) << endl;
        return;
    }

    cout << "Staff Records:" << endl;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        string record = "ID: " + to_string(sqlite3_column_int(stmt, 0)) +
                        ", Username: " + reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) +
                        ", First Name: " + reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)) +
                        ", Last Name: " + reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)) +
                        ", Specialization: " + reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5)) +
                        ", Salary: " + to_string(sqlite3_column_double(stmt, 6)) +
                        ", Age: " + to_string(sqlite3_column_int(stmt, 8)) +
                        ", Telephone: " + reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)) +
                        ", Location: " + reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10)) +
                        ", Hire Date: " + reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        records.push_back(record);
    }

    sqlite3_finalize(stmt);

    for (const auto& record : records) {
        cout << record << endl;
    }
}
// Main function code
int main() {
    sqlite3* db;
    int rc = sqlite3_open("hospital.db", &db);
    if (rc) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return 1;
    } else {
        cout << "Opened database successfully" << endl;
    }

    while (true) {
        int choice = 0;
        string errorMessage;
        vector<string> mainMenuOptions = {
            "Choose your role in Nairobi International Hospital:",
            "1. Administrator",
            "2. Doctor",
            "3. Receptionist",
            "4. Exit" // Add an exit option
        };

        while (choice < 1 || choice > 4) {
            printMenu(mainMenuOptions);

            if (!errorMessage.empty()) {
                cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Print error message in red
            }

            cin >> choice;

            if (cin.fail()) { // Check if the input is invalid
                errorMessage = "Your choice is invalid";
                cin.clear(); // Clear the error flag
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
                choice = 0; // Reset choice to continue the loop
            } else if (choice < 1 || choice > 4) {
                errorMessage = "Invalid choice. Please choose 1, 2, 3, or 4.";
            } else {
                errorMessage.clear(); // Clear the error message if the input is valid
            }
        }

        string result = mainMenu(choice, db);
        cout << result << endl;

        if (choice == 4) {
            break; // Exit the loop and terminate the program
        }

        system("cls"); // Clear the console
    }

    sqlite3_close(db);
    return 0;
}


string mainMenu(int choice, sqlite3* db) {
    switch(choice) {
        case 1: {
            string username, password;
            cout << "Enter admin username: ";
            cin >> username;
            cout << "Enter admin password: ";
            cin >> password;
            if (authenticateAdmin(username, password)) {
                adminMenu(db);
                return "Admin menu accessed";
            } else {
                return "Authentication failed";
            }
        }
        case 2:
            doctorMenu(db);
            return "Doctor menu accessed";
        case 3:
            receptionistMenu(db);
            return "Receptionist menu accessed";
        case 4:
            return "Exiting...";
        default:
            return "Invalid choice";
    }
}