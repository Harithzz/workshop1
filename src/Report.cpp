#include "../include/Report.h"
#include "../include/db.h"
#include "../include/screen.h"
#include <iostream>
#include <iomanip>
#include <limits>

using namespace std;

// Helper: wait for Enter key
static void waitForEnter() {
    cout << "\nPress Enter to continue...";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// Helper: print the sales summary table
static void printSalesTable(const vector<SalesItemSummary>& items) {
    cout << "+------------+----------------------+----------+-------------+\n";
    cout << "| Product ID | Name                 | Quantity | Total sales |\n";
    cout << "+------------+----------------------+----------+-------------+\n";
    for (const auto& s : items) {
        cout << "| " << left << setw(10) << s.itemID
             << " | " << setw(20) << s.name
             << " | " << setw(8) << s.quantitySold
             << " | RM " << setw(7) << fixed << setprecision(2) << s.totalSales
             << " |\n";
    }
    cout << "+------------+----------------------+----------+-------------+\n";
}

// 1. Sales Item Summary
static void salesSummaryReport() {
    string fromDate, toDate;
    cout << "Enter start date (YYYY-MM-DD): ";
    cin >> fromDate;
    cout << "Enter end date (YYYY-MM-DD): ";
    cin >> toDate;
    cin.ignore();

    vector<SalesItemSummary> items;
    if (getSalesItemsSummary(fromDate, toDate, items)) {
        if (items.empty()) {
            cout << "\nNo sales found in the given period.\n";
        } else {
            cout << "\n=== Sales Item Summary (" << fromDate << " to " << toDate << ") ===\n";
            printSalesTable(items);
        }
    } else {
        cerr << "Failed to retrieve sales summary.\n";
    }
    waitForEnter();
}

// 2. Expired Items Report (no prompt, already expired)
static void expiringItemsReport() {
    vector<Item> expired;
    if (getExpiredItems(expired)) {
        if (expired.empty()) {
            cout << "\nNo expired items.\n";
        } else {
            cout << "\n=== Expired Items Report ===\n";
            for (const auto& item : expired) {
                double totalValue = item.price * item.quantity;
                int stars = static_cast<int>(totalValue / 100);
                string bar(stars, '*'); 

                cout << item.name << " (" << item.category << "): "
                     << bar << " " << item.quantity << " units "
                     << "(Total lost: RM " << fixed << setprecision(2) << totalValue << ")\n";
            }
        }
    } else {
        cerr << "Failed to retrieve expired items.\n";
    }
    waitForEnter();
}

// 3. Inventory Valuation (total + per item)
static void inventoryValuation() {
    double totalValue;
    vector<pair<string, double>> itemValues;
    if (getInventoryValuationDetails(totalValue, itemValues)) {
        cout << "\n=== Inventory Report ===\n";
        cout << "Total price for overall remaining stock: RM "
             << fixed << setprecision(2) << totalValue << "\n\n";

        if (!itemValues.empty()) {
            cout << "items\n-----\n";
            for (const auto& [name, value] : itemValues) {
                cout << name << ": RM " << fixed << setprecision(2) << value << "\n";
            }
        }
    } else {
        cerr << "Failed to compute inventory report.\n";
    }
    waitForEnter();
}

// Main report menu
void showReportMenu() {
    int choice;
    do {
        clearScreen();
        cout << "\n============= REPORT MENU =============\n";
        cout << "1. Sales Item Summary\n";
        cout << "2. Expiry Item Report\n";
        cout << "3. Inventory Report\n";
        cout << "4. Back to Main Menu\n";
        cout << "=========================================\n";
        cout << "Enter your choice: ";

        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = 0;
        }

        switch (choice) {
            case 1: salesSummaryReport(); break;
            case 2: expiringItemsReport(); break;
            case 3: inventoryValuation(); break;
            case 4: cout << "Returning to main menu...\n"; break;
            default: cout << "Invalid choice. Please try again.\n";
        }
        // No external pause here – each report handles its own pause.
    } while (choice != 4);
}