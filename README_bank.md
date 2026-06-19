# Bank Management Application (C++)

A console-based Bank Management Application using **Object-Oriented Programming** and **file handling** in C++.

## OOP Design

| Class | Responsibility |
|---|---|
| `Transaction` | Stores a single transaction (type, amount, balance, timestamp) |
| `Account` | Manages one bank account — all operations and transaction history |
| `Bank` | Manages the collection of accounts, file I/O, and the main menu |

## Features

| Feature | Details |
|---|---|
| **Create Account** | Name, type (Savings/Current), initial deposit (min ₹500), 4-digit PIN |
| **Deposit** | Add funds — verified by PIN |
| **Withdraw** | Remove funds — checks balance, verified by PIN |
| **Balance Check** | Full account summary — verified by PIN |
| **Fund Transfer** | Move money between two accounts — verified by PIN |
| **Mini Statement** | Last 10 transactions with type, amount, balance, timestamp |
| **View All Accounts** | Admin view: all accounts and total funds |
| **Delete Account** | Remove account — requires PIN + typing "DELETE" |
| **File Persistence** | All data saved to `bank_data.dat` between runs |

## Build & Run

```bash
# Compile
make
# or
g++ -std=c++17 -Wall -O2 -o bank_management bank_management.cpp

# Run
./bank_management        # Linux / macOS
.\bank_management.exe    # Windows

# Clean
make clean
```

## Data Storage

Data is stored in `bank_data.dat` using a custom serialization format. Each line represents one account, including all transaction history.

## File Structure

```
bank_management/
├── bank_management.cpp   # Full source (OOP, single file)
├── Makefile
├── README.md
└── bank_data.dat         # Created on first run
```

## Security Notes (Demo)
- PIN is used to authenticate all operations
- Minimum deposit of ₹500 enforced on account creation
- Transfer validation prevents same-account transfers
- Deletion requires typing "DELETE" as confirmation
