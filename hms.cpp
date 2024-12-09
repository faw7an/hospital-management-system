#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <cstdlib> // Include the cstdlib library for the exit function
#include "sqlite3.h"
#include <sstream>
#include <iomanip> // Include for std::setw and std::left
#include <ctime>   // Include for time functions

using namespace std;

// Function prototypes
bool authenticateAdmin(const string &username, const string &password);
void adminMenu(sqlite3 *db);
void doctorMenu(sqlite3* db, int doctorID);
void receptionistMenu(sqlite3 *db);
void registerPatient(sqlite3 *db);
void staffReg(sqlite3 *db);
void printMenu(const vector<string> &menu);
string mainMenu(int choice, sqlite3 *db);
void viewStaffRecords(sqlite3 *db);
void viewPatientRecords(sqlite3 *db);
void createAppointment(sqlite3 *db, string &errorMessage); // Updated prototype
int authenticateUser(sqlite3 *db, const string &username, const string &password, const string &specialization);
void viewDoctorAppointments(sqlite3 *db, int doctorID); // Declare the function here
void tendToAppointment(sqlite3* db, int appointmentID, int doctorID); 
void viewBilling(sqlite3 *db);
string verifyPayment(sqlite3 *db, int patientID);
void financialAnalysis(sqlite3 *db);
void createAppointmentsTable(sqlite3 *db);


// padding center
void printCentered(const string &text, int consoleWidth)
{
    int textLength = text.length();
    int padding = (consoleWidth - textLength - 4) / 2;      // Subtract 4 for the "||" borders
    int extraPadding = (consoleWidth - textLength - 4) % 2; // Handle odd/even length differences
    cout << "||" << string(padding, ' ') << text << string(padding + extraPadding, ' ') << "||" << endl;
}

// Border on the system
void printWithBorder(const vector<string> &lines)
{
    int consoleWidth = 80; // Assuming a console width of 80 characters
    string border(consoleWidth, '=');

    cout << border << endl;
    for (const string &line : lines)
    {
        printCentered(line, consoleWidth);
    }
    cout << border << endl;
}

void printMenu(const vector<string> &menu)
{
    system("cls");     // Clear the console
    cout << "\033[1m"; // Set text to bold
    printWithBorder(menu);
    cout << "\033[0m"; // Reset text formatting
}

// Authenticate user based on username, password, and specialization
int authenticateUser(sqlite3 *db, const string &username, const string &password, const string &specialization)
{
    const char *sql;
    if (specialization == "doctor")
    {
        // Check for any specialization except "receptionist"
        sql = "SELECT ID FROM staff WHERE username = ? AND password = ? AND specialization != 'receptionist'";
    }
    else
    {
        // Check for specific specialization
        sql = "SELECT ID FROM staff WHERE username = ? AND password = ? AND specialization = ?";
    }

    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error (preparing statement): " << sqlite3_errmsg(db) << endl;
        return -1;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);
    if (specialization != "doctor")
    {
        sqlite3_bind_text(stmt, 3, specialization.c_str(), -1, SQLITE_TRANSIENT);
    }

    rc = sqlite3_step(stmt);
    int doctorID = (rc == SQLITE_ROW) ? sqlite3_column_int(stmt, 0) : -1;

    sqlite3_finalize(stmt);
    return doctorID;
}


// Access patient records
void viewDiagnoses(sqlite3 *db) {
    const char *sql = "SELECT ID, DoctorName, PatientName, Diagnosis, Prescription FROM Diagnoses";
    sqlite3_stmt *stmt;
    vector<string> diagnoses;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (retrieving diagnoses): " << sqlite3_errmsg(db) << endl;
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        stringstream diagnosis;
        diagnosis << left << setw(5) << sqlite3_column_int(stmt, 0)
                  << left << setw(20) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))
                  << left << setw(20) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2))
                  << left << setw(30) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3))
                  << left << setw(30) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        diagnoses.push_back(diagnosis.str());
    }

    sqlite3_finalize(stmt);

    while (true) {
        system("cls");     // Clear the console
        cout << "\033[1m"; // Set text to bold

        // Print table header
        cout << left << setw(5) << "ID"
             << left << setw(20) << "Doctor Name"
             << left << setw(20) << "Patient Name"
             << left << setw(30) << "Diagnosis"
             << left << setw(30) << "Prescription"
             << endl;

        cout << string(105, '=') << endl; // Print a separator line

        // Print each diagnosis
        for (const auto &diagnosis : diagnoses) {
            cout << diagnosis << endl;
        }

        cout << "\033[0m"; // Reset text formatting

        cout << endl;
        cout << "==|| Press 'b' to go back to the Doctor menu ||==" << endl;
        char choice;
        cin >> choice;
        if (choice == 'b' || choice == 'B') {
            break;
        }
    }
}

// Function to print appointments
void printAppointments(const vector<string> &appointments)
{
    system("cls");     // Clear the console
    cout << "\033[1m"; // Set text to bold

    // Print table header
    cout << left << setw(5) << "ID"
         << left << setw(20) << "Appointment Date"
         << left << setw(15) << "Appointment Time"
         << left << setw(20) << "Patient Name"
         << endl;

    cout << string(60, '=') << endl; // Print a separator line

    // Print each appointment
    for (const auto &appointment : appointments)
    {
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
        cout << "==|| Press 'b' to go back to the Doctor menu ||==" << endl;
        cout << "==|| Enter the ID of the appointment to tend to: ||==" << endl;
        char choice;
        cin >> choice;
        if (choice == 'b' || choice == 'B') {
            break;
        } else {
            int appointmentID = choice - '0'; // Convert char to int
            tendToAppointment(db, appointmentID, doctorID); // Updated call
        }
    }
}

// Function to tend to an appointment
void tendToAppointment(sqlite3 *db, int appointmentID, int doctorID) {
    const char *sql = "SELECT AppointmentDate, AppointmentTime, PatientName, DoctorID, DoctorName FROM Appointments WHERE ID = ?";
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (retrieving appointment): " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, appointmentID);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        string appointmentDate = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        string appointmentTime = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        string patientName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
        string doctorName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
        cout << "Tending to appointment:" << endl;
        cout << "Date: " << appointmentDate << ", Time: " << appointmentTime << ", Patient: " << patientName << endl;

        // Clear the screen
        system("cls");

        // Collect diagnosis data
        string diagnosis, prescription;
        double bill;
        cout << "Enter diagnosis: ";
        cin.ignore();
        getline(cin, diagnosis);
        cout << "Enter prescription: ";
        getline(cin, prescription);
        cout << "Enter bill amount: ";
        cin >> bill;

        // Create Diagnoses table if it doesn't exist
        const char *createTableSQL = R"sql(
            CREATE TABLE IF NOT EXISTS Diagnoses (
                ID INTEGER PRIMARY KEY AUTOINCREMENT,
                AppointmentID INTEGER NOT NULL,
                DoctorID INTEGER NOT NULL,
                DoctorName TEXT NOT NULL,
                PatientName TEXT NOT NULL,
                Diagnosis TEXT NOT NULL,
                Prescription TEXT NOT NULL,
                Bill REAL NOT NULL,
                FOREIGN KEY(AppointmentID) REFERENCES Appointments(ID)
            );
        )sql";

        rc = sqlite3_exec(db, createTableSQL, 0, 0, 0);
        if (rc != SQLITE_OK) {
            cerr << "SQL error (creating Diagnoses table): " << sqlite3_errmsg(db) << endl;
            return;
        }

        // Insert diagnosis data into Diagnoses table
        const char *insertSQL = "INSERT INTO Diagnoses (AppointmentID, DoctorID, DoctorName, PatientName, Diagnosis, Prescription, Bill) VALUES (?, ?, ?, ?, ?, ?, ?)";
        rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            cerr << "SQL error (prepare statement): " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_int(stmt, 1, appointmentID);
        sqlite3_bind_int(stmt, 2, doctorID);
        sqlite3_bind_text(stmt, 3, doctorName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, patientName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, diagnosis.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, prescription.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 7, bill);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            cerr << "SQL error (inserting diagnosis): " << sqlite3_errmsg(db) << endl;
        } else {
            cout << "Diagnosis, prescription, and bill saved successfully!" << endl;
        }

        sqlite3_finalize(stmt);

        // Remove the appointment from the Appointments table
        const char *deleteSQL = "DELETE FROM Appointments WHERE ID = ?";
        rc = sqlite3_prepare_v2(db, deleteSQL, -1, &stmt, 0);
        if (rc != SQLITE_OK) {
            cerr << "SQL error (prepare delete statement): " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_int(stmt, 1, appointmentID);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            cerr << "SQL error (deleting appointment): " << sqlite3_errmsg(db) << endl;
        } else {
            cout << "Appointment removed successfully!" << endl;
        }

        sqlite3_finalize(stmt);
    } else {
        cout << "Appointment not found." << endl;
    }

    sqlite3_finalize(stmt);

    // Return to viewDoctorAppointments
    viewDoctorAppointments(db, doctorID);
}


// Doctor's menu
void doctorMenu(sqlite3 *db, int doctorID) {
    vector<string> doctorMenuOptions = {
        "Doctor's Appointments:",
        "1. Access Patients Records",
        "2. View My Appointments",
        "3. Back to Main Menu"
    };

    int choice = 0;
    string lastChoice; // Variable to store the last selected option
    string errorMessage;

    while (true) {
        printMenu(doctorMenuOptions);

        if (!errorMessage.empty()) {
            cout << "Error: " << errorMessage << endl;
            errorMessage.clear();
        }

        if (!lastChoice.empty()) {
            cout << "Last choice: " << lastChoice << endl;
            lastChoice.clear();
        }

        cout << "Enter your choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            errorMessage = "Invalid input. Please enter a number.";
            continue;
        }

        if (choice < 1 || choice > 3) {
            errorMessage = "Invalid choice. Please select a valid option.";
            continue;
        }

        switch (choice) {
            case 1:
               // View Diagnoses
                viewDiagnoses(db);
                break;
            case 2:
                // View My Appointments
                viewDoctorAppointments(db, doctorID);
                break;
            case 3:
                // Back to Main Menu
                lastChoice = "Returning to main menu";
                return;
        }
    }
}

// Receptionist code:
void receptionistMenu(sqlite3 *db)
{
    vector<string> receptionistMenuOptions = {
        "Receptionist Menu:",
        "1. Patient registration",
        "2. Appointment Scheduling",
        "3. Billing & Payments",
        "4. Back to Main Menu"};

    int choice = 0;
    string lastChoice; // Variable to store the last selected option
    string errorMessage;

    while (true)
    {
        printMenu(receptionistMenuOptions);

        if (!errorMessage.empty())
        {
            cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Print error message in red
        }

        if (!lastChoice.empty())
        {
            cout << lastChoice << endl;
        }

        cout << "Enter your choice: ";
        cin >> choice;

        if (cin.fail())
        { // Check if the input is invalid
            errorMessage = "Your choice is invalid";
            cin.clear();                                         // Clear the error flag
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
            choice = 0;                                          // Reset choice to continue the loop
        }
        else if (choice < 1 || choice > 4)
        {
            errorMessage = "Invalid choice. Please choose 1, 2, 3, or 4.";
        }
        else
        {
            errorMessage.clear(); // Clear the error message if the input is valid
            switch (choice)
            {
            case 1:
                registerPatient(db);
                break;
            case 2:
                createAppointment(db, errorMessage); // Pass the error message reference
                break;
            case 3:
                // Billing & Payments
                viewBilling(db);
                break;
            case 4:
                // Go back to main menu
                lastChoice = "Returning to main menu";
                return;
            }
        }
    }
}




// View billing
void viewBilling(sqlite3 *db) {
    const char *sql = "SELECT ID, PatientName, Bill FROM Diagnoses";
    sqlite3_stmt *stmt;
    vector<string> billing;
    string message;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (retrieving billing): " << sqlite3_errmsg(db) << endl;
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        stringstream bill;
        bill << left << setw(5) << sqlite3_column_int(stmt, 0)
             << left << setw(30) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))
             << left << setw(10) << sqlite3_column_double(stmt, 2);
        billing.push_back(bill.str());
    }

    sqlite3_finalize(stmt);

    while (true) {
        system("cls");     // Clear the console
        cout << "\033[1m"; // Set text to bold

        // Print table header
        cout << left << setw(5) << "ID"
             << left << setw(30) << "Patient Name"
             << left << setw(10) << "Bill"
             << endl;

        cout << string(45, '=') << endl; // Print a separator line

        // Print each billing record
        for (const auto &bill : billing) {
            cout << bill << endl;
        }

        cout << "\033[0m"; // Reset text formatting

        if (!message.empty()) {
            cout << message << endl;
            message.clear();
        }

        cout << endl;
        cout << "==|| Press 'b' to go back to the Receptionist menu ||==" << endl;
        cout << "==|| Enter the ID of the patient to verify payment: ||==" << endl;
        char choice;
        cin >> choice;
        if (choice == 'b' || choice == 'B') {
            break;
        } else {
            int patientID = choice - '0'; // Convert char to int
            message = verifyPayment(db, patientID); // Verify payment and store the message
        }
    }
}


// Verify payment
string verifyPayment(sqlite3 *db, int patientID) {
    const char *selectSQL = "SELECT Bill FROM Diagnoses WHERE ID = ?";
    sqlite3_stmt *stmt;
    double billAmount = 0.0;
    string message;

    int rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (preparing select statement): " << sqlite3_errmsg(db) << endl;
        return "\033[31mSQL error (preparing select statement)\033[0m"; // Red text
    }

    sqlite3_bind_int(stmt, 1, patientID);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        billAmount = sqlite3_column_double(stmt, 0);
    } else {
        cerr << "Error: Patient ID not found." << endl;
        sqlite3_finalize(stmt);
        return "\033[31mError: Patient ID not found\033[0m"; // Red text
    }

    sqlite3_finalize(stmt);

    // Create PaidBills table if it doesn't exist
    const char *createTableSQL = R"sql(
        CREATE TABLE IF NOT EXISTS PaidBills (
            ID INTEGER PRIMARY KEY,
            BillPaid REAL NOT NULL
        );
    )sql";

    rc = sqlite3_exec(db, createTableSQL, 0, 0, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (creating PaidBills table): " << sqlite3_errmsg(db) << endl;
        return "\033[31mSQL error (creating PaidBills table)\033[0m"; // Red text
    }

    // Check if the payment record already exists
    const char *checkSQL = "SELECT COUNT(*) FROM PaidBills WHERE ID = ?";
    rc = sqlite3_prepare_v2(db, checkSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (preparing check statement): " << sqlite3_errmsg(db) << endl;
        return "\033[31mSQL error (preparing check statement)\033[0m"; // Red text
    }

    sqlite3_bind_int(stmt, 1, patientID);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
        cerr << "Error: Payment record already exists for this patient ID." << endl;
        sqlite3_finalize(stmt);
        return "\033[31mError: Payment record already exists for this patient ID\033[0m"; // Red text
    }

    sqlite3_finalize(stmt);

    // Insert payment data into PaidBills table
    const char *insertSQL = "INSERT INTO PaidBills (ID, BillPaid) VALUES (?, ?)";
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (preparing insert statement): " << sqlite3_errmsg(db) << endl;
        return "\033[31mSQL error (preparing insert statement)\033[0m"; // Red text
    }

    sqlite3_bind_int(stmt, 1, patientID);
    sqlite3_bind_double(stmt, 2, billAmount);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL error (inserting payment): " << sqlite3_errmsg(db) << endl;
        message = "\033[31mSQL error (inserting payment)\033[0m"; // Red text
    } else {
        message = "\033[32mPayment verified and recorded successfully!\033[0m"; // Green text
    }

    sqlite3_finalize(stmt);
    return message;
}

// Register patients
void registerPatient(sqlite3 *db)
{
    char *errMsg = 0;
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
    if (rc != SQLITE_OK)
    {
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
          "VALUES ('" +
          fName + "', '" + lName + "', " + to_string(age) + ", '" + sex + "', '" + allergies + "', '" + nextOfKinTelNo + "', '" + patientTelNo + "', '" + location + "');";

    rc = sqlite3_exec(db, sql.c_str(), 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error (insertion): " << errMsg << endl;
        sqlite3_free(errMsg);
    }
    else
    {
        cout << "Patient registered successfully!" << endl;
    }
}

bool parseDateTime(const std::string &dateTimeStr, struct tm &tm)
{
    std::istringstream ss(dateTimeStr);
    ss >> std::get_time(&tm, "%Y-%m-%d %H:%M");
    return !ss.fail();
}


void createAppointmentsTable(sqlite3 *db)
{
    const char *sql = R"sql(
        CREATE TABLE IF NOT EXISTS Appointments(
            ID INTEGER PRIMARY KEY AUTOINCREMENT,
            DoctorID INTEGER NOT NULL,
            DoctorName TEXT NOT NULL,
            PatientID INTEGER NOT NULL UNIQUE,
            PatientName TEXT NOT NULL,
            AppointmentDate TEXT NOT NULL,
            AppointmentTime TEXT NOT NULL,
            FOREIGN KEY(DoctorID) REFERENCES staff(ID),
            FOREIGN KEY(PatientID) REFERENCES Patients(ID)
        );
    )sql";

    char *errMsg = 0;
    int rc = sqlite3_exec(db, sql, 0, 0, &errMsg);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error (creating appointments table): " << errMsg << endl;
        sqlite3_free(errMsg);
    }
}

// Book Appointment
void createAppointment(sqlite3 *db, string &errorMessage)
{
    // Ensure the Appointments table exists
    createAppointmentsTable(db);

    const char *sql = "SELECT ID, fName, lName, specialization FROM staff WHERE specialization != 'receptionist'";
    sqlite3_stmt *stmt;
    vector<string> doctors;
    char *errMsg = 0;

    while (true)
    {
        if (!errorMessage.empty())
        {
            cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Display error message in red
            cout << "Please try again." << endl;
            errorMessage.clear(); // Clear the error message after displaying it
        }

        int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            cerr << "SQL error (retrieving doctors): " << sqlite3_errmsg(db) << endl;
            return;
        }

        doctors.clear();
        cout << "Available Doctors:" << endl;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            stringstream doctor;
            doctor << "ID: " << sqlite3_column_int(stmt, 0) << ", "
                   << "Name: " << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)) << " "
                   << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2)) << ", "
                   << "Specialization: " << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            doctors.push_back(doctor.str());
        }

        sqlite3_finalize(stmt);

        if (doctors.empty())
        {
            cout << "No doctors available." << endl;
            return;
        }

        for (const auto &doctor : doctors)
        {
            cout << doctor << endl;
        }

        int doctorID;
        cout << "Enter the ID of the doctor for the appointment: ";
        cin >> doctorID;

        // Check if the doctor exists and retrieve the doctor's name
        string doctorName;
        bool doctorExists = false;
        for (const auto &doctor : doctors)
        {
            if (doctor.find("ID: " + to_string(doctorID)) != string::npos)
            {
                doctorExists = true;
                size_t nameStart = doctor.find("Name: ") + 6;
                size_t nameEnd = doctor.find(", Specialization:");
                doctorName = doctor.substr(nameStart, nameEnd - nameStart);
                break;
            }
        }

        if (!doctorExists)
        {
            errorMessage = "Error: Doctor with ID " + to_string(doctorID) + " does not exist.";
            continue;
        }

        // Retrieve patients from the Patients table
        sql = "SELECT ID, fName, lName FROM Patients";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            cerr << "SQL error (retrieving patients): " << sqlite3_errmsg(db) << endl;
            return;
        }

        vector<string> patients;
        cout << "Available Patients:" << endl;
        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            stringstream patient;
            patient << "ID: " << sqlite3_column_int(stmt, 0) << ", "
                    << "Name: " << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1)) << " "
                    << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2));
            patients.push_back(patient.str());
        }

        sqlite3_finalize(stmt);

        if (patients.empty())
        {
            cout << "No patients available." << endl;
            return;
        }

        for (const auto &patient : patients)
        {
            cout << patient << endl;
        }

        int patientID;
        cout << "Enter the ID of the patient for the appointment: ";
        cin >> patientID;

        // Check if the patient exists and retrieve the patient's name
        string patientName;
        bool patientExists = false;
        for (const auto &patient : patients)
        {
            if (patient.find("ID: " + to_string(patientID)) != string::npos)
            {
                patientExists = true;
                size_t nameStart = patient.find("Name: ") + 6;
                patientName = patient.substr(nameStart);
                break;
            }
        }

        if (!patientExists)
        {
            errorMessage = "Error: Patient with ID " + to_string(patientID) + " does not exist.";
            continue;
        }

        // Check if the patient already has an appointment
        sql = "SELECT COUNT(*) FROM Appointments WHERE PatientID = ?";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            cerr << "SQL error (checking existing appointment): " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_int(stmt, 1, patientID);
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0)
        {
            errorMessage = "Error: This patient already has an appointment.";
            sqlite3_finalize(stmt);
            continue;
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
        if (!parseDateTime(appointmentDate + " " + appointmentTime, tm))
        {
            errorMessage = "Error: Invalid date/time format.";
            continue;
        }

        // Check if the date/time is in the past
        time_t now = time(0);
        time_t appointmentTimeT = mktime(&tm);
        if (difftime(appointmentTimeT, now) < 0)
        {
            errorMessage = "Error: Appointment date/time is in the past.";
            continue;
        }

        // Check if the appointment slot is already taken
        sql = "SELECT COUNT(*) FROM Appointments WHERE AppointmentDate = ? AND AppointmentTime = ?";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
        if (rc != SQLITE_OK)
        {
            cerr << "SQL error (checking appointment slot): " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_text(stmt, 1, appointmentDate.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, appointmentTime.c_str(), -1, SQLITE_TRANSIENT);
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0)
        {
            errorMessage = "Error: This appointment slot is already taken.";
            sqlite3_finalize(stmt);
            continue;
        }
        sqlite3_finalize(stmt);

        stringstream insertSql;
        insertSql << "INSERT INTO Appointments (DoctorID, DoctorName, PatientID, PatientName, AppointmentDate, AppointmentTime) VALUES ("
                  << doctorID << ", '" << doctorName << "', " << patientID << ", '" << patientName << "', '" << appointmentDate << "', '" << appointmentTime << "');";

        rc = sqlite3_exec(db, insertSql.str().c_str(), 0, 0, &errMsg);
        if (rc != SQLITE_OK)
        {
            cerr << "SQL error (inserting appointment): " << errMsg << endl;
            sqlite3_free(errMsg);
        }
        else
        {
            cout << "Appointment created successfully!" << endl;
            break;
        }
    }
}

// Admin code:
void adminMenu(sqlite3 *db) {
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

    while (true) {
        printMenu(adminMenuOptions);

        if (!errorMessage.empty()) {
            cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Print error message in red
        }

        if (!lastChoice.empty()) {
            cout << lastChoice << endl;
        }

        cout << "Enter your choice: ";
        cin >> choice;

        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            errorMessage = "Your choice is invalid";
            choice = 0;
        } else if (choice < 1 || choice > 5) {
            errorMessage = "Invalid choice. Please choose 1, 2, 3, 4, or 5.";
        } else {
            errorMessage.clear(); // Clear the error message if the input is valid
            switch (choice) {
                case 1:
                    staffReg(db);
                    break;
                case 2:
                    viewStaffRecords(db);
                    break;
                case 3:
                    viewPatientRecords(db);
                    break;
                case 4:
                    financialAnalysis(db);
                    break;
                case 5:
                    lastChoice = "Returning to main menu";
                    return;
            }
        }
    }
}

// Admin auth:
bool authenticateAdmin(const string &username, const string &password)
{
    const string adminUser = "admin";
    const string adminPass = "admin";
    return (username == adminUser && password == adminPass);
}

// Staff regestration function DB:
void staffReg(sqlite3 *db)
{
    // Create table if it doesn't exist
    const char *createTableSQL = R"sql(
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

    char *errMsg = 0;
    int rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);

    if (rc != SQLITE_OK)
    {
        cerr << "SQL error (table creation): " << errMsg << endl;
        sqlite3_free(errMsg);
        return;
    }
    else
    {
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
    const char *insertSQL = R"sql(
        INSERT INTO staff (username, password, fName, lName, specialization, salary, idNumber, age, telNo, location)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )sql";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
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
    if (rc != SQLITE_DONE)
    {
        cerr << "SQL error (insertion): " << sqlite3_errmsg(db) << endl;
    }
    else
    {
        cout << "Staff registered successfully!" << endl;
    }

    // Finalize the statement
    sqlite3_finalize(stmt);
}

// View staff records:
void printRecords(const vector<string> &menu)
{
    system("cls");     // Clear the console
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
    for (const auto &record : menu)
    {
        cout << record << endl;
    }

    cout << "\033[0m"; // Reset text formatting
}

void viewStaffRecords(sqlite3 *db)
{
    const char *sql = "SELECT * FROM staff";
    sqlite3_stmt *stmt;
    vector<string> menu;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error (retrieving records): " << sqlite3_errmsg(db) << endl;
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        stringstream record;
        record << left << setw(5) << sqlite3_column_int(stmt, 0)
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3))
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4))
               << left << setw(20) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5))
               << left << setw(10) << to_string(sqlite3_column_int(stmt, 6))
               << left << setw(5) << to_string(sqlite3_column_int(stmt, 8))
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 9))
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 10))
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 11));
        menu.push_back(record.str());
    }

    sqlite3_finalize(stmt);

    while (true)
    {
        printRecords(menu);
        cout << endl;
        cout << endl;
        cout << "==|| Press 'b' to go back to the Admin menu: ||==" << endl;
        char choice;
        cin >> choice;
        if (choice == 'b' || choice == 'B')
        {
            break;
        }
    }
}

// Patients records:
void printPatientRecords(const vector<string> &menu)
{
    system("cls");     // Clear the console
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
    for (const auto &record : menu)
    {
        cout << record << endl;
    }

    cout << "\033[0m"; // Reset text formatting
}

void viewPatientRecords(sqlite3 *db)
{
    const char *sql = "SELECT * FROM Patients";
    sqlite3_stmt *stmt;
    vector<string> menu;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK)
    {
        cerr << "SQL error (retrieving records): " << sqlite3_errmsg(db) << endl;
        return;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        stringstream record;
        record << left << setw(5) << sqlite3_column_int(stmt, 0)                                    // ID
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1))  // fName
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 2))  // lName
               << left << setw(5) << sqlite3_column_int(stmt, 3)                                    // Age
               << left << setw(10) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4))  // Sex
               << left << setw(20) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5))  // Allergies
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 6))  // NextOfKinTelNo
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 7))  // PatientTelNo
               << left << setw(15) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 8))  // Location
               << left << setw(20) << reinterpret_cast<const char *>(sqlite3_column_text(stmt, 9)); // registrationDate
        menu.push_back(record.str());
    }

    sqlite3_finalize(stmt);

    while (true)
    {
        printPatientRecords(menu);
        cout << endl;
        cout << endl;
        cout << "==|| Press 'b' to go back to the Admin menu: ||==" << endl;
        char choice;
        cin >> choice;
        if (choice == 'b' || choice == 'B')
        {
            break;
        }
    }
}

// Financial analysis:
void financialAnalysis(sqlite3 *db) {
    const int investedMoney = 10000000;
    const int tax = 100000;
    const char *sql = "SELECT SUM(salary) FROM staff";
    sqlite3_stmt *stmt;
    int totalSalary = 0;

    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (retrieving total salary): " << sqlite3_errmsg(db) << endl;
        return;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        totalSalary = sqlite3_column_int(stmt, 0);
    } else {
        cerr << "Error: Could not retrieve total salary." << endl;
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    int profit = investedMoney - (totalSalary + tax);

    // Create FinancialAnalysis table if it doesn't exist
    const char *createTableSQL = R"sql(
        CREATE TABLE IF NOT EXISTS FinancialAnalysis (
            Year INTEGER PRIMARY KEY,
            InvestedMoney INTEGER NOT NULL,
            TotalSalary INTEGER NOT NULL,
            Tax INTEGER NOT NULL,
            Profit INTEGER NOT NULL
        );
    )sql";

    rc = sqlite3_exec(db, createTableSQL, 0, 0, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (creating FinancialAnalysis table): " << sqlite3_errmsg(db) << endl;
        return;
    }

    // Insert financial data into FinancialAnalysis table
    const char *insertSQL = "INSERT INTO FinancialAnalysis (Year, InvestedMoney, TotalSalary, Tax, Profit) VALUES (strftime('%Y', 'now'), ?, ?, ?, ?)";
    rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (preparing insert statement): " << sqlite3_errmsg(db) << endl;
        return;
    }

    sqlite3_bind_int(stmt, 1, investedMoney);
    sqlite3_bind_int(stmt, 2, totalSalary);
    sqlite3_bind_int(stmt, 3, tax);
    sqlite3_bind_int(stmt, 4, profit);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cerr << "SQL error (inserting financial data): " << sqlite3_errmsg(db) << endl;
    } else {
        cout << "Financial analysis data recorded successfully!" << endl;
    }

    sqlite3_finalize(stmt);

    // Display the financial analysis data
    const char *selectSQL = "SELECT * FROM FinancialAnalysis";
    rc = sqlite3_prepare_v2(db, selectSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "SQL error (retrieving financial analysis data): " << sqlite3_errmsg(db) << endl;
        return;
    }

    system("cls");     // Clear the console
    cout << "\033[1m"; // Set text to bold

    // Print table header
    cout << left << setw(10) << "Year"
         << left << setw(15) << "Invested Money"
         << left << setw(15) << "Total Salary"
         << left << setw(10) << "Tax"
         << left << setw(10) << "Profit"
         << endl;

    cout << string(60, '=') << endl; // Print a separator line

    // Print each financial record
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        cout << left << setw(10) << sqlite3_column_int(stmt, 0)
             << left << setw(15) << sqlite3_column_int(stmt, 1)
             << left << setw(15) << sqlite3_column_int(stmt, 2)
             << left << setw(10) << sqlite3_column_int(stmt, 3)
             << left << setw(10) << sqlite3_column_int(stmt, 4)
             << endl;
    }

    cout << "\033[0m"; // Reset text formatting

    sqlite3_finalize(stmt);

    cout << "==|| Press 'b' to go back to the Admin menu ||==" << endl;
    char choice;
    cin >> choice;
    if (choice == 'b' || choice == 'B') {
        return;
    }
}

// Main function code
int main()
{
    sqlite3 *db;
    int rc = sqlite3_open("hospital.db", &db);
    if (rc)
    {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }
    else
    {
        cout << "Opened database successfully" << endl;
    }

    while (true)
    {
        int choice = 0;
        string errorMessage;
        vector<string> mainMenuOptions = {
            "Choose your role in Nairobi International Hospital:",
            "1. Administrator",
            "2. Doctor",
            "3. Receptionist",
            "4. Exit" // Add an exit option
        };

        while (choice < 1 || choice > 4)
        {
            printMenu(mainMenuOptions);

            if (!errorMessage.empty())
            {
                cout << "\033[31m" << errorMessage << "\033[0m" << endl; // Print error message in red
            }

            cin >> choice;

            if (cin.fail())
            { // Check if the input is invalid
                errorMessage = "Your choice is invalid";
                cin.clear();                                         // Clear the error flag
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
                choice = 0;                                          // Reset choice to continue the loop
            }
            else if (choice < 1 || choice > 4)
            {
                errorMessage = "Invalid choice. Please choose 1, 2, 3, or 4.";
            }
            else
            {
                errorMessage.clear(); // Clear the error message if the input is valid
            }
        }

        string result = mainMenu(choice, db);
        cout << result << endl;

        if (choice == 4)
        {
            break; // Exit the loop and terminate the program
        }

        system("cls"); // Clear the console
    }

    sqlite3_close(db);
    return 0;
}

string mainMenu(int choice, sqlite3 *db)
{
    switch (choice)
    {
    case 1:
    {
        string username, password;
        cout << "Enter admin username: ";
        cin >> username;
        cout << "Enter admin password: ";
        cin >> password;
        if (authenticateAdmin(username, password))
        {
            adminMenu(db);
            return "Admin menu accessed";
        }
        else
        {
            return "Authentication failed";
        }
    }
    case 2:
    {
        string username, password;
        cout << "Enter doctor username: ";
        cin >> username;
        cout << "Enter doctor password: ";
        cin >> password;
        int doctorID = authenticateUser(db, username, password, "doctor");
        if (doctorID != -1)
        {
            doctorMenu(db, doctorID);
            return "Doctor menu accessed";
        }
        else
        {
            return "Authentication failed";
        }
    }
    case 3:
    {
        string username, password;
        cout << "Enter receptionist username: ";
        cin >> username;
        cout << "Enter receptionist password: ";
        cin >> password;
        if (authenticateUser(db, username, password, "receptionist") != -1)
        {
            receptionistMenu(db);
            return "Receptionist menu accessed";
        }
        else
        {
            return "Authentication failed";
        }
    }
    case 4:
        return "Exiting...";
    default:
        return "Invalid choice";
    }
}