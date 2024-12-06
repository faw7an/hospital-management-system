#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <cstdlib> // Include the cstdlib library for the exit function
#include "sqlite3.h"

using namespace std;

// Function prototypes
bool authenticateAdmin(const string& username, const string& password);
void adminMenu();
void doctorMenu();
void receptionistMenu();
void printMenu(const vector<string>& menu);
string mainMenu(int choice);
void registerPatient(sqlite3* db);


void printCentered(const string& text, int consoleWidth) {
    int textLength = text.length();
    int padding = (consoleWidth - textLength - 4) / 2; // Subtract 4 for the "||" borders
    int extraPadding = (consoleWidth - textLength - 4) % 2; // Handle odd/even length differences
    cout << "||" << string(padding, ' ') << text << string(padding + extraPadding, ' ') << "||" << endl;
}

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


void adminMenu() {
    vector<string> adminMenuOptions = {
        "Admin Menu:",
        "1. Option 1",
        "2. Option 2",
        "3. Option 3",
        "4. Back to Main Menu"
    };

    int choice = 0;
    string lastChoice; // Variable to store the last selected option
    string errorMessage;

    while (choice != 4) {
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
        } else if (choice < 1 || choice > 4) {
            errorMessage = "Invalid choice. Please choose 1, 2, 3, or 4.";
        } else {
            errorMessage.clear(); // Clear the error message if the input is valid
            // Store the last valid choice

            switch (choice) {
                case 1:
                    // Call function for Option 1
                    lastChoice = "Admin Option 1 selected";
                    break;
                case 2:
                    // Call function for Option 2
                    lastChoice = "Admin Option 2 selected";
                    break;
                case 3:
                    // Call function for Option 3
                    lastChoice = "Admin Option 3 selected";
                    break;
                case 4:
                    // Go back to main menu
                    lastChoice = "Returning to main menu";
                    break;
            }
        }
    }
}

void adminAddStaff() {
    // Function for Admin add staff 
}

void adminViewRecords(){

}

void doctorMenu() {
    vector<string> doctorMenuOptions = {
        "Doctor's Appointments:",
        "1. Access Patients Records",
        "2. Appointments",
        // "3. Flag Emergency Patients",
        "3. Back to Main Menu"
    };

    int choice = 0;
    string lastChoice; // Variable to store the last selected option
    string errorMessage;

    while (choice != 4) {
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
                // case 3:
                //     // Call function for Option 3
                //     lastChoice = "Doctor Option 3 selected";
                //     break;
                case 3:
                    // Go back to main menu
                    lastChoice = "Returning to main menu";
                    break;
            }
        }
    }
}

void receptionistMenu() {
    vector<string> receptionistMenuOptions = {
        "Receptionist Menu:",
        "1.  Patient registration",
        "2. Appointment Scheduling",
        "3. Billing & Payments",
        "4. Back to Main Menu"
    };

    int choice = 0;
    string lastChoice; // Variable to store the last selected option
    string errorMessage;
    string dbMessage; // Variable to store the database message
    
    // Initialize SQLite
    sqlite3* db;
    int rc = sqlite3_open("hospital.db", &db);
    if (rc) {
        dbMessage = "Can't open database: " + string(sqlite3_errmsg(db));
        cerr << dbMessage << endl;
        return;
    } else {
        dbMessage = "Opened database successfully";
        cout << dbMessage << endl;
    }
    
    while (choice != 4) {
        printMenu(receptionistMenuOptions);
    
        if (!dbMessage.empty()) {
            cout << dbMessage << endl; // Print the database message
        }
    
        if (!errorMessage.empty()) {
            cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Print error message in red
        }
    
        if (!lastChoice.empty()) {
            cout << lastChoice << endl;
        }
    
        cout << "Enter your choice: ";
        cin >> choice;
    
        switch (choice) {
            case 1:
                // Implement patient registration
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
                // Exit
                break;
            default:
                errorMessage = "Invalid choice. Please choose 1, 2, 3, or 4.";
                break;
        }
    }
    // Close SQLite
    sqlite3_close(db);
}

void registerPatient(sqlite3* db) {
    char* errMsg = 0;
    string sql = "CREATE TABLE IF NOT EXISTS Patients ("
                 "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
                 "Name TEXT NOT NULL,"
                 "Age INTEGER NOT NULL,"
                 "Sex TEXT NOT NULL,"
                 "Allergies TEXT,"
                 "NextOfKinTelNo TEXT NOT NULL,"
                 "PatientTelNo TEXT NOT NULL,"
                 "Location TEXT NOT NULL);";

    int rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (table creation): " << errMsg << endl;
        sqlite3_free(errMsg);
        return;
    }

    string name, sex, allergies, nextOfKinTelNo, patientTelNo, location;
    int age;

    cout << "Enter patient name: ";
    cin.ignore();
    getline(cin, name);
    cout << "Enter patient age: ";
    cin >> age;
    cout << "Enter patient sex: ";
    cin >> sex;
    cout << "Enter patient allergies: ";
    cin.ignore();
    getline(cin, allergies);
    cout << "Enter next of kin telephone number: ";
    getline(cin, nextOfKinTelNo);
    cout << "Enter patient telephone number: ";
    getline(cin, patientTelNo);
    cout << "Enter patient location: ";
    getline(cin, location);

    sql = "INSERT INTO Patients (Name, Age, Sex, Allergies, NextOfKinTelNo, PatientTelNo, Location) "
          "VALUES ('" + name + "', " + to_string(age) + ", '" + sex + "', '" + allergies + "', '" + nextOfKinTelNo + "', '" + patientTelNo + "', '" + location + "');";

    rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (insertion): " << errMsg << endl;
        sqlite3_free(errMsg);
    } else {
        cout << "Patient registered successfully!" << endl;
    }
}
bool authenticateAdmin(const string& username, const string& password) {
    const string adminUser = "admin";
    const string adminPass = "admin";
    return (username == adminUser && password == adminPass);
}

int main() {
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

    cout << mainMenu(choice) << endl;

    return 0;
}

string mainMenu(int choice) {
    switch(choice) {
        case 1: {
            string username, password;
            cout << "Enter admin username: ";
            cin >> username;
            cout << "Enter admin password: ";
            cin >> password;
            if (authenticateAdmin(username, password)) {
                adminMenu();
                return "Admin menu accessed";
            } else {
                return "Authentication failed";
            }
        }
        case 2:
            doctorMenu();
            return "Doctor menu accessed";
        case 3:
            receptionistMenu();
            return "Receptionist menu accessed";
        case 4:
            exit(0); // Terminate the process and close the terminal
        default:
            return "Invalid choice";
    }
}