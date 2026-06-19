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

//Expired Items Report 
static void expiringItemsReport() {
    vector<Item> expired;
    if (!getExpiredItems(expired)) {
        cerr << "Failed to retrieve expired items.\n";
        waitForEnter();
        return;
    }

    if (expired.empty()) {
        cout << "\nNo expired items.\n";
        waitForEnter();
        return;
    }

    // Compute lost value for each item and find the maximum
    vector<double> lostValues;
    double maxLost = 0.0;
    for (const auto& item : expired) {
        double lost = item.price * item.quantity;
        lostValues.push_back(lost);
        if (lost > maxLost) maxLost = lost;
    }

    // Column widths (adjust as needed)
    const int NAME_WIDTH   = 20;
    const int CAT_WIDTH    = 25;
    const int QTY_WIDTH    = 7;
    const int EXPIRY_WIDTH = 15;
    const int LOST_WIDTH   = 25;
    const int BAR_WIDTH    = 20;

    // Header
    cout << "\n=== Expired Items Report ===\n\n";
    cout << left
         << setw(NAME_WIDTH)   << "Item"
         << setw(CAT_WIDTH)    << "Category"
         << setw(QTY_WIDTH)    << "Qty"
         << setw(EXPIRY_WIDTH) << "Expiry Date"
         << setw(LOST_WIDTH)   << "Lost Value (RM)"
         << "Impact\n";
    cout << string(NAME_WIDTH + CAT_WIDTH + QTY_WIDTH + EXPIRY_WIDTH + LOST_WIDTH + BAR_WIDTH + 8, '-') << "\n";

    // Print each expired item
    for (size_t i = 0; i < expired.size(); ++i) {
        const Item& item = expired[i];
        double lost = lostValues[i];
        int percent = (maxLost > 0) ? static_cast<int>((lost / maxLost) * 100) : 0;

        // Build bar: full blocks for filled, light blocks for empty
        int filled = (percent * BAR_WIDTH) / 100;
        string bar;
        bar.append(filled, '#');                 // filled portion
        bar.append(BAR_WIDTH - filled, '*');     // empty portion

        // Priority label based on lost value percentage
        string label;
        if (percent >= 70)      label = "[High]";
        else if (percent >= 30) label = "[Medium]";
        else                    label = "[Low]";

        cout << left
             << setw(NAME_WIDTH)   << item.name
             << setw(CAT_WIDTH)    << item.category
             << setw(QTY_WIDTH)    << item.quantity
             << setw(EXPIRY_WIDTH) << item.expiryDate
             << setw(LOST_WIDTH)   << fixed << setprecision(2) << lost
             << bar << " " << label << "\n";
    }

    waitForEnter();
}

// 3. Inventory Valuation (total + per item)
static void inventoryValuation() {
    double totalValue;
    vector<InventoryItemValuation> itemDetails;
    if (getInventoryValuationDetails(totalValue, itemDetails)) {
        cout << "\n=== Inventory Report ===\n";
        cout << "Total price for overall remaining stock: RM "
             << fixed << setprecision(2) << totalValue << "\n\n";

        if (!itemDetails.empty()) {
            cout << "Items\n-----\n";
            for (const auto& detail : itemDetails) {
                // Display: name (quantity) : RM value
                cout << detail.name << " (" << detail.quantity << " units): RM "
                     << fixed << setprecision(2) << detail.value << "\n";
            }
        } else {
            cout << "No items with positive quantity.\n";
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