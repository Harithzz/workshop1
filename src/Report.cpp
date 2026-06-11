#include "../include/Report.h"
#include "../include/db.h"
#include "../include/screen.h"
#include <iostream>
#include <iomanip>
#include <limits>

using namespace std;

// Helper to print a simple table header
static void printHeader(const string& title) {
    cout << "\n=== " << title << " ===\n";
    cout << left << setw(5) << "ID"
         << setw(25) << "Name"
         << setw(15) << "Category"
         << setw(10) << "Price"
         << setw(12) << "Expiry Date"
         << setw(10) << "Quantity" << endl;
    cout << string(77, '-') << endl;
}

// Low stock report
static void lowStockReport() {
    int threshold;
    cout << "Enter low stock threshold (e.g., 5): ";
    cin >> threshold;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    vector<Item> lowStock;
    if (getLowStockItems(threshold, lowStock)) {
        if (lowStock.empty()) {
            cout << "\nNo items with quantity <= " << threshold << ".\n";
        } else {
            printHeader("Low Stock Items (<= " + to_string(threshold) + ")");
            for (const auto& item : lowStock) {
                cout << left << setw(5) << item.id
                     << setw(25) << item.name
                     << setw(15) << item.category
                     << setw(10) << fixed << setprecision(2) << item.price
                     << setw(12) << item.expiryDate
                     << setw(10) << item.quantity << endl;
            }
        }
    } else {
        cerr << "Failed to retrieve low stock data.\n";
    }
}

// Expiring items report
static void expiringItemsReport() {
    int days;
    cout << "Show items expiring within next (days): ";
    cin >> days;
    cin.ignore();

    vector<Item> expiring;
    if (getExpiringItems(days, expiring)) {
        if (expiring.empty()) {
            cout << "\nNo items expiring within " << days << " days.\n";
        } else {
            printHeader("Items Expiring Within " + to_string(days) + " Days");
            for (const auto& item : expiring) {
                cout << left << setw(5) << item.id
                     << setw(25) << item.name
                     << setw(15) << item.category
                     << setw(10) << fixed << setprecision(2) << item.price
                     << setw(12) << item.expiryDate
                     << setw(10) << item.quantity << endl;
            }
        }
    } else {
        cerr << "Failed to retrieve expiring items.\n";
    }
}

// Sales summary report
static void salesSummaryReport() {
    string fromDate, toDate;
    cout << "Enter start date (YYYY-MM-DD): ";
    cin >> fromDate;
    cout << "Enter end date (YYYY-MM-DD): ";
    cin >> toDate;
    cin.ignore();

    double revenue;
    int itemsSold;
    if (getSalesSummary(fromDate, toDate, revenue, itemsSold)) {
        cout << "\n=== Sales Summary (" << fromDate << " to " << toDate << ") ===\n";
        cout << "Total items sold: " << itemsSold << endl;
        cout << fixed << setprecision(2);
        cout << "Total revenue   : $" << revenue << endl;
    } else {
        cerr << "Failed to retrieve sales summary.\n";
    }
}

// Inventory valuation
static void inventoryValuation() {
    double totalValue;
    if (getInventoryValuation(totalValue)) {
        cout << "\n=== Inventory Valuation ===\n";
        cout << fixed << setprecision(2);
        cout << "Total stock value: $" << totalValue << endl;
    } else {
        cerr << "Failed to compute inventory valuation.\n";
    }
}

// Top selling items
static void topSellingItemsReport() {
    int limit;
    cout << "How many top items to show? ";
    cin >> limit;
    cin.ignore();

    vector<pair<string, int>> items;
    if (getTopSellingItems(limit, items)) {
        if (items.empty()) {
            cout << "\nNo sales records found.\n";
        } else {
            cout << "\n=== Top " << limit << " Selling Items ===\n";
            cout << left << setw(40) << "Item Name" << "Quantity Sold" << endl;
            cout << string(50, '-') << endl;
            for (const auto& [name, qty] : items) {
                cout << left << setw(40) << name << qty << endl;
            }
        }
    } else {
        cerr << "Failed to retrieve top selling items.\n";
    }
}

// Main report menu (called from your main program)
void showReportMenu() {
    int choice;
    do {
        clearScreen();
        cout << "\n========== INVENTORY REPORTS ==========\n";
        cout << "1. Low Stock Items\n";
        cout << "2. Expiring Items\n";
        cout << "3. Sales Summary (by date range)\n";
        cout << "4. Inventory Valuation\n";
        cout << "5. Top Selling Items\n";
        cout << "6. Back to Main Menu\n";
        cout << "========================================\n";
        cout << "Enter your choice: ";

        cin >> choice;
        if (cin.fail()) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            choice = 0;
        }

        switch (choice) {
            case 1: lowStockReport(); break;
            case 2: expiringItemsReport(); break;
            case 3: salesSummaryReport(); break;
            case 4: inventoryValuation(); break;
            case 5: topSellingItemsReport(); break;
            case 6: cout << "Returning to main menu...\n"; break;
            default: cout << "Invalid choice. Please try again.\n";
        }

        if (choice != 6) {
            pauseMenu(); // assumes you have a function that waits for Enter
        }
    } while (choice != 6);
}