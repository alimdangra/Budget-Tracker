#include <stdio.h>
#include <string.h>

// 1. Expense Structure
struct Expense {
    float amount;
    char category[30];
    char date[20];
    char place[50];
    char paymentMethod[20];
    char status[20]; // Paid / Unpaid
};

// Global Array and Counter for Expenses
struct Expense expenses[100];
int expenseCount = 0;

// Function Prototypes for Expense Management
void addExpense();
void viewExpenses();
void searchByCategory();

int main() {
    int option;
    do {
        printf("\n--- EXPENSE MANAGEMENT MODULE ---\n");
        printf("1. Add Expense\n");
        printf("2. View All Expenses\n");
        printf("3. Search Expenses by Category\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        switch(option) {
            case 1:
                addExpense();
                break;
            case 2:
                viewExpenses();
                break;
            case 3:
                searchByCategory();
                break;
            case 4:
                printf("Exiting Expense Module...\n");
                break;
            default:
                printf("Invalid choice! Try again.\n");
        }
    } while(option != 4);

    return 0;
}

// 1. Add Expense Function
void addExpense() {
    if (expenseCount >= 100) {
        printf("\nError: Expense storage is full!\n");
        return;
    }

    printf("\n--- Add New Expense ---\n");
    printf("Enter amount: ");
    scanf("%f", &expenses[expenseCount].amount);
    
    printf("Enter category (e.g., Food, Bills, Transport): ");
    scanf("%s", expenses[expenseCount].category);
    
    printf("Enter date (DD-MM-YYYY): ");
    scanf("%s", expenses[expenseCount].date);
    
    printf("Enter place (e.g., Supermarket): ");
    scanf("%s", expenses[expenseCount].place);
    
    printf("Enter payment method (Cash/Bank): ");
    scanf("%s", expenses[expenseCount].paymentMethod);
    
    printf("Enter status (Paid/Unpaid): ");
    scanf("%s", expenses[expenseCount].status);

    expenseCount++;
    printf("Expense added successfully!\n");
}

// 2. View All Expenses Function
void viewExpenses() {
    printf("\n=================================================================\n");
    printf("                        ALL EXPENSES LIST                        \n");
    printf("=================================================================\n");
    
    if (expenseCount == 0) {
        printf("No expense records found.\n");
        return;
    }

    for (int i = 0; i < expenseCount; i++) {
        printf("%d. Amount: %.2f | Category: %s | Date: %s | Place: %s | Method: %s | Status: %s\n",
            i + 1,
            expenses[i].amount,
            expenses[i].category,
            expenses[i].date,
            expenses[i].place,
            expenses[i].paymentMethod,
            expenses[i].status);
    }
}

// 3. Search & Filter Using Loops and Conditions
void searchByCategory() {
    if (expenseCount == 0) {
        printf("\nNo expenses available to search.\n");
        return;
    }

    char searchCat[30];
    printf("\nEnter category to search (e.g., Food): ");
    scanf("%s", searchCat);

    float totalCategoryExpense = 0;
    int found = 0;

    printf("\n--- Results for Category: %s ---\n", searchCat);
    
    // Loop through all records using conditions
    for (int i = 0; i < expenseCount; i++) {
        if (strcmp(expenses[i].category, searchCat) == 0) {
            found = 1;
            totalCategoryExpense += expenses[i].amount;
            printf("- Amount: %.2f | Date: %s | Place: %s | Status: %s\n",
            expenses[i].amount, expenses[i].date, expenses[i].place, expenses[i].status);
        }
    }

    if (found) {
        printf("-------------------------------------------------\n");
        printf("Total spent in [%s]: %.2f\n", searchCat, totalCategoryExpense);
    } else {
        printf("No expenses found under category '%s'.\n", searchCat);
    }
}