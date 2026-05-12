#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100
#define PASSWORD "admin123"

// structure
struct clientData {
    unsigned int acctNum;
    char lastName[15];
    char firstName[10];
    double balance;
};

// prototypes
unsigned int menu();
int login();
void textFile(FILE *);
void updateRecord(FILE *);
void newRecord(FILE *);
void deleteRecord(FILE *);
void searchByName(FILE *);
void deposit(FILE *);
void withdraw(FILE *);
int countAccounts(FILE *);
void logTransaction(int, double, const char *);

int main() {
    FILE *cfPtr;

    if (!login()) {
        printf("Access Denied!\n");
        return 0;
    }

    // open file
    if ((cfPtr = fopen("credit.dat", "rb+")) == NULL) {
        printf("File not found. Creating new file...\n");

        cfPtr = fopen("credit.dat", "wb+");
        if (cfPtr == NULL) {
            printf("File creation failed!\n");
            return 1;
        }

        struct clientData blank = {0, "", "", 0.0};
        for (int i = 0; i < MAX; i++)
            fwrite(&blank, sizeof(struct clientData), 1, cfPtr);
    }

    unsigned int choice;

    while ((choice = menu()) != 9) {
        switch (choice) {
            case 1: textFile(cfPtr); break;
            case 2: updateRecord(cfPtr); break;
            case 3: newRecord(cfPtr); break;
            case 4: deleteRecord(cfPtr); break;
            case 5: searchByName(cfPtr); break;
            case 6: deposit(cfPtr); break;
            case 7: withdraw(cfPtr); break;
            case 8: printf("Total accounts: %d\n", countAccounts(cfPtr)); break;
            default: printf("Invalid choice\n");
        }
    }

    fclose(cfPtr);
    return 0;
}

// login
int login() {
    char pass[20];
    printf("Enter password: ");
    scanf("%19s", pass);
    return strcmp(pass, PASSWORD) == 0;
}

// menu
unsigned int menu() {
    unsigned int c;
    printf("\n1.Export\n2.Update\n3.Add\n4.Delete\n5.Search\n6.Deposit\n7.Withdraw\n8.Count\n9.Exit\nChoice: ");
    scanf("%u", &c);
    return c;
}

// export
void textFile(FILE *fPtr) {
    FILE *out = fopen("accounts.txt", "w");
    if (out == NULL) {
        printf("Error creating text file\n");
        return;
    }

    struct clientData c;
    rewind(fPtr);

    fprintf(out, "%-6s%-16s%-11s%10s\n", "Acct", "Last", "First", "Balance");

    while (fread(&c, sizeof(c), 1, fPtr) == 1) {
        if (c.acctNum != 0)
            fprintf(out, "%-6d%-16s%-11s%10.2f\n",
                    c.acctNum, c.lastName, c.firstName, c.balance);
    }

    fclose(out);
    printf("Exported to accounts.txt\n");
}

// add
void newRecord(FILE *fPtr) {
    struct clientData c = {0, "", "", 0.0};
    unsigned int acc;

    printf("Enter account (1-100): ");
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX) {
        printf("Invalid account number\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);

    if (fread(&c, sizeof(c), 1, fPtr) != 1) return;

    if (c.acctNum != 0) {
        printf("Account exists!\n");
        return;
    }

    printf("Enter last first balance: ");
    scanf("%14s%9s%lf", c.lastName, c.firstName, &c.balance);

    c.acctNum = acc;

    fseek(fPtr, -sizeof(c), SEEK_CUR);
    fwrite(&c, sizeof(c), 1, fPtr);
}

// update
void updateRecord(FILE *fPtr) {
    unsigned int acc;
    double amt;
    struct clientData c;

    printf("Enter account: ");
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX) {
        printf("Invalid account\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);

    if (fread(&c, sizeof(c), 1, fPtr) != 1 || c.acctNum == 0) {
        printf("Account not found\n");
        return;
    }

    printf("Enter amount (+/-): ");
    scanf("%lf", &amt);

    c.balance += amt;

    fseek(fPtr, -sizeof(c), SEEK_CUR);
    fwrite(&c, sizeof(c), 1, fPtr);

    logTransaction(acc, amt, "Update");
}

// delete
void deleteRecord(FILE *fPtr) {
    struct clientData blank = {0, "", "", 0};
    unsigned int acc;

    printf("Enter account: ");
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX) {
        printf("Invalid account\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(blank), SEEK_SET);
    fwrite(&blank, sizeof(blank), 1, fPtr);

    printf("Account deleted\n");
}

// search
void searchByName(FILE *fPtr) {
    char name[15];
    struct clientData c;
    int found = 0;

    printf("Enter last name: ");
    scanf("%14s", name);

    rewind(fPtr);

    while (fread(&c, sizeof(c), 1, fPtr) == 1) {
        if (c.acctNum != 0 && strcmp(c.lastName, name) == 0) {
            printf("%d %s %s %.2f\n",
                   c.acctNum, c.lastName, c.firstName, c.balance);
            found = 1;
        }
    }

    if (!found)
        printf("No records found\n");
}

// deposit
void deposit(FILE *fPtr) {
    unsigned int acc;
    double amt;
    struct clientData c;

    printf("Enter account: ");
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX) {
        printf("Invalid account\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);

    if (fread(&c, sizeof(c), 1, fPtr) != 1 || c.acctNum == 0) {
        printf("Account not found\n");
        return;
    }

    printf("Enter amount: ");
    scanf("%lf", &amt);

    if (amt <= 0) {
        printf("Invalid amount\n");
        return;
    }

    c.balance += amt;

    fseek(fPtr, -sizeof(c), SEEK_CUR);
    fwrite(&c, sizeof(c), 1, fPtr);

    logTransaction(acc, amt, "Deposit");
}

// withdraw
void withdraw(FILE *fPtr) {
    unsigned int acc;
    double amt;
    struct clientData c;

    printf("Enter account: ");
    scanf("%u", &acc);

    if (acc < 1 || acc > MAX) {
        printf("Invalid account\n");
        return;
    }

    fseek(fPtr, (acc - 1) * sizeof(c), SEEK_SET);

    if (fread(&c, sizeof(c), 1, fPtr) != 1 || c.acctNum == 0) {
        printf("Account not found\n");
        return;
    }

    printf("Enter amount: ");
    scanf("%lf", &amt);

    if (amt <= 0 || amt > c.balance) {
        printf("Invalid or insufficient balance\n");
        return;
    }

    c.balance -= amt;

    fseek(fPtr, -sizeof(c), SEEK_CUR);
    fwrite(&c, sizeof(c), 1, fPtr);

    logTransaction(acc, -amt, "Withdraw");
}

// count
int countAccounts(FILE *fPtr) {
    struct clientData c;
    int count = 0;

    rewind(fPtr);

    while (fread(&c, sizeof(c), 1, fPtr) == 1)
        if (c.acctNum != 0)
            count++;

    return count;
}

// log
void logTransaction(int acc, double amt, const char *type) {
    FILE *fp = fopen("transactions.txt", "a");
    if (fp == NULL) return;

    fprintf(fp, "Acc:%d Type:%s Amount:%.2f\n", acc, type, amt);
    fclose(fp);
}