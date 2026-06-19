#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include <algorithm>
#include <ctime>
#include <map>

using namespace std;

// ─────────────────────────────────────────────
//  Utility: current timestamp string
// ─────────────────────────────────────────────
string currentTimestamp() {
    time_t now = time(nullptr);
    char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}

// ─────────────────────────────────────────────
//  Class: Transaction
// ─────────────────────────────────────────────
class Transaction {
public:
    string type;      // DEPOSIT / WITHDRAW / TRANSFER
    double amount;
    double balanceAfter;
    string timestamp;

    Transaction() {}
    Transaction(const string& t, double a, double b)
        : type(t), amount(a), balanceAfter(b), timestamp(currentTimestamp()) {}

    string serialize() const {
        ostringstream oss;
        oss << type << "|" << fixed << setprecision(2)
            << amount << "|" << balanceAfter << "|" << timestamp;
        return oss.str();
    }

    static Transaction deserialize(const string& line) {
        Transaction t;
        istringstream iss(line);
        string tok;
        getline(iss, tok, '|'); t.type         = tok;
        getline(iss, tok, '|'); t.amount       = stod(tok);
        getline(iss, tok, '|'); t.balanceAfter = stod(tok);
        getline(iss, tok, '|'); t.timestamp    = tok;
        return t;
    }
};

// ─────────────────────────────────────────────
//  Class: Account
// ─────────────────────────────────────────────
class Account {
private:
    int    accountNumber;
    string holderName;
    string accountType;   // SAVINGS / CURRENT
    double balance;
    string pin;           // stored as plain string (demo only)
    string createdAt;
    vector<Transaction> transactions;

public:
    // Constructor for new account
    Account(int accNo, const string& name, const string& type,
            double initialDeposit, const string& pin)
        : accountNumber(accNo), holderName(name), accountType(type),
          balance(initialDeposit), pin(pin), createdAt(currentTimestamp()) {
        transactions.emplace_back("DEPOSIT", initialDeposit, balance);
    }

    // Default constructor (for deserialization)
    Account() : accountNumber(0), balance(0.0) {}

    // ── Getters ──────────────────────────────
    int           getAccountNumber() const { return accountNumber; }
    string        getHolderName()    const { return holderName;    }
    string        getAccountType()   const { return accountType;   }
    double        getBalance()       const { return balance;       }
    string        getCreatedAt()     const { return createdAt;     }
    const vector<Transaction>& getTransactions() const { return transactions; }

    bool verifyPin(const string& p) const { return pin == p; }

    // ── Operations ───────────────────────────
    bool deposit(double amount) {
        if (amount <= 0) return false;
        balance += amount;
        transactions.emplace_back("DEPOSIT", amount, balance);
        return true;
    }

    bool withdraw(double amount) {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        transactions.emplace_back("WITHDRAW", amount, balance);
        return true;
    }

    bool transferOut(double amount) {
        if (amount <= 0 || amount > balance) return false;
        balance -= amount;
        transactions.emplace_back("TRANSFER OUT", amount, balance);
        return true;
    }

    void transferIn(double amount) {
        balance += amount;
        transactions.emplace_back("TRANSFER IN", amount, balance);
    }

    // ── Serialization ────────────────────────
    string serialize() const {
        ostringstream oss;
        oss << accountNumber << "~"
            << holderName    << "~"
            << accountType   << "~"
            << fixed << setprecision(2) << balance << "~"
            << pin           << "~"
            << createdAt     << "~"
            << transactions.size();
        for (const auto& tx : transactions)
            oss << "~" << tx.serialize();
        return oss.str();
    }

    static Account deserialize(const string& line) {
        Account a;
        istringstream iss(line);
        string tok;
        getline(iss, tok, '~'); a.accountNumber = stoi(tok);
        getline(iss, tok, '~'); a.holderName    = tok;
        getline(iss, tok, '~'); a.accountType   = tok;
        getline(iss, tok, '~'); a.balance       = stod(tok);
        getline(iss, tok, '~'); a.pin           = tok;
        getline(iss, tok, '~'); a.createdAt     = tok;
        getline(iss, tok, '~'); int txCount     = stoi(tok);
        for (int i = 0; i < txCount; ++i) {
            getline(iss, tok, '~');
            a.transactions.push_back(Transaction::deserialize(tok));
        }
        return a;
    }
};

// ─────────────────────────────────────────────
//  Class: Bank  (manages all accounts)
// ─────────────────────────────────────────────
class Bank {
private:
    vector<Account> accounts;
    const string DATA_FILE = "bank_data.dat";
    int lastAccountNumber = 1000;

    void load() {
        ifstream fin(DATA_FILE);
        if (!fin.is_open()) return;
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            try {
                Account a = Account::deserialize(line);
                lastAccountNumber = max(lastAccountNumber, a.getAccountNumber());
                accounts.push_back(a);
            } catch (...) {}
        }
    }

    void save() {
        ofstream fout(DATA_FILE, ios::trunc);
        for (const auto& a : accounts)
            fout << a.serialize() << "\n";
    }

    Account* findAccount(int accNo) {
        for (auto& a : accounts)
            if (a.getAccountNumber() == accNo) return &a;
        return nullptr;
    }

public:
    Bank() { load(); }
    ~Bank() { save(); }

    // ── UI helpers ───────────────────────────
    void clearScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif
    }

    void printHeader(const string& title) {
        cout << "\n";
        cout << "╔══════════════════════════════════════════════════════╗\n";
        cout << "║          BANK MANAGEMENT APPLICATION  v1.0           ║\n";
        cout << "╠══════════════════════════════════════════════════════╣\n";
        cout << "║  " << left << setw(52) << title << "║\n";
        cout << "╚══════════════════════════════════════════════════════╝\n";
    }

    void pause() {
        cout << "\n  Press Enter to continue...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
    }

    int getInt(const string& prompt) {
        int val;
        while (true) {
            cout << prompt;
            if (cin >> val) { cin.ignore(); return val; }
            cout << "  ✗ Invalid. Enter a number.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    double getDouble(const string& prompt) {
        double val;
        while (true) {
            cout << prompt;
            if (cin >> val && val >= 0) { cin.ignore(); return val; }
            cout << "  ✗ Invalid amount.\n";
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
        }
    }

    string getString(const string& prompt) {
        string val;
        cout << prompt;
        getline(cin, val);
        return val;
    }

    void printAccountSummary(const Account& a) {
        cout << "\n"
             << "  ┌─────────────────────────────────────────┐\n"
             << "  │  Account No : " << setw(27) << a.getAccountNumber() << "│\n"
             << "  │  Name       : " << left << setw(27) << a.getHolderName() << "│\n"
             << "  │  Type       : " << setw(27) << a.getAccountType()   << "│\n"
             << "  │  Balance    : ₹" << fixed << setprecision(2)
                                      << setw(26) << a.getBalance()      << "│\n"
             << "  │  Opened     : " << setw(27) << a.getCreatedAt()     << "│\n"
             << "  └─────────────────────────────────────────┘\n";
    }

    // ── Banking operations ───────────────────

    void createAccount() {
        clearScreen();
        printHeader("CREATE NEW ACCOUNT");

        string name = getString("  Full Name      : ");
        if (name.empty()) { cout << "  ✗ Name cannot be empty.\n"; pause(); return; }

        cout << "  Account Type:\n  1. Savings\n  2. Current\n";
        int typeChoice = getInt("  Choose (1/2)   : ");
        string accType = (typeChoice == 2) ? "CURRENT" : "SAVINGS";

        double deposit = getDouble("  Initial Deposit (min ₹500): ₹");
        if (deposit < 500) {
            cout << "  ✗ Minimum initial deposit is ₹500.\n";
            pause(); return;
        }

        string newPin = getString("  Set 4-digit PIN: ");
        if (newPin.length() != 4 || newPin.find_first_not_of("0123456789") != string::npos) {
            cout << "  ✗ PIN must be exactly 4 digits.\n";
            pause(); return;
        }

        int accNo = ++lastAccountNumber;
        accounts.emplace_back(accNo, name, accType, deposit, newPin);
        save();

        cout << "\n  ✓ Account created successfully!\n";
        cout << "  ★ Your Account Number: " << accNo << "  (save this!)\n";
        pause();
    }

    void depositMoney() {
        clearScreen();
        printHeader("DEPOSIT MONEY");

        int accNo = getInt("  Account Number : ");
        Account* acc = findAccount(accNo);
        if (!acc) { cout << "  ✗ Account not found.\n"; pause(); return; }

        string pin = getString("  Enter PIN      : ");
        if (!acc->verifyPin(pin)) { cout << "  ✗ Incorrect PIN.\n"; pause(); return; }

        double amount = getDouble("  Deposit Amount : ₹");
        if (acc->deposit(amount)) {
            save();
            cout << "\n  ✓ ₹" << fixed << setprecision(2) << amount << " deposited.\n";
            cout << "  New Balance: ₹" << acc->getBalance() << "\n";
        } else {
            cout << "  ✗ Invalid amount.\n";
        }
        pause();
    }

    void withdrawMoney() {
        clearScreen();
        printHeader("WITHDRAW MONEY");

        int accNo = getInt("  Account Number : ");
        Account* acc = findAccount(accNo);
        if (!acc) { cout << "  ✗ Account not found.\n"; pause(); return; }

        string pin = getString("  Enter PIN      : ");
        if (!acc->verifyPin(pin)) { cout << "  ✗ Incorrect PIN.\n"; pause(); return; }

        cout << "  Current Balance: ₹" << fixed << setprecision(2) << acc->getBalance() << "\n";
        double amount = getDouble("  Withdraw Amount: ₹");

        if (acc->withdraw(amount)) {
            save();
            cout << "\n  ✓ ₹" << fixed << setprecision(2) << amount << " withdrawn.\n";
            cout << "  Remaining Balance: ₹" << acc->getBalance() << "\n";
        } else {
            cout << "  ✗ Insufficient balance or invalid amount.\n";
        }
        pause();
    }

    void checkBalance() {
        clearScreen();
        printHeader("BALANCE INQUIRY");

        int accNo = getInt("  Account Number : ");
        Account* acc = findAccount(accNo);
        if (!acc) { cout << "  ✗ Account not found.\n"; pause(); return; }

        string pin = getString("  Enter PIN      : ");
        if (!acc->verifyPin(pin)) { cout << "  ✗ Incorrect PIN.\n"; pause(); return; }

        printAccountSummary(*acc);
        pause();
    }

    void transferFunds() {
        clearScreen();
        printHeader("FUND TRANSFER");

        int fromAcc = getInt("  From Account   : ");
        Account* from = findAccount(fromAcc);
        if (!from) { cout << "  ✗ Source account not found.\n"; pause(); return; }

        string pin = getString("  Enter PIN      : ");
        if (!from->verifyPin(pin)) { cout << "  ✗ Incorrect PIN.\n"; pause(); return; }

        int toAcc = getInt("  To Account     : ");
        Account* to = findAccount(toAcc);
        if (!to) { cout << "  ✗ Destination account not found.\n"; pause(); return; }
        if (fromAcc == toAcc) { cout << "  ✗ Cannot transfer to same account.\n"; pause(); return; }

        cout << "  From Balance: ₹" << fixed << setprecision(2) << from->getBalance() << "\n";
        double amount = getDouble("  Transfer Amount: ₹");

        if (from->transferOut(amount)) {
            to->transferIn(amount);
            save();
            cout << "\n  ✓ ₹" << fixed << setprecision(2) << amount
                 << " transferred to Account " << toAcc << "\n";
            cout << "  Your Remaining Balance: ₹" << from->getBalance() << "\n";
        } else {
            cout << "  ✗ Insufficient balance or invalid amount.\n";
        }
        pause();
    }

    void viewMiniStatement() {
        clearScreen();
        printHeader("MINI STATEMENT");

        int accNo = getInt("  Account Number : ");
        Account* acc = findAccount(accNo);
        if (!acc) { cout << "  ✗ Account not found.\n"; pause(); return; }

        string pin = getString("  Enter PIN      : ");
        if (!acc->verifyPin(pin)) { cout << "  ✗ Incorrect PIN.\n"; pause(); return; }

        const auto& txs = acc->getTransactions();
        int start = max(0, (int)txs.size() - 10);

        cout << "\n  Last " << (txs.size() - start) << " transactions for Account " << accNo << "\n";
        cout << "  " << string(60, '-') << "\n";
        cout << "  " << left << setw(14) << "Type"
                              << setw(12) << "Amount"
                              << setw(14) << "Balance"
                              << "Timestamp\n";
        cout << "  " << string(60, '-') << "\n";

        for (int i = start; i < (int)txs.size(); ++i) {
            const auto& tx = txs[i];
            cout << "  " << left << setw(14) << tx.type
                         << "₹" << setw(11) << fixed << setprecision(2) << tx.amount
                         << "₹" << setw(13) << tx.balanceAfter
                         << tx.timestamp << "\n";
        }
        pause();
    }

    void displayAllAccounts() {
        clearScreen();
        printHeader("ALL ACCOUNTS  [ADMIN VIEW]");

        if (accounts.empty()) {
            cout << "\n  No accounts found.\n";
            pause(); return;
        }

        cout << "\n  " << left
             << setw(10) << "Acc No"
             << setw(22) << "Name"
             << setw(10) << "Type"
             << setw(14) << "Balance"
             << "Opened\n";
        cout << "  " << string(70, '-') << "\n";

        double totalDeposits = 0;
        for (const auto& a : accounts) {
            cout << "  " << left
                 << setw(10) << a.getAccountNumber()
                 << setw(22) << a.getHolderName()
                 << setw(10) << a.getAccountType()
                 << "₹" << setw(13) << fixed << setprecision(2) << a.getBalance()
                 << a.getCreatedAt() << "\n";
            totalDeposits += a.getBalance();
        }

        cout << "  " << string(70, '-') << "\n";
        cout << "  Total Accounts : " << accounts.size() << "\n";
        cout << "  Total Funds    : ₹" << fixed << setprecision(2) << totalDeposits << "\n";
        pause();
    }

    void deleteAccount() {
        clearScreen();
        printHeader("DELETE ACCOUNT");

        int accNo = getInt("  Account Number : ");
        auto it = find_if(accounts.begin(), accounts.end(),
                          [accNo](const Account& a){ return a.getAccountNumber() == accNo; });

        if (it == accounts.end()) { cout << "  ✗ Account not found.\n"; pause(); return; }

        string pin = getString("  Enter PIN      : ");
        if (!it->verifyPin(pin)) { cout << "  ✗ Incorrect PIN.\n"; pause(); return; }

        printAccountSummary(*it);
        string confirm = getString("\n  Type 'DELETE' to confirm: ");
        if (confirm == "DELETE") {
            accounts.erase(it);
            save();
            cout << "  ✓ Account deleted successfully.\n";
        } else {
            cout << "  – Deletion cancelled.\n";
        }
        pause();
    }

    // ── Main Menu ────────────────────────────
    void run() {
        int choice;
        do {
            clearScreen();
            printHeader("MAIN MENU");
            cout << "\n"
                 << "  ── Customer ──────────────────────\n"
                 << "  1.  Create New Account\n"
                 << "  2.  Deposit Money\n"
                 << "  3.  Withdraw Money\n"
                 << "  4.  Check Balance\n"
                 << "  5.  Fund Transfer\n"
                 << "  6.  Mini Statement\n"
                 << "\n"
                 << "  ── Admin ─────────────────────────\n"
                 << "  7.  View All Accounts\n"
                 << "  8.  Delete Account\n"
                 << "\n"
                 << "  0.  Exit\n\n";

            choice = getInt("  Enter choice: ");

            switch (choice) {
                case 1: createAccount();     break;
                case 2: depositMoney();      break;
                case 3: withdrawMoney();     break;
                case 4: checkBalance();      break;
                case 5: transferFunds();     break;
                case 6: viewMiniStatement(); break;
                case 7: displayAllAccounts();break;
                case 8: deleteAccount();     break;
                case 0:
                    clearScreen();
                    cout << "\n  Thank you for using the Bank Management System!\n"
                         << "  Data saved to 'bank_data.dat'.\n\n";
                    break;
                default:
                    cout << "\n  ✗ Invalid option.\n";
                    pause();
            }
        } while (choice != 0);
    }
};

// ─────────────────────────────────────────────
//  Entry point
// ─────────────────────────────────────────────
int main() {
    Bank bank;
    bank.run();
    return 0;
}
