/* This is free and unencumbered software released into the public domain.
 * Refer to LICENSE.txt in this directory. */

/*
 * Program demonstrating a result which differs between runs of the same binary with the same input.
 *
 * The ledger below should always balance to zero, but it sometimes does not. Run it several times
 * to see both outcomes. UDB disables address-space layout randomization (ASLR) by default, so
 * every run under UDB takes the same path: use `set disable-randomization off` in UDB, or record
 * with `live-record` (which keeps ASLR enabled), to capture a run where the ledger does not balance.
 */

#include <assert.h>

#include <iostream>
#include <unordered_set>
#include <vector>

struct Account
{
    const char *name;
    double balance;
};

int
main()
{
    /* The customers pay all their money in fees, so these accounts balance. */
    std::vector<Account> customers_and_fees = {
        { "alice", 1.0 },
        { "bob", 2.0 },
        { "carol", 3.0 },
        { "dave", 4.0 },
        { "fees", -10.0 },
    };

    std::unordered_set<const Account *> accounts;
    for (const Account &account : customers_and_fees)
    {
        accounts.insert(&account);
    }

    /* Moves the reserve out of the bank, so these two accounts cancel each other out. */
    Account reserve = { "reserve", 1e17 };
    Account transfer = { "transfer", -1e17 };
    accounts.insert(&reserve);
    accounts.insert(&transfer);

    double total = 0.0;
    for (const Account *account : accounts)
    {
        std::cout << account->name << ": " << account->balance << "\n";
        total += account->balance;
    }
    /* Use std::endl to flush the output before the assertion can abort the program. */
    std::cout << "Total: " << total << std::endl;

    /* Every credit has a matching debit, so the ledger must balance. */
    assert(total == 0.0);

    return EXIT_SUCCESS;
}
