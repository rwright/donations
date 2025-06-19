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


// donation_tracker.h
#ifndef DONATION_TRACKER_H
#define DONATION_TRACKER_H

// Required Qt and standard library includes for UI elements, database interaction, and data structures.
#include <QObject>
#include <QDialog>
#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QLabel>
#include <QDateEdit> // Include for QDateEdit
#include <QDoubleValidator> // Include for QDoubleValidator (needed for DonationDialog)
#include <sqlite3.h> // SQLite database library for data persistence.
#include <string> // Standard C++ string manipulation.
#include <vector> // Standard C++ dynamic array (used for donor IDs).

/**
 * @brief The DonationTracker class manages all database interactions for donors, donations, and organization details.
 * It acts as the backend logic for the application, abstracting direct SQLite operations from the UI.
 */
class DonationTracker : public QObject {
    Q_OBJECT // Macro to enable Qt's meta-object system (signals and slots).

private:
    sqlite3* db; // Pointer to the SQLite database connection.
    void createTables(); // Private helper function to create necessary database tables if they don't exist.

public:
    // Constructor initializes the database connection.
    // No parent argument needed for DonationTracker, as it's a backend logic class.
    DonationTracker();
    // Destructor closes the database connection.
    ~DonationTracker();

    /**
     * @brief Adds a new donor record to the database.
     * @param firstName The first name of the donor.
     * @param lastName The last name of the donor.
     * @param street The street address of the donor.
     * @param city The city of the donor.
     * @param state The state/province of the donor.
     * @param zip The postal/zip code of the donor.
     * @param country The country of the donor.
     * @param phone The phone number of the donor.
     * @param email The email address of the donor.
     * @return True if the donor was added successfully, false otherwise.
     */
    bool addDonor(const std::string& firstName, const std::string& lastName, const std::string& street, const std::string& city,
                  const std::string& state, const std::string& zip, const std::string& country,
                  const std::string& phone, const std::string& email);

    /**
     * @brief Updates an existing donor record in the database.
     * @param id The ID of the donor to update.
     * @param firstName The new first name of the donor.
     * @param lastName The new last name of the donor.
     * @param street The new street address of the donor.
     * @param city The new city of the donor.
     * @param state The new state/province of the donor.
     * @param zip The new postal/zip code of the donor.
     * @param country The new country of the donor.
     * @param phone The new phone number of the donor.
     * @param email The new email address of the donor.
     * @return True if the donor was updated successfully, false otherwise.
     */
    bool updateDonor(int id, const std::string& firstName, const std::string& lastName, const std::string& street, const std::string& city,
                     const std::string& state, const std::string& zip, const std::string& country,
                     const std::string& phone, const std::string& email);

    /**
     * @brief Deletes a donor record from the database.
     * @param id The ID of the donor to delete.
     * @return True if the donor was deleted successfully, false otherwise.
     */
    bool deleteDonor(int id);

    /**
     * @brief Adds a new donation record to the database.
     * @param donorId The ID of the donor associated with this donation.
     * @param amount The amount of the donation.
     * @param date The date of the donation (e.g., "YYYY-MM-DD").
     * @param paymentMethod The method of payment for the donation.
     * @return True if the donation was added successfully, false otherwise.
     */
    bool addDonation(int donorId, double amount, const std::string& date, const std::string& paymentMethod);

    /**
     * @brief Updates an existing donation record in the database.
     * @param id The ID of the donation to update.
     * @param donorId The new donor ID associated with this donation.
     * @param amount The new amount of the donation.
     * @param date The new date of the donation.
     * @param paymentMethod The new method of payment for the donation.
     * @return True if the donation was updated successfully, false otherwise.
     */
    bool updateDonation(int id, int donorId, double amount, const std::string& date, const std::string& paymentMethod);

    /**
     * @brief Deletes a donation record from the database.
     * @param id The ID of the donation to delete.
     * @return True if the donation was deleted successfully, false otherwise.
     */
    bool deleteDonation(int id);

    /**
     * @brief Retrieves the organization's details from the database.
     * @param name Output parameter to store the organization's name.
     * @param address Output parameter to store the organization's address.
     * @return True if details were retrieved successfully, false if not set or an error occurred.
     */
    bool getOrganizationDetails(std::string& name, std::string& address);

    /**
     * @brief Sets or updates the organization's details in the database.
     * @param name The name of the organization.
     * @param address The address of the organization.
     * @return True if details were set successfully, false otherwise.
     */
    bool setOrganizationDetails(const std::string& name, const std::string& address);

    /**
     * @brief Generates donation letters for all donors for a specified year.
     * Letters are saved as text files in a "letters" directory.
     * @param year The year for which to generate donation letters.
     * @return True if letters were generated successfully for all donors, false otherwise.
     */
    bool generateDonationLetters(int year);

    /**
     * @brief Searches for donors based on a search term across multiple fields and populates a QTableWidget.
     * @param searchTerm The string to search for.
     * @param table The QTableWidget to populate with search results.
     * @param includeAll If true, all donors are returned regardless of searchTerm.
     */
    void searchDonors(const std::string& searchTerm, QTableWidget* table, bool includeAll = false);

    /**
     * @brief Retrieves all donor IDs from the database, ordered by ID.
     * @return A vector of donor IDs.
     */
    std::vector<int> getAllDonorIds();

    /**
     * @brief Retrieves details for a specific donor by ID.
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
    bool getDonorDetails(int id, std::string& firstName, std::string& lastName, std::string& street, std::string& city,
                                     std::string& state, std::string& zip, std::string& country,
                                     std::string& phone, std::string& email);

    /**
     * @brief Retrieves all donations for a given donor and populates a QTableWidget.
     * @param donorId The ID of the donor whose donations are to be retrieved.
     * @param table The QTableWidget to populate with donation records.
     */
    void getDonationsForDonor(int donorId, QTableWidget* table);
};

/**
 * @brief The DonorDialog class provides a dialog for adding or editing donor information.
 */
class DonorDialog : public QDialog {
    Q_OBJECT // Enables Qt's meta-object system.

public:
    /**
     * @brief Constructor for DonorDialog.
     * @param parent The parent widget.
     */
    DonorDialog(QWidget* parent = nullptr);

    /**
     * @brief Validates the input fields in the dialog.
     * @return True if all inputs are valid, false otherwise.
     */
    bool validateInputs();

    // Public QLineEdit pointers to allow access to input fields from MainWindow for data retrieval.
    QLineEdit* idEdit;
    QLineEdit* firstNameEdit;
    QLineEdit* lastNameEdit;
    QLineEdit* streetEdit;
    QLineEdit* cityEdit;
    QLineEdit* stateEdit;
    QLineEdit* zipEdit;
    QLineEdit* countryEdit;
    QLineEdit* phoneEdit;
    QLineEdit* emailEdit;

signals:
    // Signal emitted when a donor is successfully saved (added or updated).
    void donorSaved(int id);
};

/**
 * @brief The DonationDialog class provides a dialog for adding or editing donation information.
 */
class DonationDialog : public QDialog {
    Q_OBJECT // Enables Qt's meta-object system.

public:
    /**
     * @brief Constructor for DonationDialog.
     * @param parent The parent widget.
     */
    DonationDialog(QWidget* parent = nullptr);

    /**
     * @brief Validates the input fields in the dialog.
     * @return True if all inputs are valid, false otherwise.
     */
    bool validateInputs();

    // Public QLineEdit and QDateEdit pointers for input fields.
    QLineEdit* idEdit;
    QLineEdit* donorIdEdit;
    QLineEdit* amountEdit;
    QDateEdit* dateEdit; // Using QDateEdit for date input.
    QLineEdit* paymentMethodEdit;

signals:
    // Signal emitted when a donation is successfully saved (added or updated).
    void donationSaved(int id);
};

/**
 * @brief The OrganizationDialog class provides a dialog for setting or updating organization details.
 */
class OrganizationDialog : public QDialog {
    Q_OBJECT // Enables Qt's meta-object system.

public:
    /**
     * @brief Constructor for OrganizationDialog.
     * @param parent The parent widget.
     */
    OrganizationDialog(QWidget* parent = nullptr);

    /**
     * @brief Validates the input fields in the dialog.
     * @return True if all inputs are valid, false otherwise.
     */
    bool validateInputs();

    // Public QLineEdit pointers for organization details.
    QLineEdit* nameEdit;
    QLineEdit* streetEdit;
    QLineEdit* cityEdit;
    QLineEdit* stateEdit;
    QLineEdit* zipEdit;
    QLineEdit* countryEdit;
};

/**
 * @brief The MainWindow class represents the main application window.
 * It orchestrates the UI, handles user interactions, and communicates with the DonationTracker backend.
 */
class MainWindow : public QMainWindow {
    Q_OBJECT // Enables Qt's meta-object system for signals and slots.

public:
    /**
     * @brief Constructor for MainWindow.
     * @param parent The parent widget.
     */
    MainWindow(QWidget* parent = nullptr);

private slots:
    // Slots for handling button clicks and other UI events.
    void addDonor();
    void editDonor();
    void deleteDonor();
    void addDonation();
    void editDonation();
    void deleteDonation();
    void generateLetters();
    void setOrganization();
    void search(); // Slot for initiating a donor search.

    // Slots for navigating through donor records.
    void loadFirstDonor();
    void loadPreviousDonor();
    void loadNextDonor();
    void loadLastDonor();

    // Slots for handling item clicks in the donor and donation tables.
    void onDonorTableItemClicked(QTableWidgetItem* item);
    void onDonationTableItemClicked(QTableWidgetItem* item);

private:
    DonationTracker* tracker; // Instance of the backend database tracker.
    QLabel* orgDetailsLabel; // Label to display organization details.
    void updateOrganizationDisplay(); // Helper to refresh the organization details display.

    // Donor Details fields displayed in the main window.
    QLineEdit* donorIdEdit;
    QLineEdit* donorFirstNameEdit;
    QLineEdit* donorLastNameEdit;
    QLineEdit* donorStreetEdit;
    QLineEdit* donorCityEdit;
    QLineEdit* donorStateEdit;
    QLineEdit* donorZipEdit;
    QLineEdit* donorCountryEdit;
    QLineEdit* donorPhoneEdit;
    QLineEdit* donorEmailEdit;

    // Table for displaying search results (donors).
    QTableWidget* table;

    // Table for displaying donations of a selected donor.
    QTableWidget* donationsTable;

    // Search fields for filtering donor records.
    QLineEdit* searchField;
    QLineEdit* searchValue;

    // Navigation buttons for Browse donor records.
    QPushButton* firstButton;
    QPushButton* previousButton;
    QPushButton* nextButton;
    QPushButton* lastButton;

    // Donor navigation logic.
    std::vector<int> donorIds; // Stores IDs of all donors for navigation.
    int currentDonorIndex; // Current index in the donorIds vector.
    int currentDonorId; // Store the ID of the currently loaded donor.

    void populateDonorIds(); // Populates the donorIds vector.
    void loadDonor(int id); // Loads donor details and their donations into the UI.
    void updateNavigationButtonStates(); // Enables/disables navigation buttons based on current position.
};

#endif // DONATION_TRACKER_H
