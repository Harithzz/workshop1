#include "../include/ExpiryTrack.h"
#include "../include/db.h"
#include "../include/screen.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

static const int NEAR_EXPIRY_DAYS = 7;

static void clearInput() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

static std::tm getLocalTime(std::time_t timeValue) {
    std::tm result{};

#ifdef _WIN32
    localtime_s(&result, &timeValue);
#else
    localtime_r(&timeValue, &result);
#endif

    return result;
}

static bool parseDate(const std::string& dateText, std::tm& dateOutput) {
    std::istringstream ss(dateText);

    dateOutput = {};
    ss >> std::get_time(&dateOutput, "%Y-%m-%d");

    if (ss.fail()) {
        return false;
    }

    // Use noon to avoid date shifting problems.
    dateOutput.tm_hour = 12;
    dateOutput.tm_min = 0;
    dateOutput.tm_sec = 0;
    dateOutput.tm_isdst = -1;

    return true;
}

static bool getDaysUntilExpiry(const std::string& expiryDate, int& daysOutput) {
    std::tm expiryTm{};

    if (!parseDate(expiryDate, expiryTm)) {
        return false;
    }

    std::time_t now = std::time(nullptr);
    std::tm todayTm = getLocalTime(now);

    todayTm.tm_hour = 12;
    todayTm.tm_min = 0;
    todayTm.tm_sec = 0;
    todayTm.tm_isdst = -1;

    std::time_t todayTime = std::mktime(&todayTm);
    std::time_t expiryTime = std::mktime(&expiryTm);

    if (todayTime == -1 || expiryTime == -1) {
        return false;
    }

    double secondsDifference = std::difftime(expiryTime, todayTime);
    daysOutput = static_cast<int>(secondsDifference / (60 * 60 * 24));

    return true;
}

static std::string getExpiryStatus(int daysUntilExpiry) {
    if (daysUntilExpiry < 0) {
        return "Expired";
    }

    if (daysUntilExpiry == 0) {
        return "Expires today";
    }

    if (daysUntilExpiry <= NEAR_EXPIRY_DAYS) {
        return "Near expiry";
    }

    return "Safe";
}

static void printItemHeader() {
    std::cout
        << std::left
        << std::setw(6) << "ID"
        << std::setw(22) << "Name"
        << std::setw(17) << "Category"
        << std::setw(10) << "Qty"
        << std::setw(14) << "Expiry"
        << std::setw(16) << "Days left"
        << "Status"
        << '\n';

    std::cout << std::string(95, '-') << '\n';
}

static void printItemRow(const Item& item, int daysUntilExpiry, const std::string& status) {
    std::cout
        << std::left
        << std::setw(6) << item.id
        << std::setw(22) << item.name
        << std::setw(17) << item.category
        << std::setw(10) << item.quantity
        << std::setw(14) << item.expiryDate
        << std::setw(16) << daysUntilExpiry
        << status
        << '\n';
}

static bool loadInventory(std::vector<Item>& inventory) {
    int nextId = 1;

    if (!loadItemsFromDatabase(inventory, nextId)) {
        std::cout << "Failed to load items from database.\n";
        return false;
    }

    if (inventory.empty()) {
        std::cout << "No items found in inventory.\n";
        return false;
    }

    return true;
}

static void checkAllExpiryDates() {
    clearScreen();

    std::cout << "=== Check Expiry Date ===\n\n";

    std::vector<Item> inventory;

    if (!loadInventory(inventory)) {
        return;
    }

    printItemHeader();

    for (const Item& item : inventory) {
        int daysUntilExpiry = 0;

        if (!getDaysUntilExpiry(item.expiryDate, daysUntilExpiry)) {
            std::cout
                << std::left
                << std::setw(6) << item.id
                << std::setw(22) << item.name
                << std::setw(17) << item.category
                << std::setw(10) << item.quantity
                << std::setw(14) << item.expiryDate
                << std::setw(16) << "-"
                << "Invalid expiry date"
                << '\n';

            continue;
        }

        printItemRow(item, daysUntilExpiry, getExpiryStatus(daysUntilExpiry));
    }
}

static void showNearExpiryItems() {
    clearScreen();

    std::cout << "=== Near Expiry Items ===\n\n";
    std::cout << "Items expiring within " << NEAR_EXPIRY_DAYS << " days.\n\n";

    std::vector<Item> inventory;

    if (!loadInventory(inventory)) {
        return;
    }

    bool found = false;

    printItemHeader();

    for (const Item& item : inventory) {
        int daysUntilExpiry = 0;

        if (!getDaysUntilExpiry(item.expiryDate, daysUntilExpiry)) {
            continue;
        }

        if (daysUntilExpiry >= 0 && daysUntilExpiry <= NEAR_EXPIRY_DAYS) {
            printItemRow(item, daysUntilExpiry, getExpiryStatus(daysUntilExpiry));
            found = true;
        }
    }

    if (!found) {
        std::cout << "No near expiry items found.\n";
    }
}

static void showExpiredItems() {
    clearScreen();

    std::cout << "=== Expired Items ===\n\n";

    std::vector<Item> inventory;

    if (!loadInventory(inventory)) {
        return;
    }

    bool found = false;

    printItemHeader();

    for (const Item& item : inventory) {
        int daysUntilExpiry = 0;

        if (!getDaysUntilExpiry(item.expiryDate, daysUntilExpiry)) {
            continue;
        }

        if (daysUntilExpiry < 0) {
            printItemRow(item, daysUntilExpiry, "Expired");
            found = true;
        }
    }

    if (!found) {
        std::cout << "No expired items found.\n";
    }
}

void showExpiryTrackingMenu() {
    while (true) {
        clearScreen();

        std::cout << "=== Expiry Tracking ===\n\n";
        std::cout << "1. Check expiry date\n";
        std::cout << "2. Identify near expiry items\n";
        std::cout << "3. Identify expired items\n";
        std::cout << "4. Back to main menu\n\n";
        std::cout << "Choose an option: ";

        int choice;

        if (!(std::cin >> choice)) {
            clearInput();
            std::cout << "Invalid selection. Please enter a number.\n";
            pauseMenu();
            continue;
        }

        switch (choice) {
        case 1:
            checkAllExpiryDates();
            clearInput();
            pauseMenu();
            break;

        case 2:
            showNearExpiryItems();
            clearInput();
            pauseMenu();
            break;

        case 3:
            showExpiredItems();
            clearInput();
            pauseMenu();
            break;

        case 4:
            clearInput();
            return;

        default:
            clearInput();
            std::cout << "Invalid option. Please choose 1 to 4.\n";
            pauseMenu();
            break;
        }
    }
}