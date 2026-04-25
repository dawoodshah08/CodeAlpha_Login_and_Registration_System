#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <conio.h>
#include <cctype>
#include <limits>
#include <cstdlib>
#include <windows.h>

using namespace std;

// Predefined Color Constants
const int RED = 12;
const int GREEN = 10;
const int YELLOW = 14;
const int WHITE = 15;
const int CYAN = 11;

// Handle for console color changes
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

// System Functions
void clearScreen() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Helper Functions
string toLower(const string& str) {
    string result = str;
    for (char& c : result) {
        c = tolower(c);
    }
    return result;
}

string hashPassword(const string& password) {
    unsigned long hash = 5381;
    for (char c : password) {
        hash = ((hash << 5) + hash) + c;
    }
    return to_string(hash);
}

string getHiddenInput(const string& prompt) {
    cout << prompt;
    cout.flush();
    
    string input;
    char ch;
    
    while (true) {
        ch = _getch();
        
        if (ch == '\r') {
            break;
        }
        else if (ch == '\b') {
            if (!input.empty()) {
                input.pop_back();
                cout << "\b \b";
                cout.flush();
            }
        }
        else if (isprint(ch)) {
            input += ch;
            cout << '*';
            cout.flush();
        }
    }
    
    cout << endl;
    return input;
}

void clearInputBuffer() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void waitForUser() {
    cout << "\nPress any key to continue...";
    _getch();
    clearScreen();
}

// Validation Functions
struct ValidationResult {
    bool isValid;
    string errorMessage;
};

ValidationResult validateUsername(const string& username) {
    if (username.empty()) {
        return {false, "Username cannot be empty."};
    }
    
    if (username.length() < 3) {
        return {false, "Username must be at least 3 characters long."};
    }
    
    if (username.length() > 20) {
        return {false, "Username cannot exceed 20 characters."};
    }
    
    for (char c : username) {
        if (!isalnum(c)) {
            return {false, "Username can only contain letters and numbers."};
        }
    }
    
    return {true, ""};
}

ValidationResult validatePassword(const string& password) {
    if (password.empty()) {
        return {false, "Password cannot be empty."};
    }
    
    if (password.length() < 6) {
        return {false, "Password must be at least 6 characters long."};
    }
    
    if (password.length() > 50) {
        return {false, "Password cannot exceed 50 characters."};
    }
    
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    
    for (char c : password) {
        if (isupper(c)) hasUpper = true;
        if (islower(c)) hasLower = true;
        if (isdigit(c)) hasDigit = true;
    }
    
    if (!hasUpper) {
        return {false, "Password must contain at least one uppercase letter."};
    }
    
    if (!hasLower) {
        return {false, "Password must contain at least one lowercase letter."};
    }
    
    if (!hasDigit) {
        return {false, "Password must contain at least one number."};
    }
    
    return {true, ""};
}

// Database Class
class UserDatabase {
private:
    string filename;
    map<string, string> users;
    
    void loadFromFile() {
        users.clear();
        ifstream file(filename);
        
        if (!file.is_open()) {
            return;
        }
        
        string line;
        while (getline(file, line)) {
            size_t separator = line.find(':');
            if (separator != string::npos) {
                string username = line.substr(0, separator);
                string passwordHash = line.substr(separator + 1);
                users[username] = passwordHash;
            }
        }
        
        file.close();
    }
    
    void saveToFile() {
        ofstream file(filename);
        
        if (!file.is_open()) {
            SetConsoleTextAttribute(hConsole, RED);
            cerr << "Error: Unable to save user data." << endl;
            SetConsoleTextAttribute(hConsole, WHITE);
            return;
        }
        
        for (const auto& entry : users) {
            file << entry.first << ":" << entry.second << endl;
        }
        
        file.close();
    }
    
public:
    UserDatabase(const string& file = "users.txt") : filename(file) {
        loadFromFile();
    }
    
    bool usernameExists(const string& username) {
        string lowerUsername = toLower(username);
        
        for (const auto& entry : users) {
            if (toLower(entry.first) == lowerUsername) {
                return true;
            }
        }
        return false;
    }
    
    bool addUser(const string& username, const string& password) {
        if (usernameExists(username)) {
            return false;
        }
        
        users[username] = hashPassword(password);
        saveToFile();
        return true;
    }
    
    bool authenticateUser(const string& username, const string& password) {
        string lowerUsername = toLower(username);
        
        for (const auto& entry : users) {
            if (toLower(entry.first) == lowerUsername) {
                return entry.second == hashPassword(password);
            }
        }
        return false;
    }
    
    string getOriginalUsername(const string& caseInsensitiveUsername) {
        string lowerInput = toLower(caseInsensitiveUsername);
        
        for (const auto& entry : users) {
            if (toLower(entry.first) == lowerInput) {
                return entry.first;
            }
        }
        return "";
    }
    
    bool isEmpty() {
        return users.empty();
    }
};

// Authentication System Class
class AuthenticationSystem {
private:
    UserDatabase database;
    
    void showHeader(const string& title) {
        cout << endl;
        SetConsoleTextAttribute(hConsole, GREEN);
        cout << "=====================================" << endl;
        cout << "       " << title << endl;
        cout << "=====================================" << endl;
        SetConsoleTextAttribute(hConsole, WHITE);
        cout << endl;
    }
    
    bool registerNewUser() {
        clearScreen();
        showHeader("CREATE NEW ACCOUNT");
        
        string username;
        while (true) {
            cout << "Choose a username: ";
            getline(cin, username);
            
            ValidationResult result = validateUsername(username);
            if (!result.isValid) {
                SetConsoleTextAttribute(hConsole, RED);
                cout << "ERROR: " << result.errorMessage << endl << endl;
                SetConsoleTextAttribute(hConsole, WHITE);
                continue;
            }
            
            if (database.usernameExists(username)) {
                SetConsoleTextAttribute(hConsole, RED);
                cout << "ERROR: Username already taken. Please choose another one." << endl << endl;
                SetConsoleTextAttribute(hConsole, WHITE);
                continue;
            }
            
            break;
        }
        
        string password;
        while (true) {
            password = getHiddenInput("Choose a password: ");
            cout << endl;
            
            ValidationResult result = validatePassword(password);
            if (!result.isValid) {
                SetConsoleTextAttribute(hConsole, RED);
                cout << "ERROR: " << result.errorMessage << endl << endl;
                SetConsoleTextAttribute(hConsole, WHITE);
                continue;
            }
            
            break;
        }
        
        int maxAttempts = 3;
        for (int attempt = 1; attempt <= maxAttempts; attempt++) {
            string confirmPassword = getHiddenInput("Confirm your password: ");
            cout << endl;
            
            if (password == confirmPassword) {
                if (database.addUser(username, password)) {
                    SetConsoleTextAttribute(hConsole, GREEN);
                    cout << "SUCCESS! Account created successfully." << endl;
                    cout << "Welcome, " << username << "!" << endl;
                    SetConsoleTextAttribute(hConsole, WHITE);
                    waitForUser();
                    return true;
                } else {
                    SetConsoleTextAttribute(hConsole, RED);
                    cout << "ERROR: Failed to create account. Please try again." << endl;
                    SetConsoleTextAttribute(hConsole, WHITE);
                    waitForUser();
                    return false;
                }
            } else {
                SetConsoleTextAttribute(hConsole, RED);
                cout << "ERROR: Passwords do not match. ";
                SetConsoleTextAttribute(hConsole, WHITE);
                if (attempt < maxAttempts) {
                    cout << "Attempts remaining: " << (maxAttempts - attempt) << endl << endl;
                } else {
                    cout << "No more attempts. Registration cancelled." << endl;
                    waitForUser();
                    return false;
                }
            }
        }
        
        return false;
    }
    
    bool loginExistingUser() {
        clearScreen();
        showHeader("LOGIN PORTAL");
        
        if (database.isEmpty()) {
            SetConsoleTextAttribute(hConsole, YELLOW);
            cout << "NOTE: No accounts found. Please register first." << endl;
            SetConsoleTextAttribute(hConsole, WHITE);
            waitForUser();
            return false;
        }
        
        string username;
        cout << "Username: ";
        getline(cin, username);
        
        if (username.empty()) {
            SetConsoleTextAttribute(hConsole, RED);
            cout << "ERROR: Username cannot be empty." << endl;
            SetConsoleTextAttribute(hConsole, WHITE);
            waitForUser();
            return false;
        }
        
        string originalUsername = database.getOriginalUsername(username);
        if (originalUsername.empty()) {
            SetConsoleTextAttribute(hConsole, RED);
            cout << "ERROR: No account found with username '" << username << "'." << endl;
            SetConsoleTextAttribute(hConsole, WHITE);
            waitForUser();
            return false;
        }
        
        int maxAttempts = 3;
        for (int attempt = 1; attempt <= maxAttempts; attempt++) {
            string password = getHiddenInput("Password: ");
            cout << endl;
            
            if (database.authenticateUser(originalUsername, password)) {
                SetConsoleTextAttribute(hConsole, GREEN);
                cout << "LOGIN SUCCESSFUL!" << endl;
                cout << "Welcome back, " << originalUsername << "!" << endl;
                SetConsoleTextAttribute(hConsole, WHITE);
                waitForUser();
                return true;
            } else {
                SetConsoleTextAttribute(hConsole, RED);
                cout << "ERROR: Incorrect password. ";
                SetConsoleTextAttribute(hConsole, WHITE);
                if (attempt < maxAttempts) {
                    cout << "Attempts remaining: " << (maxAttempts - attempt) << endl << endl;
                } else {
                    cout << "Too many failed attempts. Access denied." << endl;
                    waitForUser();
                }
            }
        }
        
        return false;
    }
    
public:
    void run() {
        while (true) {
            clearScreen();
            cout << endl;
            SetConsoleTextAttribute(hConsole, CYAN);
            cout << "=========================================" << endl;
            cout << "      SECURE ACCESS SYSTEM              " << endl;
            cout << "=========================================" << endl;
            SetConsoleTextAttribute(hConsole, WHITE);
            cout << "  1. Register New Account               " << endl;
            cout << "  2. Login to Existing Account          " << endl;
            cout << "  3. Exit System                        " << endl;
            SetConsoleTextAttribute(hConsole, CYAN);
            cout << "=========================================" << endl;
            SetConsoleTextAttribute(hConsole, WHITE);
            cout << endl << "Enter your choice (1-3): ";
            
            string choice;
            getline(cin, choice);
            
            if (choice == "1") {
                registerNewUser();
            } 
            else if (choice == "2") {
                loginExistingUser();
            } 
            else if (choice == "3") {
                clearScreen();
                cout << endl;
                SetConsoleTextAttribute(hConsole, CYAN);
                cout << "=========================================" << endl;
                cout << "   Thank you for using the system!" << endl;
                cout << "   Goodbye!" << endl;
                cout << "=========================================" << endl;
                SetConsoleTextAttribute(hConsole, WHITE);
                break;
            } 
            else {
                SetConsoleTextAttribute(hConsole, RED);
                cout << "ERROR: Invalid choice. Please enter 1, 2, or 3." << endl;
                SetConsoleTextAttribute(hConsole, WHITE);
                waitForUser();
            }
        }
    }
};

// Main Entry Point
int main() {
    clearScreen();
    cout << endl;
    SetConsoleTextAttribute(hConsole, CYAN);
    cout << "=========================================" << endl;
    cout << "                                        " << endl;
    cout << "     WELCOME TO SECURE ACCESS SYSTEM    " << endl;
    cout << "     Enterprise Authentication Portal  " << endl;
    cout << "                                        " << endl;
    cout << "=========================================" << endl;
    SetConsoleTextAttribute(hConsole, WHITE);
    cout << endl;
    cout << "Press any key to continue...";
    _getch();
    
    AuthenticationSystem authSystem;
    authSystem.run();
    
    return 0;
}