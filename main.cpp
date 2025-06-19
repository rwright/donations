/*
 * Donation Tracker - A Qt-based application for managing donations
 * Copyright (C) 2025 Russ Wright russ.wright@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


// main.cpp
// This file contains the main application entry point and the implementation of the
// DonationTracker, DonorDialog, DonationDialog, OrganizationDialog, and MainWindow classes.

// Required Qt and standard library includes for GUI, file operations, and data structures.
#include "donation_tracker.h" // Custom header for DonationTracker classes.
#include <QApplication> // Core application class.
#include <QVBoxLayout>  // Vertical layout manager.
#include <QHBoxLayout>  // Horizontal layout manager.
#include <QGridLayout>  // Grid layout manager.
#include <QFormLayout>  // Form layout for labels and input fields.
#include <QPushButton>  // Push button widget.
#include <QLabel>       // Widget for displaying text or images.
#include <QMessageBox>  // For displaying message boxes (errors, warnings, info).
#include <QRegularExpression> // For regular expression validation (e.g., email, phone).
#include <QDir>         // For directory operations (e.g., creating "letters" folder).
#include <QFile>        // For file I/O operations.
#include <QTextStream>  // For reading and writing text.
#include <algorithm>    // Standard algorithms (e.g., for std::find).
#include <QGroupBox>    // For grouping related widgets.
#include <QHeaderView>  // For customizing table headers.
#include <QInputDialog> // For simple input dialogs.
#include <QDate>        // For date manipulation.
#include <QDateEdit>    // Widget for editing dates.
#include <QIcon>        // Added for QIcon - to use icons on buttons.
#include <QStyle>       // Added for QStyle - to get standard pixmaps.
#include <QStyleFactory> // Added for QStyleFactory - to set application style.
#include <QDebug>       // Added for qDebug() - for debugging output.

// -----------------------------------------------------------------------------
// DonationTracker Implementation
// This section implements the core database logic defined in donation_tracker.h.
// -----------------------------------------------------------------------------

/**
 * @brief Constructor for DonationTracker.
 * Initializes the SQLite database connection. If the database file doesn't exist,
 * it will be created. Critical errors during opening will result in a message box.
 */
DonationTracker::DonationTracker() : db(nullptr) {
    // Attempt to open the SQLite database file "donations.db".
    if (sqlite3_open("donations.db", &db) != SQLITE_OK) {
        // If opening fails, display a critical error message.
        QMessageBox::critical(nullptr, "Error", "Cannot open database: " + QString(sqlite3_errmsg(db)));
    } else {
        // If successful, create necessary tables.
        createTables();
    }
}

/**
 * @brief Destructor for DonationTracker.
 * Closes the SQLite database connection if it's open.
 */
DonationTracker::~DonationTracker() {
    if (db) {
        sqlite3_close(db); // Close the database connection.
    }
}

/**
 * @brief Creates the necessary tables in the SQLite database if they don't already exist.
 * This includes 'donors', 'donations', and 'organization' tables.
 * Errors during table creation are reported via a message box.
 */
void DonationTracker::createTables() {
    // SQL statement to create three tables: donors, donations, and organization.
    // 'donors' table stores donor personal information.
    // 'donations' table stores donation records, with a foreign key to 'donors' and CASCADE delete.
    // 'organization' table stores details of the organization (single row).
    const char* sql = "CREATE TABLE IF NOT EXISTS donors ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "first_name TEXT, last_name TEXT, street TEXT, city TEXT, "
                      "state TEXT, zip TEXT, country TEXT, "
                      "phone TEXT, email TEXT);"
                      "CREATE TABLE IF NOT EXISTS donations ("
                      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                      "donor_id INTEGER, amount REAL, date TEXT, "
                      "payment_method TEXT, "
                      "FOREIGN KEY(donor_id) REFERENCES donors(id) ON DELETE CASCADE);" // Added ON DELETE CASCADE for referential integrity.
                      "CREATE TABLE IF NOT EXISTS organization ("
                      "id INTEGER PRIMARY KEY, "
                      "name TEXT, address TEXT);";
    char* errMsg = nullptr; // Pointer to store SQLite error messages.
    // Execute the SQL statement.
    if (sqlite3_exec(db, sql, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        // If execution fails, display a critical error message.
        QMessageBox::critical(nullptr, "Database Error", QString("SQL error in createTables: %1").arg(errMsg));
        sqlite3_free(errMsg); // Free the error message memory.
    }
}

/**
 * @brief Adds a new donor record to the 'donors' table.
 * Uses a prepared statement to prevent SQL injection and efficiently bind parameters.
 * @return True on successful insertion, false otherwise.
 */
bool DonationTracker::addDonor(const std::string& firstName, const std::string& lastName, const std::string& street, const std::string& city,
                  const std::string& state, const std::string& zip, const std::string& country,
                  const std::string& phone, const std::string& email) {
    const char* sql = "INSERT INTO donors (first_name, last_name, street, city, state, zip, country, phone, email) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt; // Prepared statement object.
    // Prepare the SQL statement.
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        // Bind parameters to the prepared statement.
        sqlite3_bind_text(stmt, 1, firstName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, lastName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, street.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, city.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, state.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, zip.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, country.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, phone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, email.c_str(), -1, SQLITE_TRANSIENT);
        // Execute the prepared statement.
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt); // Finalize the statement to release resources.
            return true;
        } else {
            // Report failure to add donor.
            QMessageBox::warning(nullptr, "Database Error", QString("Failed to add donor: %1").arg(sqlite3_errmsg(db)));
        }
    }
    sqlite3_finalize(stmt); // Ensure statement is finalized even if prepare fails.
    return false;
}

/**
 * @brief Updates an existing donor record in the 'donors' table.
 * @param id The ID of the donor to update.
 * @return True on successful update, false otherwise.
 */
bool DonationTracker::updateDonor(int id, const std::string& firstName, const std::string& lastName, const std::string& street, const std::string& city,
                     const std::string& state, const std::string& zip, const std::string& country,
                     const std::string& phone, const std::string& email) {
    const char* sql = "UPDATE donors SET first_name=?, last_name=?, street=?, city=?, state=?, zip=?, country=?, phone=?, email=? WHERE id=?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, firstName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, lastName.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, street.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, city.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, state.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 6, zip.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 7, country.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 8, phone.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 9, email.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 10, id); // Bind the ID for the WHERE clause.
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return true;
        } else {
            QMessageBox::warning(nullptr, "Database Error", QString("Failed to update donor: %1").arg(sqlite3_errmsg(db)));
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

/**
 * @brief Deletes a donor record from the 'donors' table based on ID.
 * Due to ON DELETE CASCADE, related donations in the 'donations' table will also be deleted.
 * @param id The ID of the donor to delete.
 * @return True on successful deletion, false otherwise.
 */
bool DonationTracker::deleteDonor(int id) {
    const char* sql = "DELETE FROM donors WHERE id=?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return true;
        } else {
            QMessageBox::warning(nullptr, "Database Error", QString("Failed to delete donor: %1").arg(sqlite3_errmsg(db)));
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

/**
 * @brief Adds a new donation record to the 'donations' table.
 * @return True on successful insertion, false otherwise.
 */
bool DonationTracker::addDonation(int donorId, double amount, const std::string& date, const std::string& paymentMethod) {
    const char* sql = "INSERT INTO donations (donor_id, amount, date, payment_method) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, donorId);
        sqlite3_bind_double(stmt, 2, amount);
        sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, paymentMethod.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return true;
        } else {
            QMessageBox::warning(nullptr, "Database Error", QString("Failed to add donation: %1").arg(sqlite3_errmsg(db)));
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

/**
 * @brief Updates an existing donation record in the 'donations' table.
 * @param id The ID of the donation to update.
 * @return True on successful update, false otherwise.
 */
bool DonationTracker::updateDonation(int id, int donorId, double amount, const std::string& date, const std::string& paymentMethod) {
    const char* sql = "UPDATE donations SET donor_id=?, amount=?, date=?, payment_method=? WHERE id=?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, donorId);
        sqlite3_bind_double(stmt, 2, amount);
        sqlite3_bind_text(stmt, 3, date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, paymentMethod.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 5, id);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return true;
        } else {
            QMessageBox::warning(nullptr, "Database Error", QString("Failed to update donation: %1").arg(sqlite3_errmsg(db)));
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

/**
 * @brief Deletes a donation record from the 'donations' table.
 * @param id The ID of the donation to delete.
 * @return True on successful deletion, false otherwise.
 */
bool DonationTracker::deleteDonation(int id) {
    const char* sql = "DELETE FROM donations WHERE id=?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return true;
        } else {
            QMessageBox::warning(nullptr, "Database Error", QString("Failed to delete donation: %1").arg(sqlite3_errmsg(db)));
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

/**
 * @brief Retrieves the organization's name and address from the 'organization' table.
 * Assumes there is only one organization record with id=1.
 * @param name Reference to a string to store the retrieved name.
 * @param address Reference to a string to store the retrieved address.
 * @return True if details are found, false otherwise.
 */
bool DonationTracker::getOrganizationDetails(std::string& name, std::string& address) {
    const char* sql = "SELECT name, address FROM organization WHERE id=1;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // Retrieve data from the current row.
            name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            sqlite3_finalize(stmt);
            return true;
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

/**
 * @brief Sets or updates the organization's details in the 'organization' table.
 * Uses INSERT OR REPLACE to either insert a new record or update an existing one (for id=1).
 * @param name The organization's name.
 * @param address The organization's address.
 * @return True on successful operation, false otherwise.
 */
bool DonationTracker::setOrganizationDetails(const std::string& name, const std::string& address) {
    const char* sql = "INSERT OR REPLACE INTO organization (id, name, address) VALUES (1, ?, ?);";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, address.c_str(), -1, SQLITE_TRANSIENT);
        if (sqlite3_step(stmt) == SQLITE_DONE) {
            sqlite3_finalize(stmt);
            return true;
        } else {
            QMessageBox::warning(nullptr, "Database Error", QString("Failed to set organization details: %1").arg(sqlite3_errmsg(db)));
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

/**
 * @brief Generates donation letters for all donors who made donations in the specified year.
 * Creates a "letters" directory and saves each letter as a text file.
 * @param year The year for which to aggregate donations and generate letters.
 * @return True if all letters were successfully generated, false if any error occurred.
 */
bool DonationTracker::generateDonationLetters(int year) {
    QDir().mkdir("letters"); // Ensure the "letters" directory exists.

    // SQL query to get donor details and sum of their donations for a specific year.
    const char* sql = "SELECT d.first_name, d.last_name, d.street, d.city, d.state, d.zip, d.country, SUM(don.amount) "
                      "FROM donors d JOIN donations don ON d.id = don.donor_id "
                      "WHERE SUBSTR(don.date, 1, 4) = ? " // Filter by year from the date string.
                      "GROUP BY d.id;"; // Group by donor to sum up donations.
    sqlite3_stmt* stmt;
    bool success = true; // Flag to track overall success.

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, std::to_string(year).c_str(), -1, SQLITE_TRANSIENT); // Bind the year.
        std::string orgName, orgAddress;
        getOrganizationDetails(orgName, orgAddress); // Get organization details for the letterhead.

        // Iterate through each donor's summed donations.
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            // Retrieve donor and total donation amount.
            std::string firstName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string lastName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            std::string street = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            std::string city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            std::string state = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            std::string zip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            std::string country = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            double totalAmount = sqlite3_column_double(stmt, 7);

            // Construct the filename for the donation letter.
            QString fileName = QString("letters/%1_%2_%3_donation_letter.txt")
                                   .arg(QString::fromStdString(firstName))
                                   .arg(QString::fromStdString(lastName))
                                   .arg(year);
            QFile file(fileName);
            // Attempt to open the file for writing.
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file); // Use QTextStream for easy text writing.
                // Write the letter content.
                out << QString::fromStdString(orgName) << "\n";
                out << QString::fromStdString(orgAddress) << "\n\n";
                out << QDate::currentDate().toString("MMMM d, yyyy") << "\n\n"; // Current date.
                out << QString::fromStdString(firstName) << " " << QString::fromStdString(lastName) << "\n";
                out << QString::fromStdString(street) << "\n";
                out << QString::fromStdString(city) << ", " << QString::fromStdString(state) << " " << QString::fromStdString(zip) << "\n";
                out << QString::fromStdString(country) << "\n\n";
                out << "Dear " << QString::fromStdString(firstName) << ",\n\n";
                out << "Thank you for your generous total donation of $" << QString::number(totalAmount, 'f', 2) // Format amount to 2 decimal places.
                    << " to " << QString::fromStdString(orgName) << " in " << year << ".\n";
                out << "Your support makes a significant difference to our mission.\n\n";
                out << "Sincerely,\n";
                out << QString::fromStdString(orgName) << "\n";
                file.close(); // Close the file.
            } else {
                // Report error if file could not be opened.
                QMessageBox::warning(nullptr, "File Error", "Could not open file for writing: " + fileName);
                success = false; // Mark overall process as failed.
            }
        }
    } else {
        // Report error if SQL statement preparation failed.
        QMessageBox::warning(nullptr, "Database Error", QString("Failed to prepare statement for letter generation: %1").arg(sqlite3_errmsg(db)));
        success = false;
    }
    sqlite3_finalize(stmt); // Finalize the statement.
    return success;
}

/**
 * @brief Retrieves the full details of a specific donor by their ID.
 * @param id The ID of the donor to retrieve.
 * @param firstName Output parameter for donor's first name.
 * @param lastName Output parameter for donor's last name.
 * @param street Output parameter for donor's street address.
 * @param city Output parameter for donor's city.
 * @param state Output parameter for donor's state.
 * @param zip Output parameter for donor's ZIP code.
 * @param country Output parameter for donor's country.
 * @param phone Output parameter for donor's phone number.
 * @param email Output parameter for donor's email.
 * @return True if donor details were found, false otherwise.
 */
bool DonationTracker::getDonorDetails(int id, std::string& firstName, std::string& lastName, std::string& street, std::string& city,
                                     std::string& state, std::string& zip, std::string& country,
                                     std::string& phone, std::string& email) {
    const char* sql = "SELECT first_name, last_name, street, city, state, zip, country, phone, email FROM donors WHERE id=?;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id); // Bind the donor ID.
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            // Retrieve all donor details from the current row.
            firstName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            lastName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            street = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            state = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            zip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            country = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));
            phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7));
            email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
            sqlite3_finalize(stmt);
            return true;
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

/**
 * @brief Retrieves all donor IDs from the database, ordered by ID.
 * This is used for sequential navigation through donor records.
 * @return A vector of integer donor IDs.
 */
std::vector<int> DonationTracker::getAllDonorIds() {
    std::vector<int> ids; // Vector to store donor IDs.
    const char* sql = "SELECT id FROM donors ORDER BY id;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        // Iterate through all rows and add each ID to the vector.
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ids.push_back(sqlite3_column_int(stmt, 0));
        }
    }
    sqlite3_finalize(stmt);
    return ids;
}

/**
 * @brief Searches for donors based on a given search term across multiple fields
 * (first name, last name, email, phone, city, state, zip, country).
 * Populates the provided QTableWidget with the search results.
 * If includeAll is true or searchTerm is empty, all donors are returned.
 * @param searchTerm The string to search for (case-insensitive, partial match).
 * @param table Pointer to the QTableWidget to display results.
 * @param includeAll If true, bypasses the search term and fetches all donors.
 */
void DonationTracker::searchDonors(const std::string& searchTerm, QTableWidget* table, bool includeAll) {
    sqlite3_stmt* stmt;
    std::string sql;

    if (includeAll || searchTerm.empty()) {
        // SQL to retrieve all donors, ordered by first and last name.
        sql = "SELECT id, first_name, last_name, street, city, state, zip, country, phone, email FROM donors ORDER BY first_name, last_name;";
    } else {
        // SQL to search for the searchTerm in multiple fields using LIKE operator.
        sql = "SELECT id, first_name, last_name, street, city, state, zip, country, phone, email FROM donors WHERE "
              "LOWER(first_name) LIKE ? OR "
              "LOWER(last_name) LIKE ? OR "
              "LOWER(email) LIKE ? OR "
              "LOWER(phone) LIKE ? OR "
              "LOWER(city) LIKE ? OR "
              "LOWER(state) LIKE ? OR "
              "LOWER(zip) LIKE ? OR "
              "LOWER(country) LIKE ?;";
    }

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        qDebug() << "Failed to prepare statement for search: " << sqlite3_errmsg(db);
        return;
    }

    if (!(includeAll || searchTerm.empty())) {
        // If searching, prepare the search term with wildcards and bind to all relevant parameters.
        std::string lowerSearchTerm = "%" + QString::fromStdString(searchTerm).toLower().toStdString() + "%";
        for (int i = 1; i <= 8; ++i) { // Bind to first_name, last_name, email, phone, city, state, zip, country.
            sqlite3_bind_text(stmt, i, lowerSearchTerm.c_str(), -1, SQLITE_TRANSIENT);
        }
    }

    table->setRowCount(0); // Clear existing rows in the table.
    table->setColumnCount(10); // Set the number of columns if not already set.
    // Set header labels for the table.
    table->setHorizontalHeaderLabels({"ID", "First Name", "Last Name", "Street", "City", "State", "ZIP", "Country", "Phone", "Email"});
    table->horizontalHeader()->setStretchLastSection(true); // Make the last section stretch to fill available space.
    table->setSelectionBehavior(QAbstractItemView::SelectRows); // Select entire rows.
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make table read-only.

    int row = 0;
    // Populate the table with fetched data.
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(QString::number(sqlite3_column_int(stmt, 0)))); // ID
        table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1))))); // First Name
        table->setItem(row, 2, new QTableWidgetItem(QString::fromStdString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2))))); // Last Name
        table->setItem(row, 3, new QTableWidgetItem(QString::fromStdString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3))))); // Street
        table->setItem(row, 4, new QTableWidgetItem(QString::fromStdString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4))))); // City
        table->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5))))); // State
        table->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6))))); // ZIP
        table->setItem(row, 7, new QTableWidgetItem(QString::fromStdString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7))))); // Country
        table->setItem(row, 8, new QTableWidgetItem(QString::fromStdString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8))))); // Phone
        table->setItem(row, 9, new QTableWidgetItem(QString::fromStdString(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9))))); // Email
        row++;
    }
    sqlite3_finalize(stmt);
}

/**
 * @brief Retrieves all donations for a given donor ID and populates a QTableWidget.
 * @param donorId The ID of the donor whose donations are to be retrieved.
 * @param table Pointer to the QTableWidget to display donation records.
 */
void DonationTracker::getDonationsForDonor(int donorId, QTableWidget* table) {
    const char* sql = "SELECT id, donor_id, amount, date, payment_method FROM donations WHERE donor_id=? ORDER BY date DESC;";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, donorId); // Bind the donor ID.

        table->clearContents(); // Clear existing content in the table.
        table->setRowCount(0); // Reset row count.
        table->setColumnCount(5); // Set the number of columns.
        // Set header labels for the donations table.
        table->setHorizontalHeaderLabels({"ID", "Donor ID", "Amount", "Date", "Payment Method"});
        table->horizontalHeader()->setStretchLastSection(true);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);

        int row = 0;
        // Populate the table with donation data.
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            table->insertRow(row);
            table->setItem(row, 0, new QTableWidgetItem(QString::number(sqlite3_column_int(stmt, 0)))); // ID
            table->setItem(row, 1, new QTableWidgetItem(QString::number(sqlite3_column_int(stmt, 1)))); // Donor ID
            table->setItem(row, 2, new QTableWidgetItem(QString::number(sqlite3_column_double(stmt, 2), 'f', 2))); // Amount (formatted)
            table->setItem(row, 3, new QTableWidgetItem(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3)))); // Date
            table->setItem(row, 4, new QTableWidgetItem(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4)))); // Payment Method
            row++;
        }
    }
    sqlite3_finalize(stmt);
}

// -----------------------------------------------------------------------------
// DonorDialog Implementation
// This section implements the UI and logic for the Donor details dialog.
// -----------------------------------------------------------------------------

/**
 * @brief Constructor for DonorDialog.
 * Sets up the UI elements (labels, line edits, buttons) and their layout.
 * Initializes validators for input fields.
 * @param parent The parent widget for this dialog.
 */
DonorDialog::DonorDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Donor Details");
    setMinimumWidth(300); // Set minimum width for the dialog.

    QVBoxLayout* mainLayout = new QVBoxLayout(this); // Main vertical layout for the dialog.
    QFormLayout* formLayout = new QFormLayout(); // Form layout for label-input pairs.

    // Initialize QLineEdit widgets for donor details.
    idEdit = new QLineEdit(this);
    idEdit->setReadOnly(true); // Donor ID is read-only.
    firstNameEdit = new QLineEdit(this);
    lastNameEdit = new QLineEdit(this);
    streetEdit = new QLineEdit(this);
    cityEdit = new QLineEdit(this);
    stateEdit = new QLineEdit(this);
    zipEdit = new QLineEdit(this);
    countryEdit = new QLineEdit(this);
    phoneEdit = new QLineEdit(this);
    emailEdit = new QLineEdit(this);

    // Add labels and input fields to the form layout.
    formLayout->addRow("ID:", idEdit);
    formLayout->addRow("First Name:", firstNameEdit);
    formLayout->addRow("Last Name:", lastNameEdit);
    formLayout->addRow("Street:", streetEdit);
    formLayout->addRow("City:", cityEdit);
    formLayout->addRow("State:", stateEdit);
    formLayout->addRow("ZIP Code:", zipEdit);
    formLayout->addRow("Country:", countryEdit);
    formLayout->addRow("Phone:", phoneEdit);
    formLayout->addRow("Email:", emailEdit);

    mainLayout->addLayout(formLayout); // Add the form layout to the main layout.

    QHBoxLayout* buttonLayout = new QHBoxLayout(); // Layout for OK and Cancel buttons.
    QPushButton* saveButton = new QPushButton("Save", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch(); // Push buttons to the right.
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout); // Add button layout to the main layout.

    // Connect signals to slots.
    connect(saveButton, &QPushButton::clicked, this, &QDialog::accept); // Accept dialog on Save.
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject); // Reject dialog on Cancel.
}

/**
 * @brief Validates the input fields in the DonorDialog.
 * Checks for non-empty required fields and basic email format.
 * @return True if all inputs are valid, false otherwise.
 */
bool DonorDialog::validateInputs() {
    // Check if essential fields are empty.
    if (firstNameEdit->text().isEmpty() || lastNameEdit->text().isEmpty() ||
        streetEdit->text().isEmpty() || cityEdit->text().isEmpty() ||
        stateEdit->text().isEmpty() || zipEdit->text().isEmpty() ||
        countryEdit->text().isEmpty() || phoneEdit->text().isEmpty() ||
        emailEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return false;
    }

    // Basic email format validation using QRegularExpression.
    QRegularExpression emailRegex("\\b[A-Z0-9._%+-]+@[A-Z0-9.-]+\\.[A-Z]{2,4}\\b", QRegularExpression::CaseInsensitiveOption);
    if (!emailRegex.match(emailEdit->text()).hasMatch()) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid email address.");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// DonationDialog Implementation
// This section implements the UI and logic for the Donation details dialog.
// -----------------------------------------------------------------------------

/**
 * @brief Constructor for DonationDialog.
 * Sets up the UI for adding/editing donation records.
 * Includes validators for numerical input (amount).
 * @param parent The parent widget for this dialog.
 */
DonationDialog::DonationDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Donation Details");
    setMinimumWidth(300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    idEdit = new QLineEdit(this);
    idEdit->setReadOnly(true);
    donorIdEdit = new QLineEdit(this);
    amountEdit = new QLineEdit(this);
    dateEdit = new QDateEdit(QDate::currentDate(), this); // Initialize with current date.
    dateEdit->setCalendarPopup(true); // Enable calendar pop-up.
    paymentMethodEdit = new QLineEdit(this);

    // Set a validator for the amountEdit to accept only doubles.
    QDoubleValidator* amountValidator = new QDoubleValidator(0.00, 10000000.00, 2, this); // Range from 0 to 10M, 2 decimal places.
    amountValidator->setNotation(QDoubleValidator::StandardNotation);
    amountEdit->setValidator(amountValidator);

    formLayout->addRow("ID:", idEdit);
    formLayout->addRow("Donor ID:", donorIdEdit);
    formLayout->addRow("Amount:", amountEdit);
    formLayout->addRow("Date:", dateEdit);
    formLayout->addRow("Payment Method:", paymentMethodEdit);

    mainLayout->addLayout(formLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* saveButton = new QPushButton("Save", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    connect(saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

/**
 * @brief Validates the input fields in the DonationDialog.
 * Checks for non-empty fields.
 * @return True if all inputs are valid, false otherwise.
 */
bool DonationDialog::validateInputs() {
    if (donorIdEdit->text().isEmpty() || amountEdit->text().isEmpty() ||
        dateEdit->text().isEmpty() || paymentMethodEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return false;
    }

    // Check if amount is a valid number after validation.
    if (amountEdit->hasAcceptableInput() == false) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid amount (e.g., 123.45).");
        return false;
    }

    return true;
}

// -----------------------------------------------------------------------------
// OrganizationDialog Implementation
// This section implements the UI and logic for the Organization details dialog.
// -----------------------------------------------------------------------------

/**
 * @brief Constructor for OrganizationDialog.
 * Sets up the UI for entering organization name and address details.
 * @param parent The parent widget for this dialog.
 */
OrganizationDialog::OrganizationDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Organization Details");
    setMinimumWidth(300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    nameEdit = new QLineEdit(this);
    streetEdit = new QLineEdit(this);
    cityEdit = new QLineEdit(this);
    stateEdit = new QLineEdit(this);
    zipEdit = new QLineEdit(this);
    countryEdit = new QLineEdit(this);

    formLayout->addRow("Name:", nameEdit);
    formLayout->addRow("Street:", streetEdit);
    formLayout->addRow("City:", cityEdit);
    formLayout->addRow("State:", stateEdit);
    formLayout->addRow("ZIP Code:", zipEdit);
    formLayout->addRow("Country:", countryEdit);

    mainLayout->addLayout(formLayout);

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* saveButton = new QPushButton("Save", this);
    QPushButton* cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addStretch();
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addStretch();

    mainLayout->addLayout(buttonLayout);

    connect(saveButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

/**
 * @brief Validates the input fields in the OrganizationDialog.
 * Checks if all fields are non-empty.
 * @return True if all inputs are valid, false otherwise.
 */
bool OrganizationDialog::validateInputs() {
    if (nameEdit->text().isEmpty() || streetEdit->text().isEmpty() ||
        cityEdit->text().isEmpty() || stateEdit->text().isEmpty() ||
        zipEdit->text().isEmpty() || countryEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please fill in all fields.");
        return false;
    }
    return true;
}

// -----------------------------------------------------------------------------
// MainWindow Implementation
// This section implements the main application window's UI and event handling.
// -----------------------------------------------------------------------------

/**
 * @brief Constructor for MainWindow.
 * Sets up the main window's layout, widgets, and connects signals/slots.
 * Initializes the DonationTracker backend and loads initial data.
 * @param parent The parent widget (nullptr for top-level window).
 */
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent), tracker(new DonationTracker()), currentDonorIndex(-1), currentDonorId(-1) {
    setWindowTitle("Donation Tracker");
    setMinimumSize(800, 600); // Set a reasonable minimum size.

    // Set the application style to Fusion for a modern look.
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    // Organization Details Display
    orgDetailsLabel = new QLabel("Organization: Not set", this);
    orgDetailsLabel->setStyleSheet("font-weight: bold; color: navy;");
    mainLayout->addWidget(orgDetailsLabel, 0, Qt::AlignTop | Qt::AlignLeft);
    updateOrganizationDisplay(); // Initial display of organization details.

    // Donor Details Section (QGroupBox for organization and styling)
    QGroupBox* donorDetailsGroup = new QGroupBox("Donor Details", this);
    QFormLayout* donorFormLayout = new QFormLayout();
    donorDetailsGroup->setLayout(donorFormLayout);

    // Initialize donor detail QLineEdit widgets.
    donorIdEdit = new QLineEdit(this);
    donorIdEdit->setReadOnly(true); // ID is read-only.
    donorFirstNameEdit = new QLineEdit(this);
    donorLastNameEdit = new QLineEdit(this);
    donorStreetEdit = new QLineEdit(this);
    donorCityEdit = new QLineEdit(this);
    donorStateEdit = new QLineEdit(this);
    donorZipEdit = new QLineEdit(this);
    donorCountryEdit = new QLineEdit(this);
    donorPhoneEdit = new QLineEdit(this);
    donorEmailEdit = new QLineEdit(this);

    // Add donor fields to the form layout.
    donorFormLayout->addRow("ID:", donorIdEdit);
    donorFormLayout->addRow("First Name:", donorFirstNameEdit);
    donorFormLayout->addRow("Last Name:", donorLastNameEdit);
    donorFormLayout->addRow("Street:", donorStreetEdit);
    donorFormLayout->addRow("City:", donorCityEdit);
    donorFormLayout->addRow("State:", donorStateEdit);
    donorFormLayout->addRow("ZIP Code:", donorZipEdit);
    donorFormLayout->addRow("Country:", donorCountryEdit);
    donorFormLayout->addRow("Phone:", donorPhoneEdit);
    donorFormLayout->addRow("Email:", donorEmailEdit);

    mainLayout->addWidget(donorDetailsGroup);

    // Donor Navigation Buttons
    QHBoxLayout* navButtonLayout = new QHBoxLayout();
    firstButton = new QPushButton("|< First", this);
    previousButton = new QPushButton("<< Previous", this);
    nextButton = new QPushButton("Next >>", this);
    lastButton = new QPushButton("Last >|", this);

    navButtonLayout->addStretch();
    navButtonLayout->addWidget(firstButton);
    navButtonLayout->addWidget(previousButton);
    navButtonLayout->addWidget(nextButton);
    navButtonLayout->addWidget(lastButton);
    navButtonLayout->addStretch();

    mainLayout->addLayout(navButtonLayout);

    // Donor Management Buttons
    QHBoxLayout* donorButtonLayout = new QHBoxLayout();
    QPushButton* addDonorButton = new QPushButton("Add Donor", this);
    QPushButton* editDonorButton = new QPushButton("Edit Donor", this);
    QPushButton* deleteDonorButton = new QPushButton("Delete Donor", this);

    donorButtonLayout->addStretch();
    donorButtonLayout->addWidget(addDonorButton);
    donorButtonLayout->addWidget(editDonorButton);
    donorButtonLayout->addWidget(deleteDonorButton);
    donorButtonLayout->addStretch();

    mainLayout->addLayout(donorButtonLayout);

    // Donations Table for current donor
    QGroupBox* donationsGroup = new QGroupBox("Donations for Current Donor", this);
    QVBoxLayout* donationsLayout = new QVBoxLayout();
    donationsGroup->setLayout(donationsLayout);

    donationsTable = new QTableWidget(this);
    donationsTable->setColumnCount(5);
    donationsTable->setHorizontalHeaderLabels({"ID", "Donor ID", "Amount", "Date", "Payment Method"});
    donationsTable->horizontalHeader()->setStretchLastSection(true);
    donationsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    donationsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    donationsLayout->addWidget(donationsTable);

    QHBoxLayout* donationButtonLayout = new QHBoxLayout();
    QPushButton* addDonationButton = new QPushButton("Add Donation", this);
    QPushButton* editDonationButton = new QPushButton("Edit Donation", this);
    QPushButton* deleteDonationButton = new QPushButton("Delete Donation", this);

    donationButtonLayout->addStretch();
    donationButtonLayout->addWidget(addDonationButton);
    donationButtonLayout->addWidget(editDonationButton);
    donationButtonLayout->addWidget(deleteDonationButton);
    donationButtonLayout->addStretch();
    donationsLayout->addLayout(donationButtonLayout);

    mainLayout->addWidget(donationsGroup);

    // Search and Other Actions Section
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchField = new QLineEdit(this);
    searchField->setPlaceholderText("Enter search term (e.g., name, email, city)");
    QPushButton* searchButton = new QPushButton("Search Donors", this);

    searchLayout->addWidget(searchField);
    searchLayout->addWidget(searchButton);
    mainLayout->addLayout(searchLayout);

    // Table for Search Results (Donors)
    table = new QTableWidget(this);
    table->setColumnCount(10); // ID, First, Last, Street, City, State, ZIP, Country, Phone, Email
    table->setHorizontalHeaderLabels({"ID", "First Name", "Last Name", "Street", "City", "State", "ZIP", "Country", "Phone", "Email"});
    table->horizontalHeader()->setStretchLastSection(true); // Last column fills remaining space.
    table->setSelectionBehavior(QAbstractItemView::SelectRows); // Entire row selected.
    table->setEditTriggers(QAbstractItemView::NoEditTriggers); // Make table read-only.
    mainLayout->addWidget(table);

    // Other Actions Buttons
    QHBoxLayout* miscButtonLayout = new QHBoxLayout();
    QPushButton* generateLettersButton = new QPushButton("Generate Donation Letters", this);
    QPushButton* setOrganizationButton = new QPushButton("Set Organization Details", this);

    miscButtonLayout->addStretch();
    miscButtonLayout->addWidget(generateLettersButton);
    miscButtonLayout->addWidget(setOrganizationButton);
    miscButtonLayout->addStretch();
    mainLayout->addLayout(miscButtonLayout);

    // Connect signals and slots for all buttons.
    connect(addDonorButton, &QPushButton::clicked, this, &MainWindow::addDonor);
    connect(editDonorButton, &QPushButton::clicked, this, &MainWindow::editDonor);
    connect(deleteDonorButton, &QPushButton::clicked, this, &MainWindow::deleteDonor);
    connect(addDonationButton, &QPushButton::clicked, this, &MainWindow::addDonation);
    connect(editDonationButton, &QPushButton::clicked, this, &MainWindow::editDonation);
    connect(deleteDonationButton, &QPushButton::clicked, this, &MainWindow::deleteDonation);
    connect(generateLettersButton, &QPushButton::clicked, this, &MainWindow::generateLetters);
    connect(setOrganizationButton, &QPushButton::clicked, this, &MainWindow::setOrganization);
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::search);

    // Connect navigation buttons.
    connect(firstButton, &QPushButton::clicked, this, &MainWindow::loadFirstDonor);
    connect(previousButton, &QPushButton::clicked, this, &MainWindow::loadPreviousDonor);
    connect(nextButton, &QPushButton::clicked, this, &MainWindow::loadNextDonor);
    connect(lastButton, &QPushButton::clicked, this, &MainWindow::loadLastDonor);

    // Connect table item clicks to slots to load details.
    connect(table, &QTableWidget::itemClicked, this, &MainWindow::onDonorTableItemClicked);
    connect(donationsTable, &QTableWidget::itemClicked, this, &MainWindow::onDonationTableItemClicked);

    populateDonorIds(); // Populate donor IDs for navigation.
    loadFirstDonor();   // Load the first donor on application start.
    updateNavigationButtonStates(); // Update button states based on initial donor loaded.

    // Perform an initial search to display all donors when the app starts
    // This effectively populates the main donor table with all entries
    tracker->searchDonors("", table, true);
}

/**
 * @brief Slot to handle adding a new donor.
 * Opens a DonorDialog, retrieves input, and calls DonationTracker to add the donor.
 * Reloads donor list and updates display upon success.
 */
void MainWindow::addDonor() {
    DonorDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted && dialog.validateInputs()) {
        if (tracker->addDonor(dialog.firstNameEdit->text().toStdString(),
                              dialog.lastNameEdit->text().toStdString(),
                              dialog.streetEdit->text().toStdString(),
                              dialog.cityEdit->text().toStdString(),
                              dialog.stateEdit->text().toStdString(),
                              dialog.zipEdit->text().toStdString(),
                              dialog.countryEdit->text().toStdString(),
                              dialog.phoneEdit->text().toStdString(),
                              dialog.emailEdit->text().toStdString())) {
            QMessageBox::information(this, "Success", "Donor added successfully.");
            populateDonorIds(); // Refresh donor IDs for navigation.
            tracker->searchDonors("", table, true); // Refresh main donor table.
            loadLastDonor(); // Load the newly added donor.
        } else {
            QMessageBox::warning(this, "Error", "Failed to add donor.");
        }
    }
}

/**
 * @brief Slot to handle editing an existing donor.
 * Opens a DonorDialog pre-filled with the current donor's data.
 * Updates the donor via DonationTracker upon successful dialog submission.
 */
void MainWindow::editDonor() {
    if (currentDonorId == -1) {
        QMessageBox::information(this, "Edit Donor", "Please select a donor to edit.");
        return;
    }

    DonorDialog dialog(this);
    // Retrieve current donor details to pre-fill the dialog.
    std::string firstName, lastName, street, city, state, zip, country, phone, email;
    if (tracker->getDonorDetails(currentDonorId, firstName, lastName, street, city, state, zip, country, phone, email)) {
        dialog.idEdit->setText(QString::number(currentDonorId));
        dialog.firstNameEdit->setText(QString::fromStdString(firstName));
        dialog.lastNameEdit->setText(QString::fromStdString(lastName));
        dialog.streetEdit->setText(QString::fromStdString(street));
        dialog.cityEdit->setText(QString::fromStdString(city));
        dialog.stateEdit->setText(QString::fromStdString(state));
        dialog.zipEdit->setText(QString::fromStdString(zip));
        dialog.countryEdit->setText(QString::fromStdString(country));
        dialog.phoneEdit->setText(QString::fromStdString(phone));
        dialog.emailEdit->setText(QString::fromStdString(email));

        if (dialog.exec() == QDialog::Accepted && dialog.validateInputs()) {
            if (tracker->updateDonor(currentDonorId,
                                      dialog.firstNameEdit->text().toStdString(),
                                      dialog.lastNameEdit->text().toStdString(),
                                      dialog.streetEdit->text().toStdString(),
                                      dialog.cityEdit->text().toStdString(),
                                      dialog.stateEdit->text().toStdString(),
                                      dialog.zipEdit->text().toStdString(),
                                      dialog.countryEdit->text().toStdString(),
                                      dialog.phoneEdit->text().toStdString(),
                                      dialog.emailEdit->text().toStdString())) {
                QMessageBox::information(this, "Success", "Donor updated successfully.");
                tracker->searchDonors("", table, true); // Refresh main donor table.
                loadDonor(currentDonorId); // Reload details for the updated donor.
            } else {
                QMessageBox::warning(this, "Error", "Failed to update donor.");
            }
        }
    } else {
        QMessageBox::warning(this, "Error", "Failed to retrieve donor details for editing.");
    }
}

/**
 * @brief Slot to handle deleting the currently loaded donor.
 * Confirms with the user before proceeding with deletion.
 * Updates the UI after deletion.
 */
void MainWindow::deleteDonor() {
    if (currentDonorId == -1) {
        QMessageBox::information(this, "Delete Donor", "No donor selected to delete.");
        return;
    }

    // Confirm deletion with the user.
    if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this donor and all associated donations?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (tracker->deleteDonor(currentDonorId)) {
            QMessageBox::information(this, "Success", "Donor and associated donations deleted successfully.");
            populateDonorIds(); // Re-populate donor IDs as one was removed.
            tracker->searchDonors("", table, true); // Refresh main donor table.
            loadFirstDonor(); // Load the first donor or clear fields if no donors remain.
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete donor.");
        }
    }
}

/**
 * @brief Slot to handle adding a new donation for the currently selected donor.
 * Opens a DonationDialog, pre-fills donor ID, and adds the donation via DonationTracker.
 * Refreshes the donations table upon success.
 */
void MainWindow::addDonation() {
    if (currentDonorId == -1) {
        QMessageBox::information(this, "Add Donation", "Please select a donor to add a donation for.");
        return;
    }

    DonationDialog dialog(this);
    dialog.donorIdEdit->setText(QString::number(currentDonorId)); // Pre-fill donor ID.
    if (dialog.exec() == QDialog::Accepted && dialog.validateInputs()) {
        // Convert date to "YYYY-MM-DD" format.
        std::string dateStr = dialog.dateEdit->date().toString("yyyy-MM-dd").toStdString();
        if (tracker->addDonation(dialog.donorIdEdit->text().toInt(),
                                  dialog.amountEdit->text().toDouble(),
                                  dateStr,
                                  dialog.paymentMethodEdit->text().toStdString())) {
            QMessageBox::information(this, "Success", "Donation added successfully.");
            tracker->getDonationsForDonor(currentDonorId, donationsTable); // Refresh donations table.
        } else {
            QMessageBox::warning(this, "Error", "Failed to add donation.");
        }
    }
}

/**
 * @brief Slot to handle editing an existing donation.
 * Opens a DonationDialog pre-filled with the selected donation's data.
 * Updates the donation via DonationTracker upon successful dialog submission.
 */
void MainWindow::editDonation() {
    // Ensure a donation is selected in the donations table.
    if (!donationsTable->selectedItems().isEmpty()) {
        int selectedRow = donationsTable->currentRow();
        int donationId = donationsTable->item(selectedRow, 0)->text().toInt(); // Get donation ID.
        int donorId = donationsTable->item(selectedRow, 1)->text().toInt(); // Get donor ID.
        double amount = donationsTable->item(selectedRow, 2)->text().toDouble(); // Get amount.
        QDate date = QDate::fromString(donationsTable->item(selectedRow, 3)->text(), "yyyy-MM-dd"); // Get date.
        std::string paymentMethod = donationsTable->item(selectedRow, 4)->text().toStdString(); // Get payment method.

        DonationDialog dialog(this);
        // Pre-fill the dialog with existing donation data.
        dialog.idEdit->setText(QString::number(donationId));
        dialog.donorIdEdit->setText(QString::number(donorId));
        dialog.amountEdit->setText(QString::number(amount, 'f', 2));
        dialog.dateEdit->setDate(date);
        dialog.paymentMethodEdit->setText(QString::fromStdString(paymentMethod));

        if (dialog.exec() == QDialog::Accepted && dialog.validateInputs()) {
            std::string newDateStr = dialog.dateEdit->date().toString("yyyy-MM-dd").toStdString();
            if (tracker->updateDonation(donationId,
                                        dialog.donorIdEdit->text().toInt(),
                                        dialog.amountEdit->text().toDouble(),
                                        newDateStr,
                                        dialog.paymentMethodEdit->text().toStdString())) {
                QMessageBox::information(this, "Success", "Donation updated successfully.");
                tracker->getDonationsForDonor(currentDonorId, donationsTable); // Refresh donations table.
            } else {
                QMessageBox::warning(this, "Error", "Failed to update donation.");
            }
        }
    } else {
        QMessageBox::information(this, "Edit Donation", "Please select a donation to edit.");
    }
}

/**
 * @brief Slot to handle deleting a selected donation.
 * Confirms with the user before deleting the donation via DonationTracker.
 * Refreshes the donations table upon success.
 */
void MainWindow::deleteDonation() {
    // Ensure a donation is selected.
    if (!donationsTable->selectedItems().isEmpty()) {
        int selectedRow = donationsTable->currentRow();
        int donationId = donationsTable->item(selectedRow, 0)->text().toInt();

        // Confirm deletion.
        if (QMessageBox::question(this, "Confirm Delete", "Are you sure you want to delete this donation?",
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            if (tracker->deleteDonation(donationId)) {
                QMessageBox::information(this, "Success", "Donation deleted successfully.");
                tracker->getDonationsForDonor(currentDonorId, donationsTable); // Refresh donations table.
            } else {
                QMessageBox::warning(this, "Error", "Failed to delete donation.");
            }
        }
    } else {
        QMessageBox::information(this, "Delete Donation", "Please select a donation to delete.");
    }
}

/**
 * @brief Slot to handle generating donation letters.
 * Prompts the user for a year and then calls DonationTracker to generate letters.
 */
void MainWindow::generateLetters() {
    bool ok;
    // Prompt user for the year. Default to current year.
    int year = QInputDialog::getInt(this, "Generate Donation Letters",
                                    "Enter year for donation letters:",
                                    QDate::currentDate().year(), 2000, QDate::currentDate().year() + 5, 1, &ok);
    if (ok) {
        if (tracker->generateDonationLetters(year)) {
            QMessageBox::information(this, "Success", "Donation letters generated successfully in the 'letters' folder.");
        } else {
            QMessageBox::warning(this, "Error", "Failed to generate donation letters. Check database or file permissions.");
        }
    }
}

/**
 * @brief Slot to handle setting organization details.
 * Opens an OrganizationDialog, retrieves input, and updates organization details via DonationTracker.
 * Updates the organization display label upon success.
 */
void MainWindow::setOrganization() {
    OrganizationDialog dialog(this);
    std::string name, address;
    // Pre-fill dialog if organization details already exist.
    if (tracker->getOrganizationDetails(name, address)) {
        // Simple parsing of address. This might need more robust handling for complex addresses.
        QString qAddress = QString::fromStdString(address);
        QStringList parts = qAddress.split(", ", Qt::SkipEmptyParts);

        if (parts.size() >= 4) {
            dialog.streetEdit->setText(parts[0]);
            dialog.cityEdit->setText(parts[1]);
            // Attempt to split "State ZIP"
            QStringList stateZip = parts[2].split(" ", Qt::SkipEmptyParts);
            if (stateZip.size() == 2) {
                dialog.stateEdit->setText(stateZip[0]);
                dialog.zipEdit->setText(stateZip[1]);
            } else {
                dialog.stateEdit->setText(parts[2]); // Fallback if format is not "State ZIP"
            }
            dialog.countryEdit->setText(parts[3]);
        } else {
            // Fallback: If parsing is difficult, just put the whole address into street.
            dialog.streetEdit->setText(qAddress);
        }
        dialog.nameEdit->setText(QString::fromStdString(name));
    }

    if (dialog.exec() == QDialog::Accepted && dialog.validateInputs()) {
        // Combine address fields into a single string for storage.
        std::string address = dialog.streetEdit->text().toStdString() + ", " +
                              dialog.cityEdit->text().toStdString() + ", " +
                              dialog.stateEdit->text().toStdString() + " " +
                              dialog.zipEdit->text().toStdString() + ", " +
                              dialog.countryEdit->text().toStdString();
        if (tracker->setOrganizationDetails(dialog.nameEdit->text().toStdString(), address)) {
            updateOrganizationDisplay(); // Refresh the display label.
            QMessageBox::information(this, "Success", "Organization details saved");
        } else {
            QMessageBox::warning(this, "Error", "Failed to save organization details");
        }
    }
}

/**
 * @brief Slot for performing a donor search.
 * Calls DonationTracker's searchDonors method and updates the main donor table.
 */
void MainWindow::search() {
    // Pass the search term from the QLineEdit to the tracker.
    tracker->searchDonors(searchField->text().toStdString(), table);
}

/**
 * @brief Updates the QLabel displaying the organization's name and address.
 * Retrieves details from DonationTracker and formats them for display.
 */
void MainWindow::updateOrganizationDisplay() {
    std::string name, address;
    if (tracker->getOrganizationDetails(name, address)) {
        orgDetailsLabel->setText(QString("Organization: %1\n%2").arg(QString::fromStdString(name), QString::fromStdString(address)));
    } else {
        orgDetailsLabel->setText("Organization: Not set");
    }
}

/**
 * @brief Populates the `donorIds` vector with all current donor IDs from the database.
 * Called to refresh the list of available donors for navigation.
 */
void MainWindow::populateDonorIds() {
    donorIds = tracker->getAllDonorIds(); // Get all donor IDs.
    // If a donor was selected and then deleted, currentDonorIndex might become invalid.
    // This part ensures `currentDonorIndex` is reset or adjusted.
    if (currentDonorId != -1) {
        auto it = std::find(donorIds.begin(), donorIds.end(), currentDonorId);
        if (it != donorIds.end()) {
            currentDonorIndex = std::distance(donorIds.begin(), it);
        } else {
            currentDonorIndex = -1; // Current donor not found (e.g., deleted).
        }
    }
    updateNavigationButtonStates(); // Update navigation button enabled states.
}

/**
 * @brief Loads the details of a specific donor into the UI fields and their donations into the table.
 * If id is -1, it clears the donor details fields.
 * @param id The ID of the donor to load.
 */
void MainWindow::loadDonor(int id) {
    if (id != -1) {
        std::string firstName, lastName, street, city, state, zip, country, phone, email;
        if (tracker->getDonorDetails(id, firstName, lastName, street, city, state, zip, country, phone, email)) {
            // Populate UI fields with donor details.
            donorIdEdit->setText(QString::number(id));
            donorFirstNameEdit->setText(QString::fromStdString(firstName));
            donorLastNameEdit->setText(QString::fromStdString(lastName));
            donorStreetEdit->setText(QString::fromStdString(street));
            donorCityEdit->setText(QString::fromStdString(city));
            donorStateEdit->setText(QString::fromStdString(state));
            donorZipEdit->setText(QString::fromStdString(zip));
            donorCountryEdit->setText(QString::fromStdString(country));
            donorPhoneEdit->setText(QString::fromStdString(phone));
            donorEmailEdit->setText(QString::fromStdString(email));
            currentDonorId = id; // Set the currently loaded donor ID.
            tracker->getDonationsForDonor(id, donationsTable); // Load donations for this donor.
        } else {
            // If donor details can't be fetched, clear fields and reset currentDonorId.
            clearDonorDetailsFields();
            QMessageBox::warning(this, "Error", "Failed to load donor details.");
            currentDonorId = -1;
        }
    } else {
        clearDonorDetailsFields(); // Clear all fields if no donor selected (id is -1).
        currentDonorId = -1;
        donationsTable->clearContents(); // Clear donations table.
        donationsTable->setRowCount(0);
    }
    updateNavigationButtonStates(); // Update button states after loading a donor.
}

/**
 * @brief Helper function to clear all donor details QLineEdit fields.
 */
void MainWindow::clearDonorDetailsFields() {
    donorIdEdit->clear();
    donorFirstNameEdit->clear();
    donorLastNameEdit->clear();
    donorStreetEdit->clear();
    donorCityEdit->clear();
    donorStateEdit->clear();
    donorZipEdit->clear();
    donorCountryEdit->clear();
    donorPhoneEdit->clear();
    donorEmailEdit->clear();
}

/**
 * @brief Slot for when a donor item in the main search table is clicked.
 * Loads the details of the clicked donor into the dedicated donor details section.
 * @param item The QTableWidgetItem that was clicked.
 */
void MainWindow::onDonorTableItemClicked(QTableWidgetItem* item) {
    if (item) {
        int donorId = table->item(item->row(), 0)->text().toInt(); // Get ID from the first column.
        qDebug() << "Donor table item clicked. Donor ID:" << donorId;
        // Find the index of the clicked donor in the donorIds vector.
        auto it = std::find(donorIds.begin(), donorIds.end(), donorId);
        if (it != donorIds.end()) {
            currentDonorIndex = std::distance(donorIds.begin(), it);
            loadDonor(donorId); // Load the details for the clicked donor.
        } else {
            qDebug() << "Clicked donor ID not found in populated donorIds vector.";
        }
    }
}

/**
 * @brief Slot for when a donation item in the donations table is clicked.
 * This can be used later to enable "Edit Donation" button, for example.
 * Currently, it just logs the clicked donation ID.
 * @param item The QTableWidgetItem that was clicked.
 */
void MainWindow::onDonationTableItemClicked(QTableWidgetItem* item) {
    if (item) {
        int donationId = donationsTable->item(item->row(), 0)->text().toInt();
        qDebug() << "Donation table item clicked. Donation ID:" << donationId;
        // Further actions like enabling an "Edit Donation" button could go here.
    }
}

/**
 * @brief Slot to load the first donor in the list.
 * Updates `currentDonorIndex` and calls `loadDonor`.
 */
void MainWindow::loadFirstDonor() {
    populateDonorIds(); // Ensure donorIds is up-to-date.
    if (!donorIds.empty()) {
        currentDonorIndex = 0;
        loadDonor(donorIds[currentDonorIndex]);
    } else {
        loadDonor(-1); // No donors.
    }
}

/**
 * @brief Slot to load the previous donor in the list.
 * Decrements `currentDonorIndex` and calls `loadDonor`.
 */
void MainWindow::loadPreviousDonor() {
    if (currentDonorIndex > 0) {
        currentDonorIndex--;
        loadDonor(donorIds[currentDonorIndex]);
    }
}

/**
 * @brief Slot to load the next donor in the list.
 * Increments `currentDonorIndex` and calls `loadDonor`.
 */
void MainWindow::loadNextDonor() {
    if (static_cast<size_t>(currentDonorIndex) < donorIds.size() - 1) {
        currentDonorIndex++;
        loadDonor(donorIds[currentDonorIndex]);
    }
}

/**
 * @brief Slot to load the last donor in the list.
 * Updates `currentDonorIndex` to the last element and calls `loadDonor`.
 */
void MainWindow::loadLastDonor() {
    populateDonorIds(); // Ensure donorIds is up-to-date.
    if (!donorIds.empty()) {
        currentDonorIndex = donorIds.size() - 1;
        loadDonor(donorIds[currentDonorIndex]);
    } else {
        loadDonor(-1); // No donors.
    }
}

/**
 * @brief Updates the enabled/disabled state of the navigation buttons.
 * Buttons are enabled only if there are valid previous/next donors to navigate to.
 */
void MainWindow::updateNavigationButtonStates() {
    if (donorIds.empty()) {
        // If no donors, disable all navigation buttons.
        firstButton->setEnabled(false);
        previousButton->setEnabled(false);
        nextButton->setEnabled(false);
        lastButton->setEnabled(false);
    } else {
        // Enable/disable based on current index relative to list bounds.
        firstButton->setEnabled(currentDonorIndex > 0);
        previousButton->setEnabled(currentDonorIndex > 0);
        nextButton->setEnabled(static_cast<size_t>(currentDonorIndex) < donorIds.size() - 1);
        lastButton->setEnabled(static_cast<size_t>(currentDonorIndex) < donorIds.size() - 1);
    }
}

// -----------------------------------------------------------------------------
// main function
// The entry point of the application.
// -----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    QApplication app(argc, argv); // Create the Qt application object.
    MainWindow window; // Create an instance of the main window.
    window.show(); // Display the main window.
    return app.exec(); // Start the Qt event loop.
}
