#include "../include/Sales.h"
#include "../include/db.h"
#include "../include/screen.h"

#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static std::string getTodayDate() {
    std::time_t now = std::time(nullptr);
    std::tm localTime{};

#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d");
    return oss.str();
}

static bool isExpired(const std::string& expiryDate) {
    // Works because date format is YYYY-MM-DD
    return expiryDate < getTodayDate();
}

static void displayItems(const std::vector<Item>& inventory) {
    std::cout << "Available Items\n";
    std::cout << "------------------------------------------------------------\n";
    std::cout << "ID\tName\t\tQty\tPrice\tExpiry Date\n";
    std::cout << "------------------------------------------------------------\n";

    for (const Item& item : inventory) {
        std::cout << item.id << "\t"
                  << item.name << "\t\t"
                  << item.quantity << "\t"
                  << item.price << "\t"
                  << item.expiryDate;

        if (item.quantity <= 0) {
            std::cout << "  [OUT OF STOCK]";
        } else if (isExpired(item.expiryDate)) {
            std::cout << "  [EXPIRED]";
        }

        std::cout << '\n';
    }

    std::cout << "------------------------------------------------------------\n";
}

static Item* findItemById(std::vector<Item>& inventory, int itemID) {
    for (Item& item : inventory) {
        if (item.id == itemID) {
            return &item;
        }
    }

    return nullptr;
}

void showSalesMenu() {
    while (true) {
        clearScreen();

        std::vector<Item> inventory;
        int nextId = 1;

        if (!loadItemsFromDatabase(inventory, nextId)) {
            std::cout << "Failed to load items from database.\n";
            pauseMenu();
            return;
        }

        std::cout << "=== Sales ===\n\n";

        if (inventory.empty()) {
            std::cout << "No items available in inventory.\n";
            pauseMenu();
            return;
        }

        displayItems(inventory);

        std::cout << "\nEnter item ID to sell (0 to go back): ";

        int itemID;
        if (!(std::cin >> itemID)) {
            clearInput();
            std::cout << "Invalid input. Please enter a number.\n";
            pauseMenu();
            continue;
        }

        if (itemID == 0) {
            return;
        }

        Item* selectedItem = findItemById(inventory, itemID);

        if (selectedItem == nullptr) {
            clearInput();
            std::cout << "Item not found.\n";
            pauseMenu();
            continue;
        }

        if (selectedItem->quantity <= 0) {
            clearInput();
            std::cout << "This item is out of stock.\n";
            pauseMenu();
            continue;
        }

        if (isExpired(selectedItem->expiryDate)) {
            clearInput();
            std::cout << "This item is expired and cannot be sold.\n";
            pauseMenu();
            continue;
        }

        std::cout << "\nSelected Item: " << selectedItem->name << '\n';
        std::cout << "Available Quantity: " << selectedItem->quantity << '\n';
        std::cout << "Enter quantity to sell: ";

        int quantityToSell;
        if (!(std::cin >> quantityToSell)) {
            clearInput();
            std::cout << "Invalid quantity. Please enter a number.\n";
            pauseMenu();
            continue;
        }

        if (quantityToSell <= 0) {
            clearInput();
            std::cout << "Quantity must be greater than 0.\n";
            pauseMenu();
            continue;
        }

        if (quantityToSell > selectedItem->quantity) {
            clearInput();
            std::cout << "Not enough stock.\n";
            std::cout << "Available quantity: " << selectedItem->quantity << '\n';
            pauseMenu();
            continue;
        }

        bool success = insertSale(
            0,                    // 0 means use default admin user from db.cpp
            selectedItem->id,
            quantityToSell,
            ""                    // empty means use today's date in database
        );

        clearInput();

        if (success) {
            std::cout << "\nSale completed successfully.\n";
            std::cout << "Item: " << selectedItem->name << '\n';
            std::cout << "Quantity sold: " << quantityToSell << '\n';
            std::cout << "Remaining stock: "
                      << selectedItem->quantity - quantityToSell << '\n';
        } else {
            std::cout << "\nFailed to complete sale.\n";
            std::cout << "The item may no longer have enough stock.\n";
        }

        pauseMenu();
    }
}