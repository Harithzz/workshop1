#ifndef DB_H
#define DB_H

#include <string>
#include <vector>

// Item structure shared across the program
struct Item {
    int id;
    std::string name;
    int quantity;
    double price;
    std::string category;
    std::string expiryDate;
};

// New structure for per‑item sales summary
struct SalesItemSummary {
    int itemID;
    std::string name;
    int quantitySold;
    double totalSales;
};

// NEW struct for inventory valuation details (includes quantity)
struct InventoryItemValuation {
    std::string name;
    int quantity;
    double value;   // price * quantity
};

// Database initialization / teardown
bool connectDatabase(); // connects to MySQL and auto-creates database/tables
void closeDatabase();

// User-related functions
bool insertUser(const std::string& username, const std::string& password);
bool validateUserCredentials(const std::string& username, const std::string& password);

// Item-related functions
bool insertItem(Item& item);
bool loadItemsFromDatabase(std::vector<Item>& inventory, int& nextId);
bool updateItemQuantityInDatabase(int itemID, int quantity);
bool deleteItemFromDatabase(int itemID);

// NEW: Search items by name (partial match) directly from DB
bool searchItemsByName(const std::string& name, std::vector<Item>& results);

// Sales-related function
bool insertSale(int userID, int itemID, int quantity, const std::string& saleDate);

// Report-related functions
bool getSalesItemsSummary(const std::string& fromDate, const std::string& toDate,
                         std::vector<SalesItemSummary>& items);
bool getInventoryValuationDetails(double& totalValue,
                                 std::vector<InventoryItemValuation>& itemDetails);
bool getExpiredItems(std::vector<Item>& expiredItems);


#endif // DB_H