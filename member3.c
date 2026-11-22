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

// Income Structure (added for editIncome & deleteIncome)
struct Income {
    float amount;
    char source[30];
    char date[20];
    char paymentMethod[20];
};

// Global Array and Counter for Expenses
struct Expense expenses[100];
int expenseCount = 0;

// Global Array and Counter for Incomes
struct Income incomes[100];
int incomeCount = 0;

// Function Prototypes for Expense Management
void addExpense();
void viewExpenses();
void searchByCategory();
void editExpense();
void deleteExpense();
void editIncome();
void deleteIncome();

int main() {
    int choice;
    do {
        printf("\n--- EXPENSE MANAGEMENT MODULE ---\n");
        printf("1. Add Expense\n");
        printf("2. View All Expenses\n");
        printf("3. Search Expenses by Category\n");
        printf("4. Edit Expense\n");
        printf("5. Delete Expense\n");
        printf("6. Edit Income\n");
        printf("7. Delete Income\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch(choice) {
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
                editExpense();
                break;
            case 5:
                deleteExpense();
                break;
            case 6:
                editIncome();
                break;
            case 7:
                deleteIncome();
                break;
            case 8:
                printf("Exiting Expense Module...\n");
                break;
            default:
                printf("Invalid choice! Try again.\n");
        }
    } while(choice != 8);

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

// 4. Edit Expense Function
void editExpense() {
    if (expenseCount == 0) {
        printf("\nNo expense records available to edit.\n");
        return;
    }

    viewExpenses();
    int index;
    printf("\nEnter expense number to edit (1-%d): ", expenseCount);
    scanf("%d", &index);

    if (index < 1 || index > expenseCount) {
        printf("Invalid expense number!\n");
        return;
    }

    int i = index - 1;
    printf("Enter new amount: ");
    scanf("%f", &expenses[i].amount);

    printf("Enter new category (e.g., Food, Bills, Transport): ");
    scanf("%s", expenses[i].category);

    printf("Enter new date (DD-MM-YYYY): ");
    scanf("%s", expenses[i].date);

    printf("Enter new place (e.g., Supermarket): ");
    scanf("%s", expenses[i].place);

    printf("Enter new payment method (Cash/Bank): ");
    scanf("%s", expenses[i].paymentMethod);

    printf("Enter new status (Paid/Unpaid): ");
    scanf("%s", expenses[i].status);

    printf("Expense updated successfully!\n");
}

// 5. Delete Expense Function
void deleteExpense() {
    if (expenseCount == 0) {
        printf("\nNo expense records available to delete.\n");
        return;
    }

    viewExpenses();
    int index;
    printf("\nEnter expense number to delete (1-%d): ", expenseCount);
    scanf("%d", &index);

    if (index < 1 || index > expenseCount) {
        printf("Invalid expense number!\n");
        return;
    }

    for (int i = index - 1; i < expenseCount - 1; i++) {
        expenses[i] = expenses[i + 1];
    }
    expenseCount--;

    printf("Expense deleted successfully!\n");
}

// 6. Edit Income Function
void editIncome() {
    if (incomeCount == 0) {
        printf("\nNo income records available to edit.\n");
        return;
    }

    int index;
    printf("\nEnter income number to edit (1-%d): ", incomeCount);
    scanf("%d", &index);

    if (index < 1 || index > incomeCount) {
        printf("Invalid income number!\n");
        return;
    }

    int i = index - 1;
    printf("Enter new amount: ");
    scanf("%f", &incomes[i].amount);

    printf("Enter new source: ");
    scanf("%s", incomes[i].source);

    printf("Enter new date (DD-MM-YYYY): ");
    scanf("%s", incomes[i].date);

    printf("Enter new payment method (Cash/Bank): ");
    scanf("%s", incomes[i].paymentMethod);

    printf("Income updated successfully!\n");
}

// 7. Delete Income Function
void deleteIncome() {
    if (incomeCount == 0) {
        printf("\nNo income records available to delete.\n");
        return;
    }

    int index;
    printf("\nEnter income number to delete (1-%d): ", incomeCount);
    scanf("%d", &index);

    if (index < 1 || index > incomeCount) {
        printf("Invalid income number!\n");
        return;
    }

    for (int i = index - 1; i < incomeCount - 1; i++) {
        incomes[i] = incomes[i + 1];
    }
    incomeCount--;

    printf("Income deleted successfully!\n");
}
