#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <sstream>
#include <unordered_map>
#include <cmath>
#include <map>
#include <set>

using namespace std;

// Structure to represent a user and store their health and dietary data.
struct User {
    string name;          // Name of the user
    int age;              // Age of the user
    double weight;        // Weight of the user in kilograms
    double height;        // Height of the user in meters
    bool isVegetarian;    // Whether the user is vegetarian
    mutable double cachedBMI = -1; // Cached BMI value for optimization

    // Function to calculate BMI (Body Mass Index) and cache it for future use.
    double calculateBMI() const {
        if (cachedBMI < 0) {
            cachedBMI = weight / (height * height);
        }
        return cachedBMI;
    }

    // Function to determine the health category based on BMI value.
    string healthCategory() const {
        double bmi = calculateBMI();
        if (bmi < 18.5) return "Underweight";
        if (bmi < 24.9) return "Normal weight";
        if (bmi < 29.9) return "Overweight";
        return "Obese";
    }
};

// TrieNode class to represent a node in the Trie data structure for string storage and search.
class TrieNode {
public:
    unordered_map<char, TrieNode*> children; // Map of child nodes
    bool isEndOfWord = false;                // Indicates if this node marks the end of a word

    ~TrieNode() {
        for (auto& child : children) {
            delete child.second; // Recursively delete child nodes
        }
    }
};

// Trie class to manage insertion and prefix-based suggestions.
class Trie {
private:
    TrieNode* root; // Root node of the Trie

public:
    Trie() : root(new TrieNode()) {}

    ~Trie() {
        delete root;
    }

    // Function to insert a word into the Trie.
    void insert(const string& word) {
        TrieNode* node = root;
        for (char c : word) {
            if (!node->children[c]) {
                node->children[c] = new TrieNode();
            }
            node = node->children[c];
        }
        node->isEndOfWord = true;
    }

    // Function to suggest words that match a given prefix.
    vector<string> suggest(const string& prefix) {
        TrieNode* node = root;
        for (char c : prefix) {
            if (!node->children[c]) {
                return {}; // No suggestions available
            }
            node = node->children[c];
        }
        vector<string> suggestions;
        findSuggestions(node, prefix, suggestions);
        return suggestions;
    }

private:
    // Helper function to recursively find all words from a given TrieNode.
    void findSuggestions(TrieNode* node, string current, vector<string>& suggestions) {
        if (node->isEndOfWord) {
            suggestions.push_back(current);
        }
        for (auto& child : node->children) {
            findSuggestions(child.second, current + child.first, suggestions);
        }
    }
};

// FitPlanAssistant class to manage users, their data, and meal plans.
class FitPlanAssistant {
private:
    vector<User> users;                           // List of all users
    unordered_map<string, vector<string>> mealPlans; // Meal plans categorized by diet type
    map<string, set<User*>> bmiCategories;       // Map to categorize users by BMI
    Trie userTrie;                               // Trie for user name suggestions
    const string dataFile = "users_data.txt";   // File to store user data

    // Function to load user data from a file.
    void loadUserData() {
        ifstream file(dataFile);
        if (!file) return; // If file doesn't exist, return

        string line;
        while (getline(file, line)) {
            istringstream iss(line);
            User user;
            string isVeg;
            if (iss >> user.name >> user.age >> user.weight >> user.height >> isVeg) {
                user.isVegetarian = (isVeg == "1");
                users.push_back(user);
                userTrie.insert(user.name);
            }
        }
        file.close();
        categorizeUsersByBMI();
    }

    // Function to save user data to a file.
    void saveUserData() {
        ofstream file(dataFile);
        for (const auto& user : users) {
            file << user.name << " " << user.age << " " << user.weight << " " << user.height << " " << user.isVegetarian << endl;
        }
        file.close();
    }

    // Function to initialize default meal plans.
    void initializeMealPlans() {
        mealPlans["Vegetarian"] = {
            "Oatmeal with fruits",
            "Grilled vegetable sandwich",
            "Lentil soup and quinoa",
            "Vegetarian stir-fry"
        };

        mealPlans["Non-Vegetarian"] = {
            "Scrambled eggs and toast",
            "Chicken salad",
            "Grilled salmon with veggies",
            "Beef stir-fry with rice"
        };
    }

    // Function to categorize users into BMI categories.
    void categorizeUsersByBMI() {
        bmiCategories.clear();
        for (auto& user : users) {
            bmiCategories[user.healthCategory()].insert(&user);
        }
    }

    // Function to display detailed information about a user.
    void displayUserDetails(const User& user) {
        cout << "\nUser Details:\n";
        cout << "Name: " << user.name << "\n";
        cout << "Age: " << user.age << "\n";
        cout << "Weight: " << user.weight << " kg\n";
        cout << "Height: " << user.height << " m\n";
        cout << "BMI: " << fixed << setprecision(2) << user.calculateBMI() << " (" << user.healthCategory() << ")\n";
        cout << "Diet Preference: " << (user.isVegetarian ? "Vegetarian" : "Non-Vegetarian") << "\n";
    }

    // Function to recommend meal plans based on the user's diet preference.
    void recommendMealPlan(const User& user) {
        string dietType = user.isVegetarian ? "Vegetarian" : "Non-Vegetarian";
        cout << "\nRecommended Meals (" << dietType << "):\n";
        for (const auto& meal : mealPlans[dietType]) {
            cout << "- " << meal << endl;
        }
    }

public:
    // Constructor to initialize the system and load data.
    FitPlanAssistant() {
        loadUserData();
        initializeMealPlans();
    }

    // Destructor to save user data before exiting.
    ~FitPlanAssistant() {
        saveUserData();
    }

    // Function to add a new user to the system.
    void addUser() {
        User newUser;
        cout << "Enter name: ";
        cin >> newUser.name;
        cout << "Enter age: ";
        cin >> newUser.age;
        cout << "Enter weight (kg): ";
        cin >> newUser.weight;
        cout << "Enter height (m): ";
        cin >> newUser.height;

        char isVeg;
        cout << "Are you vegetarian? (y/n): ";
        cin >> isVeg;
        newUser.isVegetarian = (isVeg == 'y' || isVeg == 'Y');

        users.push_back(newUser);
        userTrie.insert(newUser.name);
        categorizeUsersByBMI();
        cout << "User added successfully!\n";
    }

    // Function to view details of a user by name or prefix.
    void viewUser() {
        string prefix;
        cout << "Enter the name or prefix of the user to view: ";
        cin >> prefix;

        vector<string> suggestions = userTrie.suggest(prefix);
        if (suggestions.empty()) {
            cout << "No users found with the given prefix!\n";
            return;
        }

        cout << "\nDid you mean:\n";
        for (const auto& suggestion : suggestions) {
            cout << "- " << suggestion << endl;
        }

        string name;
        cout << "Enter the full name: ";
        cin >> name;

        auto it = find_if(users.begin(), users.end(), [&name](const User& user) {
            return user.name == name;
        });

        if (it != users.end()) {
            displayUserDetails(*it);
            recommendMealPlan(*it);
        } else {
            cout << "User not found!\n";
        }
    }

    // Function to list all users categorized by their BMI.
    void listUsersByBMICategory() {
        cout << "\nUsers by BMI Category:\n";
        for (const auto& [category, usersSet] : bmiCategories) {
            cout << "\n" << category << ":\n";
            for (const auto* user : usersSet) {
                cout << "- " << user->name << " (BMI: " << fixed << setprecision(2) << user->calculateBMI() << ")\n";
            }
        }
    }

    // Main menu for user interaction.
    void mainMenu() {
        while (true) {
            cout << "\n=== Health Management System ===\n";
            cout << "1. Add User\n";
            cout << "2. View User\n";
            cout << "3. List Users by BMI Category\n";
            cout << "4. Exit\n";

            int choice;
            cout << "Enter your choice: ";
            cin >> choice;

            switch (choice) {
                case 1:
                    addUser();
                    break;
                case 2:
                    viewUser();
                    break;
                case 3:
                    listUsersByBMICategory();
                    break;
                case 4:
                    cout << "Exiting the system. Goodbye!\n";
                    return;
                default:
                    cout << "Invalid choice. Please try again.\n";
            }
        }
    }
};

int main() {
    //Instantiate the FitPlan Assistant and display the main menu.
    FitPlanAssistant system;
    system.mainMenu();
    return 0;
}