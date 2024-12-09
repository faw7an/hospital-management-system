#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <cstdlib> // Include the cstdlib library for the exit function
#include "sqlite3.h"
#include <sstream>
#include <iomanip> // Include for std::setw and std::left
#include <ctime> // Include for time functions


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
void viewPatientRecords(sqlite3* db);
void createAppointment(sqlite3* db, string& errorMessage); // Updated prototype
int authenticateUser(sqlite3* db, const string& username, const string& password, const string& specialization);
void viewDoctorAppointments(sqlite3* db, int doctorID); // Declare the function here



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

// Authenticate user based on username, password, and specialization
int authenticateUser(sqlite3* db, const string& username, const string& password, const string& specialization) {
    const char* sql;
    if (specialization == "doctor") {
        // Check for any specialization except "receptionist"
        sql = "SELECT ID FROM staff WHERE username = ? AND password = ? AND specialization != 'receptionist'";
    } else {
        // Check for specific specialization
        sql = "SELECT ID FROM staff WHERE username = ? AND password = ? AND specialization = ?";
    }

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (preparing statement): " << sqlite3_errmsg(db) << endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    if (specialization != "doctor") {
        sqlite3_bind_text(stmt, 3, specialization.c_str(), -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    int doctorID = (rc == SQLITE_ROW) ? sqlite3_column_int(stmt, 0) : -1;

    sqlite3_finalize(stmt);
    return doctorID;
}
// Function to print appointments
void printAppointments(const vector<string>& appointments) {
    system("cls"); // Clear the console
    cout << "\033[1m"; // Set text to bold

    // Print table header
    cout << left << setw(5) << "ID" 
         << left << setw(20) << "Appointment Date" 
         << left << setw(15) << "Time" 
         << left << setw(20) << "Name" 
         << endl;

    cout << string(60, '=') << endl; // Print a separator line

    // Print each appointment
    for (const auto& appointment : appointments) {
        cout << appointment << endl;
    }

    cout << "\033[0m"; // Reset text formatting
}

// Function to view appointments for a specific doctor
void viewDoctorAppointments(sqlite3* db, int doctorID) {
    const char* sql = "SELECT ID, AppointmentDate, AppointmentTime, PatientName FROM Appointments WHERE DoctorID = ?";
    sqlite3_stmt* stmt;
    vector<string> appointments;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (retrieving appointments): " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, doctorID);

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        stringstream appointment;
        appointment << left << setw(5) << sqlite3_column_int(stmt, 0)
                    << left << setw(20) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
                    << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))
                    << left << setw(20) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        appointments.push_back(appointment.str());
    }

    sqlite3_finalize(stmt);

    while (true) {
        printAppointments(appointments);
        cout << endl;
        cout << "==|| Press 'b' to go back to the Doctor menu: ||==" << endl;
        char choice;
        cin >> choice;
        if (choice == 'b' || choice == 'B') {
            break;
        }
    }
}

// Doctor's code:
void doctorMenu(sqlite3* db, int doctorID) {
    vector<string> doctorMenuOptions = {
        "Doctor's Appointments:",
        "1. Access Patients Records",
        "2. View My Appointments",
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
                    // View appointments for this doctor
                    viewDoctorAppointments(db, doctorID);
                    
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
    string username, password;
    cout << "Enter receptionist username: ";
    cin >> username;
    cout << "Enter receptionist password: ";
    cin >> password;

    if (authenticateUser(db, username, password, "receptionist") == -1) {
        cout << "\033[31mAuthentication failed\033[0m" << endl;
        return;
    }

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
            switch (choice) {
                case 1:
                    registerPatient(db);
                    lastChoice = "Patient registration";
                    break;
                case 2:
                    createAppointment(db, errorMessage); // Pass the error message reference
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

// Register patients
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
                 "registrationDate DATETIME DEFAULT CURRENT_TIMESTAMP"
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


bool parseDateTime(const std::string& dateTimeStr, struct tm& tm) {
    std::istringstream ss(dateTimeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
    return !ss.fail();
}

// Book Appointment
void createAppointment(sqlite3* db, string& errorMessage) {
    const char* sql = "SELECT ID, fName, lName, specialization FROM staff WHERE specialization != 'receptionist'";
    sqlite3_stmt* stmt;
    vector<string> doctors;
    char* errMsg = 0;

    while (true) {
        if (!errorMessage.empty()) {
            cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Display error message in red
            cout << "Please try again." << endl;
            errorMessage.clear(); // Clear the error message after displaying it
        }

        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            cerr << "SQL error (retrieving doctors): " << sqlite3_errmsg(db) << endl;
            return;
        }

        doctors.clear();
        cout << "Available Doctors:" << endl;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            stringstream doctor;
            doctor << "ID: " << sqlite3_column_int(stmt, 0) << ", "
                   << "Name: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) << " "
                   << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)) << ", "
                   << "Specialization: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            doctors.push_back(doctor.str());
        }

        sqlite3_finalize(stmt);

        if (doctors.empty()) {
            cout << "No doctors available." << endl;
            return;
        }

        for (const auto& doctor : doctors) {
            cout << doctor << endl;
        }

        int doctorID;
        string doctorName;
        cout << "Enter the ID of the doctor for the appointment: ";
        cin >> doctorID;

        // Check if the doctor exists
        bool doctorExists = false;
        for (const auto& doctor : doctors) {
            if (doctor.find("ID: " + to_string(doctorID)) != string::npos) {
                doctorName = doctor.substr(doctor.find("Name: ") + 6);
                doctorName = doctorName.substr(0, doctorName.find(","));
                doctorExists = true;
                break;
            }
        }

        if (!doctorExists) {
            errorMessage = "Error: Doctor with ID " + to_string(doctorID) + " does not exist.";
            return;
        }

        // Retrieve patients from the Patients table
        sql = "SELECT ID, fName, lName FROM Patients";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            cerr << "SQL error (retrieving patients): " << sqlite3_errmsg(db) << endl;
            return;
        }

        vector<string> patients;
        cout << "Available Patients:" << endl;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            stringstream patient;
            patient << "ID: " << sqlite3_column_int(stmt, 0) << ", "
                    << "Name: " << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)) << " "
                    << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            patients.push_back(patient.str());
        }

        sqlite3_finalize(stmt);

        if (patients.empty()) {
            cout << "No patients available." << endl;
            return;
        }

        for (const auto& patient : patients) {
            cout << patient << endl;
        }

        int patientID;
        string patientName;
        cout << "Enter the ID of the patient for the appointment: ";
        cin >> patientID;

        // Check if the patient exists
        bool patientExists = false;
        for (const auto& patient : patients) {
            if (patient.find("ID: " + to_string(patientID)) != string::npos) {
                patientName = patient.substr(patient.find("Name: ") + 6);
                patientExists = true;
                break;
            }
        }

        if (!patientExists) {
            errorMessage = "Error: Patient with ID " + to_string(patientID) + " does not exist.";
            return;
        }

        // Check if the patient already has an appointment
        sql = "SELECT COUNT(*) FROM Appointments WHERE PatientID = ?";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            cerr << "SQL error (checking existing appointment): " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_int(stmt, 1, patientID);
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
            errorMessage = "Error: This patient already has an appointment.";
            sqlite3_finalize(stmt);
            return; 
        }
        sqlite3_finalize(stmt);

        string appointmentDate, appointmentTime;
        cout << "Enter appointment date (YYYY-MM-DD): ";
        cin.ignore();
        getline(cin, appointmentDate);
        cout << "Enter appointment time (HH:MM): ";
        getline(cin, appointmentTime);

        // Validate date and time format
        struct tm tm = {};
        if (!parseDateTime(appointmentDate + " " + appointmentTime, tm)) {
            errorMessage = "Error: Invalid date/time format.";
            return;
        }

        // Check if the date/time is in the past
        time_t now = time(0);
        time_t appointmentTimeT = mktime(&tm);
        if (difftime(appointmentTimeT, now) < 0) {
            errorMessage = "Error: Appointment date/time is in the past.";
            return;
        }

        // Check if the appointment slot is already taken
        sql = "SELECT COUNT(*) FROM Appointments WHERE AppointmentDate = ? AND AppointmentTime = ?";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            cerr << "SQL error (checking appointment slot): " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_text(stmt, 1, appointmentDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, appointmentTime.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
            errorMessage = "Error: This appointment slot is already taken.";
            sqlite3_finalize(stmt);
            return; // Terminate the loop and return to the receptionist menu
        }
        sqlite3_finalize(stmt);

        sql = "CREATE TABLE IF NOT EXISTS Appointments("
              "ID INTEGER PRIMARY KEY AUTOINCREMENT,"
              "DoctorID INTEGER NOT NULL,"
              "DoctorName TEXT NOT NULL,"
              "PatientID INTEGER NOT NULL UNIQUE," // Unique constraint on PatientID
              "PatientName TEXT NOT NULL,"
              "AppointmentDate TEXT NOT NULL,"
              "AppointmentTime TEXT NOT NULL,"
              "FOREIGN KEY(DoctorID) REFERENCES staff(ID),"
              "FOREIGN KEY(PatientID) REFERENCES Patients(ID)"
              ");";

        rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            cerr << "SQL error (creating appointments table): " << errMsg << endl;
            sqlite3_free(errMsg);
            return;
        }

        stringstream insertSql;
        insertSql << "INSERT INTO Appointments (DoctorID, DoctorName, PatientID, PatientName, AppointmentDate, AppointmentTime) VALUES ("
                  << doctorID << ", '" << doctorName << "', " << patientID << ", '" << patientName << "', '" << appointmentDate << "', '" << appointmentTime << "');";

        rc = sqlite3_exec(db, insertSql.str().c_str(), 0, 0, &errMsg);
        if (rc != SQLITE_OK) {
            cerr << "SQL error (inserting appointment): " << errMsg << endl;
            sqlite3_free(errMsg);
        } else {
            cout << "Appointment created successfully!" << endl;
            break;
        }
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
                    viewPatientRecords(db);
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
            salary INTEGER NOT NULL,
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
    sqlite3_bind_int(stmt, 6, salary);
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
void printRecords(const vector<string>& menu) {
    system("cls"); // Clear the console
    cout << "\033[1m"; // Set text to bold

    // Print table header
    cout << left << setw(5) << "ID" 
         << left << setw(15) << "Username" 
         << left << setw(15) << "First Name" 
         << left << setw(15) << "Last Name" 
         << left << setw(20) << "Specialization" 
         << left << setw(10) << "Salary" 
         << left << setw(5) << "Age" 
         << left << setw(15) << "Telephone" 
         << left << setw(15) << "Location" 
         << left << setw(15) << "Hire Date" 
         << endl;

    cout << string(130, '=') << endl; // Print a separator line

    // Print each record
    for (const auto& record : menu) {
        cout << record << endl;
    }

    cout << "\033[0m"; // Reset text formatting
}

void viewStaffRecords(sqlite3* db) {
    const char* sql = "SELECT * FROM staff";
    sqlite3_stmt* stmt;
    vector<string> menu;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (retrieving records): " << sqlite3_errmsg(db) << endl;
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        stringstream record;
        record << left << setw(5) << sqlite3_column_int(stmt, 0)
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))
               << left << setw(20) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))
               << left << setw(10) << to_string(sqlite3_column_int(stmt, 6))
               << left << setw(5) << to_string(sqlite3_column_int(stmt, 8))
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9))
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10))
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11));
        menu.push_back(record.str());
    }

    sqlite3_finalize(stmt);

    while (true) {
        printRecords(menu);
        cout<<endl;
        cout<<endl;
        cout << "==|| Press 'b' to go back to the Admin menu: ||=="<<endl;
        char choice;
        cin >> choice;
        if (choice == 'b' || choice == 'B') {
            break;
        }
    }
}

// Patients records:
void printPatientRecords(const vector<string>& menu) {
    system("cls"); // Clear the console
    cout << "\033[1m"; // Set text to bold

    // Print table header
    cout << left << setw(5) << "ID" 
         << left << setw(15) << "First Name" 
         << left << setw(15) << "Last Name" 
         << left << setw(5) << "Age" 
         << left << setw(10) << "Sex" 
         << left << setw(20) << "Allergies" 
         << left << setw(15) << "Next of Kin Tel" 
         << left << setw(15) << " Patient Tel" 
         << left << setw(15) << "Location" 
         << left << setw(20) << "Registration Date" 
         << endl;

    cout << string(135, '=') << endl; // Print a separator line

    // Print each record
    for (const auto& record : menu) {
        cout << record << endl;
    }

    cout << "\033[0m"; // Reset text formatting
}

void viewPatientRecords(sqlite3* db) {
    const char* sql = "SELECT * FROM Patients";
    sqlite3_stmt* stmt;
    vector<string> menu;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (retrieving records): " << sqlite3_errmsg(db) << endl;
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        stringstream record;
        record << left << setw(5) << sqlite3_column_int(stmt, 0)  // ID
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))  // fName
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))  // lName
               << left << setw(5) << sqlite3_column_int(stmt, 3)  // Age
               << left << setw(10) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))  // Sex
               << left << setw(20) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))  // Allergies
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6))  // NextOfKinTelNo
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7))  // PatientTelNo
               << left << setw(15) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8))  // Location
               << left << setw(20) << reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9)); // registrationDate
        menu.push_back(record.str());
    }

    sqlite3_finalize(stmt);

    while (true) {
        printPatientRecords(menu);
        cout << endl;
        cout << endl;
        cout << "==|| Press 'b' to go back to the Admin menu: ||==" << endl;
        char choice;
        cin >> choice;
        if (choice == 'b' || choice == 'B') {
            break;
        }
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
        case 2: {
            string username, password;
            cout << "Enter doctor username: ";
            cin >> username;
            cout << "Enter doctor password: ";
            cin >> password;
            int doctorID = authenticateUser(db, username, password, "doctor");
            if (doctorID != -1) {
                doctorMenu(db, doctorID);
                return "Doctor menu accessed";
            } else {
                return "Authentication failed";
            }
        }
        case 3: {
            string username, password;
            cout << "Enter receptionist username: ";
            cin >> username;
            cout << "Enter receptionist password: ";
            cin >> password;
            if (authenticateUser(db, username, password, "receptionist") != -1) {
                receptionistMenu(db);
                return "Receptionist menu accessed";
            } else {
                return "Authentication failed";
            }
        }
        case 4:
            return "Exiting...";
        default:
            return "Invalid choice";
    }
}