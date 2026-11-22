#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_INCOME 100

struct Income
{
    float amount;
    char source[30];
    char date[20];
    char paymentMethod[20];
};

struct User
{
    char name[50];
    int age;
    char phone[20];
    char city[30];
};

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


struct Income incomes[MAX_INCOME];
int incomeCount = 0;
struct User user;
int userSet = 0;

char username[30];
char password[30];
char userFile[50];
char incomeFile[50];
char expenseFile[60];
char savingFile[60];
float savingTarget = 0;


void setUserInformation();
void loadData();

void saveExpenses(void);
void viewExpenses(void);
void addExpense(void);
void searchByCategory(void);
void saveSavingTarget(void);
float getTotalExpense(void);
void getBalances(float *bankbal, float *cashbal);
int calculatebalance(float exp, int opt, float *bankbal, float *cashbal);
void checkwarning(float income, float totalexpense, float save);
void categoryreport(int size, float expense[][4]);


void removeNewline(char text[])
{
    text[strcspn(text, "\n")] = '\0';
}

void readText(char prompt[], char text[], int size)
{
    printf("%s", prompt);
    fgets(text, size, stdin);
    removeNewline(text);
}

int readInt(char prompt[])
{
    char input[50];
    readText(prompt, input, sizeof(input));
    return atoi(input);
}

float readFloat(char prompt[])
{
    char input[50];
    readText(prompt, input, sizeof(input));
    return atof(input);
}

void makeFileNames()
{
    sprintf(userFile, "%s_user.txt", username);
    sprintf(incomeFile, "%s_income.txt", username);
    sprintf(expenseFile, "%s_expense.txt", username);
    sprintf(savingFile, "%s_saving.txt", username);
}

int usernameExists(char name[])
{
    FILE *fp = fopen("accounts.txt", "r");
    char savedName[30];
    char savedPassword[30];

    if (fp == NULL)
    {
        return 0;
    }

    while (fgets(savedName, sizeof(savedName), fp) != NULL)
    {
        removeNewline(savedName);

        if (fgets(savedPassword, sizeof(savedPassword), fp) == NULL)
        {
            break;
        }
        removeNewline(savedPassword);

        if (strcmp(savedName, name) == 0)
        {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

int signUp()
{
    FILE *fp;
    char confirmPassword[30];

    printf("\n--- Sign Up ---\n");
    readText("Choose username (no spaces): ", username, sizeof(username));

    if (usernameExists(username))
    {
        printf("This username already exists.\n");
        return 0;
    }

    readText("Create password: ", password, sizeof(password));
    readText("Confirm password: ", confirmPassword, sizeof(confirmPassword));

    if (strcmp(password, confirmPassword) != 0)
    {
        printf("Passwords do not match.\n");
        return 0;
    }

    fp = fopen("accounts.txt", "a");
    if (fp == NULL)
    {
        printf("Could not create account.\n");
        return 0;
    }

    fprintf(fp, "%s\n%s\n", username, password);
    fclose(fp);

    makeFileNames();

    printf("\nAccount created successfully!\n");
    setUserInformation();

    return 1;
}

int login()
{
    FILE *fp = fopen("accounts.txt", "r");
    char savedName[30];
    char savedPassword[30];

    if (fp == NULL)
    {
        printf("No accounts exist. Please sign up first.\n");
        return 0;
    }

    printf("\n--- Login ---\n");
    readText("Username: ", username, sizeof(username));
    readText("Password: ", password, sizeof(password));

    while (fgets(savedName, sizeof(savedName), fp) != NULL)
    {
        removeNewline(savedName);

        if (fgets(savedPassword, sizeof(savedPassword), fp) == NULL)
        {
            break;
        }
        removeNewline(savedPassword);

        if (strcmp(username, savedName) == 0 &&
            strcmp(password, savedPassword) == 0)
        {
            fclose(fp);
            makeFileNames();
            loadData();
            if (!userSet) setUserInformation();
            printf("\nLogin successful. Welcome, %s!\n", user.name);
            return 1;
        }
    }

    fclose(fp);
    printf("Incorrect username or password.\n");
    return 0;
}

void saveUser()
{
    FILE *fp = fopen(userFile, "w");

    if (fp == NULL)
    {
        printf("Error: could not save user information!\n");
        return;
    }

    fprintf(fp, "%s\n%d\n%s\n%s\n", user.name, user.age, user.phone, user.city);
    fclose(fp);
}

void saveIncomes()
{
    int n;
    FILE *fp = fopen(incomeFile, "w");

    if (fp == NULL)
    {
        printf("Error: could not save incomes!\n");
        return;
    }

    for (n = 0; n < incomeCount; n++)
    {
        fprintf(fp, "%.2f\n%s\n%s\n%s\n",
                incomes[n].amount,
                incomes[n].source,
                incomes[n].date,
                incomes[n].paymentMethod);
    }

    fclose(fp);
}

void loadData()
{
    FILE *fp;
    char line[100];

    incomeCount = 0;
    expenseCount = 0;
    userSet = 0;
    savingTarget = 0;

    fp = fopen(userFile, "r");
    if (fp != NULL)
    {
        if (fgets(user.name, sizeof(user.name), fp) != NULL)
        {
            removeNewline(user.name);

            if (fgets(line, sizeof(line), fp) != NULL)
            {
                user.age = atoi(line);
            }

            if (fgets(user.phone, sizeof(user.phone), fp) != NULL)
            {
                removeNewline(user.phone);
            }

            if (fgets(user.city, sizeof(user.city), fp) != NULL)
            {
                removeNewline(user.city);
            }

            userSet = 1;
        }

        fclose(fp);
    }

    fp = fopen(incomeFile, "r");
    if (fp != NULL)
    {
        while (incomeCount < MAX_INCOME)
        {
            if (fgets(line, sizeof(line), fp) == NULL)
            {
                break;
            }

            incomes[incomeCount].amount = atof(line);

            if (fgets(incomes[incomeCount].source, sizeof(incomes[incomeCount].source), fp) == NULL)
            {
                break;
            }
            removeNewline(incomes[incomeCount].source);

            if (fgets(incomes[incomeCount].date, sizeof(incomes[incomeCount].date), fp) == NULL)
            {
                break;
            }
            removeNewline(incomes[incomeCount].date);

            if (fgets(incomes[incomeCount].paymentMethod, sizeof(incomes[incomeCount].paymentMethod), fp) == NULL)
            {
                break;
            }
            removeNewline(incomes[incomeCount].paymentMethod);

            incomeCount++;
        }

        fclose(fp);
    }

    /* Load expenses for the currently logged-in user */
    fp = fopen(expenseFile, "r");
    if (fp != NULL) {
        while (expenseCount < 100) {
            struct Expense *e = &expenses[expenseCount];
            if (!fgets(line, sizeof(line), fp)) break;
            e->amount = (float)atof(line);
            if (!fgets(e->category, sizeof(e->category), fp)) break;
            removeNewline(e->category);
            if (!fgets(e->date, sizeof(e->date), fp)) break;
            removeNewline(e->date);
            if (!fgets(e->place, sizeof(e->place), fp)) break;
            removeNewline(e->place);
            if (!fgets(e->paymentMethod, sizeof(e->paymentMethod), fp)) break;
            removeNewline(e->paymentMethod);
            if (!fgets(e->status, sizeof(e->status), fp)) break;
            removeNewline(e->status);
            expenseCount++;
        }
        fclose(fp);
    }
    fp = fopen(savingFile, "r");
    if (fp != NULL) {
        if (fgets(line, sizeof(line), fp)) savingTarget = (float)atof(line);
        fclose(fp);
    }
}

void setUserInformation()
{
    printf("\n--- Enter User Information ---\n");

    readText("Enter name: ", user.name, sizeof(user.name));

    do
    {
        user.age = readInt("Enter age: ");

        if (user.age <= 0 || user.age > 120)
        {
            printf("Invalid age! Please enter again.\n");
        }
    } while (user.age <= 0 || user.age > 120);

    readText("Enter phone number: ", user.phone, sizeof(user.phone));
    readText("Enter city: ", user.city, sizeof(user.city));

    userSet = 1;
    saveUser();
    printf("User information saved successfully!\n");
}

void showUserInformation()
{
    if (userSet == 0)
    {
        printf("\nUser information has not been added yet.\n");
        return;
    }

    printf("\n--- User Information ---\n");
    printf("Name  : %s\n", user.name);
    printf("Age   : %d\n", user.age);
    printf("Phone : %s\n", user.phone);
    printf("City  : %s\n", user.city);
}

void addIncome()
{
    if (incomeCount >= MAX_INCOME)
    {
        printf("\nIncome list is full! Cannot add more income.\n");
        return;
    }

    struct Income *i = &incomes[incomeCount];

    printf("\n--- Add Income ---\n");

    do
    {
        i->amount = readFloat("Enter amount: ");

        if (i->amount <= 0)
        {
            printf("Amount must be greater than 0!\n");
        }
    } while (i->amount <= 0);

    readText("Enter source (e.g. Salary, Business, Pocket Money): ",
             i->source, sizeof(i->source));
    readText("Enter date (DD-MM-YYYY): ", i->date, sizeof(i->date));

    int choice;

    do
    {
        choice = readInt("Payment method (1 = Cash, 2 = Bank Transfer): ");

        if (choice == 1)
        {
            strcpy(i->paymentMethod, "Cash");
        }
        else if (choice == 2)
        {
            strcpy(i->paymentMethod, "Bank Transfer");
        }
        else
        {
            printf("Please enter 1 or 2.\n");
        }
    } while (choice != 1 && choice != 2);

    incomeCount++;
    saveIncomes();
    printf("Income added successfully!\n");
}

void viewIncomes()
{
    int n;

    if (incomeCount == 0)
    {
        printf("\nNo income has been added yet.\n");
        return;
    }

    printf("\n--- All Incomes ---\n");
    printf("%-4s %-10s %-20s %-12s %-15s\n",
           "No", "Amount", "Source", "Date", "Payment");
    printf("-----------------------------------------------------------------\n");

    for (n = 0; n < incomeCount; n++)
    {
        printf("%-4d %-10.2f %-20s %-12s %-15s\n",
               n + 1,
               incomes[n].amount,
               incomes[n].source,
               incomes[n].date,
               incomes[n].paymentMethod);
    }
}

float getTotalIncome()
{
    float total = 0;
    int n;

    for (n = 0; n < incomeCount; n++)
    {
        total = total + incomes[n].amount;
    }

    return total;
}

void showTotalIncome()
{
    printf("\nTotal Income = %.2f\n", getTotalIncome());
}

void deleteAccount()
{
    FILE *fp;
    FILE *temp;
    char savedName[30];
    char savedPassword[30];
    char confirmation[10];

    readText("Type YES to permanently delete your account: ",
             confirmation, sizeof(confirmation));

    if (strcmp(confirmation, "YES") != 0)
    {
        printf("Account deletion cancelled.\n");
        return;
    }

    fp = fopen("accounts.txt", "r");
    temp = fopen("temp.txt", "w");

    if (fp == NULL || temp == NULL)
    {
        printf("Could not delete account.\n");
        if (fp != NULL) fclose(fp);
        if (temp != NULL) fclose(temp);
        return;
    }

    while (fgets(savedName, sizeof(savedName), fp) != NULL)
    {
        removeNewline(savedName);

        if (fgets(savedPassword, sizeof(savedPassword), fp) == NULL)
        {
            break;
        }
        removeNewline(savedPassword);

        if (strcmp(savedName, username) != 0)
        {
            fprintf(temp, "%s\n%s\n", savedName, savedPassword);
        }
    }

    fclose(fp);
    fclose(temp);

    if (remove("accounts.txt") != 0 || rename("temp.txt", "accounts.txt") != 0) {
        printf("Warning: Could not finish account deletion. Check account files.\n");
        return;
    }

    remove(userFile);
    remove(incomeFile);
    remove(expenseFile);
    remove(savingFile);

    printf("Your account and saved income data have been deleted.\n");
}


/*----------ALI PART------------*/
/*idhr aik calculate balance ke naams se function bage 
iske andr ham do parameters de rahe income, expense and option ke kahan
se kharch kiye hai paiese for example cash yaa bank se 
bank balance current bank balance dega aur cashbal cash balance dega
as a parameter */
int calculatebalance(float exp,int opt,
    float *bankbal,float *cashbal)/*void is liy euse kiya hai kiyonke hamne 
    pointers use kiye hai woh already change kar deta hai values  */{
    if(exp < 0 ){
            printf("invalid amount \n");
            return 0 ; 
        }
    else if (opt == 1){
        if(exp > *bankbal){/*expense check karne ke liye agr
            amount bari hogi balance se to woh decline kar dega request */
            printf("insufficientbalance \n");
            return 0;
        }
        else{
            *bankbal = *bankbal - exp;/*bank balance update ho raha hai */
            /*updated balance kpo out put kar raha hai*/
            printf("Bank balance is: %.2f \n",*bankbal);
            printf("cash balance is: %.2f \n",*cashbal);
            printf("Total balance is: %.2f \n",*cashbal + *bankbal);
            return 1;
        }
    }
    else if (opt == 2){
        if(exp > *cashbal){/*expense check karne ke liye agr
            amount bari hogi balance se to woh decline kar dega request */
            printf("insufficientbalance \n");
            return 0 ;
        }
        else{
            *cashbal = *cashbal - exp ;/*cas balance updatre ho raha hai*/
            /*updated balance kpo out put kar raha hai*/
            printf("Bank balance is: %.2f \n",*bankbal);
            printf("cash balance is: %.2f \n",*cashbal);
            printf("Total balance is: %.2f \n",*cashbal + *bankbal);
            return 1;
        }
    }
    else{
        printf("invalid option \n");
        return 0;
    }


}
/*function check warning*/
void checkwarning(float income,float totalexpense,float save){
    /*pehli condition check hogi yahan pe*/
    if (totalexpense >= (0.9 * income)){
        printf("Warning!! you are exceeding 90%% of the amount \n");
    }
    /*dusri condition*/
    else if (totalexpense >= (0.85 * income)){
        printf("Warning!! you arre exceeding 85%% of the amount \n");
    }
    /*teesri condition*/
    else if (totalexpense >= (0.8 * income)){
        printf("Warning!! you are exceeding 80%% of the amount \n");
    }
    if (totalexpense > (income - save)){
        printf("You are now in your saving zone \n");

    }
    
    
    
}
/*function setsaving*/
/*is function se ham apni marzi ki saving set karenge*/
float setsaving(){
    float save;
    /*user se poochega ke usko kitni amount save karni hai*/
    save = readFloat("Enter the amount you want to save: ");
    /*check kar raha ke saving 0 se ziyada ho negative main na ho*/
    if (save < 0 ){
        printf("Invalid ammount \n");
        return 0;
    }
    else
    /*saving target return karega*/
    return save;
}
/*function get saving*/
/*2 parameters income and total expense*/
/*iise hamen savings milengi*/
float getsaving(float income, float totalexpense){
    /*saving initialize to 0*/
    float saving = 0;
    /*check kar raha agr total expense income se ziyada ho to
    error msg show karega*/
    if(totalexpense > income){
        printf("The total expense is greater than the income \n");
    }
    /*agr nahi to yeh run hoga*/
    else{
    saving = income - totalexpense;
    }
    return saving;
}
/*function addsavings*/
/*3 parameters bankbalance, cash balance and saving */
/*ham is function se savings ko bank yaa cash ke andr add karenge*/
void Addsavings(float *Bankbal,float *cashbal,float savings){
    int opt;
    float amount;
    /*check kar raha ke saving negative na ho*/
    if (savings < 0 ){
        printf("Negative savings entered \n");
    }
    else{
    while(savings > 0){
        /*2 options de raha hai ke saving cash hai bank main*/
        printf("1 savings in bank \n 2 for savings in cash \n");
        opt = readInt("Enter option: ");
        if(opt == 1){
            printf("Enter the saving in bank \n");
            amount = readFloat("Enter amount: ");
            /*amount savings se ziyada to nahi?*/
            if(amount > 0 && amount <= savings){
            *Bankbal = *Bankbal + amount;
            savings = savings - amount;
            }
            else{
                printf("Amount is greater than savings \n");
            }
        }
        else if(opt ==2 ){
            printf("Enter the savings in cash \n");
            amount = readFloat("Enter amount: ");
            /*amount savings se ziyada to nahi?*/
            if(amount > 0 && amount <= savings){
            *cashbal = *cashbal + amount;
            savings = savings - amount;
            }
            /*agr ziyada hui to error msg show hoga*/
            else{
                printf("Amount is greater than savings \n");
            }
        }
        else{
            printf("invalid option \n");
        }
    }
    }

}

/*function warningamount*/
void waringamount(float income){
    /*yahan pr warning amoun calculate hogi jo bhi income ka 80% banegi*/
    float amount;
    amount  = income * 0.8;
    printf("The warning amount is: %f \n",amount);
}
/*menu print  hoga yahan pr */
void monthlysummary(float income, float bankbal,float cashbal, float savings,
                    float totalexpense)
{
    float remaining;
    float totalbalance;

    /*remining amount calculate karne ke liye*/
    remaining = income - totalexpense;
    /*total balance calculate karne ke liye*/
    totalbalance = bankbal + cashbal;

    /*template set kar rahe ham yahan*/
    printf("====== MONTHLY SUMMARY ======\n");

    /*total income aur expense print hoga*/
    printf("Total Income: %.2f\n", income);
    printf("Total Expense: %.2f\n", totalexpense);

    /*bank aur cash balance */
    printf("Bank Balance: %.2f\n", bankbal);
    printf("Cash Balance: %.2f\n", cashbal);

    /*total balance  aur rem income print hoa*/
    printf("Total Balance: %.2f\n", totalbalance);
    printf("Remaining Income: %.2f\n", remaining);

    printf("Saving Target: %.2f\n", savings);

    /*check karne ke liye ke saving achieve hui hai yaa nahi*/
    if (remaining >= savings)
    {
        printf("Saving Target Achieved!\n");
    }
    else
    {
        printf("Saving Target Not Achieved!\n");
    }

    printf("=============================\n");
}
void categoryreport(int size,float expense[][4]){
    /*total initialise ho raha hai*/
    float total =0 ;
    /*yahan yeh user ko bata dega ke konsi value kisko represent karegi*/
    printf("1 denotes food\n 2 denotes transport \n 3 denotes shopping \n 4 denotes bills");
    printf("------Category Report-----\n");
    /*categories ke liye*/
    for(int col = 0;col < 4; col ++){
        /*categories ke expense ke liye*/
        for(int row =0; row < size ; row ++){
            total = total + expense[row][col];
        }
        /*condition check hogi us hisaab se total aur food category print hogi*/
        if(col == 0){
            printf("Food:");
            printf("%.2f \n",total);
        }
        else if(col == 1){
            printf("Transport:");
            printf("%.2f \n",total);
        }
        else if(col == 2){
            printf("Shopping:");
            printf("%.2f \n",total);
        }
        else{
            printf("Bill:");
            printf("%.2f \n",total);
        }
        /*total dubara se initialize hoga*/
        total = 0 ;
    }
    printf("-----End-----");
}


/* Integrated file handling and balance helpers */
void saveExpenses(void) {
    FILE *fp = fopen(expenseFile, "w");
    if (fp == NULL) { printf("Could not save expenses!\n"); return; }
    for (int i = 0; i < expenseCount; i++) {
        struct Expense *e = &expenses[i];
        fprintf(fp, "%.2f\n%s\n%s\n%s\n%s\n%s\n",
                e->amount, e->category, e->date, e->place,
                e->paymentMethod, e->status);
    }
    fclose(fp);
}

void saveSavingTarget(void) {
    FILE *fp = fopen(savingFile, "w");
    if (fp != NULL) { fprintf(fp, "%.2f\n", savingTarget); fclose(fp); }
}

float getTotalExpense(void) {
    float total = 0;
    for (int i = 0; i < expenseCount; i++) {
        if (strcmp(expenses[i].status, "Paid") == 0 ||
            strcmp(expenses[i].status, "paid") == 0)
            total += expenses[i].amount;
    }
    return total;
}

void getBalances(float *bankbal, float *cashbal) {
    *bankbal = 0;
    *cashbal = 0;
    for (int i = 0; i < incomeCount; i++) {
        if (strcmp(incomes[i].paymentMethod, "Cash") == 0)
            *cashbal += incomes[i].amount;
        else
            *bankbal += incomes[i].amount;
    }
    for (int i = 0; i < expenseCount; i++) {
        if (strcmp(expenses[i].status, "Paid") != 0 &&
            strcmp(expenses[i].status, "paid") != 0) continue;
        if (strcmp(expenses[i].paymentMethod, "Cash") == 0 ||
            strcmp(expenses[i].paymentMethod, "cash") == 0)
            *cashbal -= expenses[i].amount;
        else
            *bankbal -= expenses[i].amount;
    }
}

void showAllTransactions(void) {
    viewIncomes();
    viewExpenses();
}

void showCategoryReport(void) {
    float categories[100][4] = {{0}};
    int rows = 0;
    for (int i = 0; i < expenseCount; i++) {
        if (strcmp(expenses[i].status, "Paid") != 0 &&
            strcmp(expenses[i].status, "paid") != 0) continue;
        int col = -1;
        if (strcmp(expenses[i].category, "Food") == 0) col = 0;
        else if (strcmp(expenses[i].category, "Transport") == 0) col = 1;
        else if (strcmp(expenses[i].category, "Shopping") == 0) col = 2;
        else if (strcmp(expenses[i].category, "Bills") == 0) col = 3;
        if (col >= 0) { categories[rows][col] = expenses[i].amount; rows++; }
    }
    categoryreport(rows, categories);
    printf("\nNote: report covers paid Food/Transport/Shopping/Bills expenses.\n");
}

void showMonthlySummary(void) {
    float bankbal, cashbal;
    getBalances(&bankbal, &cashbal);
    monthlysummary(getTotalIncome(), bankbal, cashbal, savingTarget, getTotalExpense());
}

// 1. Add Expense Function
void addExpense() {
    if (expenseCount >= 100) {
        printf("\nError: Expense storage is full!\n");
        return;
    }

    printf("\n--- Add New Expense ---\n");
    printf("Enter amount: ");
    expenses[expenseCount].amount = readFloat("");
    if (expenses[expenseCount].amount <= 0) {
        printf("Invalid amount!\n");
        return;
    }
    
    printf("Enter category (e.g., Food, Bills, Transport): ");
    readText("", expenses[expenseCount].category, sizeof(expenses[expenseCount].category));
    
    printf("Enter date (DD-MM-YYYY): ");
    readText("", expenses[expenseCount].date, sizeof(expenses[expenseCount].date));
    
    printf("Enter place (e.g., Supermarket): ");
    readText("", expenses[expenseCount].place, sizeof(expenses[expenseCount].place));
    
    printf("Enter payment method (Cash/Bank): ");
    readText("", expenses[expenseCount].paymentMethod, sizeof(expenses[expenseCount].paymentMethod));
    
    printf("Enter status (Paid/Unpaid): ");
    readText("", expenses[expenseCount].status, sizeof(expenses[expenseCount].status));

    struct Expense *e = &expenses[expenseCount];
    if (strcmp(e->status, "Paid") == 0 || strcmp(e->status, "paid") == 0) {
        int method;
        float bankbal, cashbal;
        getBalances(&bankbal, &cashbal);
        if (strcmp(e->paymentMethod, "Cash") == 0 || strcmp(e->paymentMethod, "cash") == 0)
            method = 2;
        else if (strcmp(e->paymentMethod, "Bank") == 0 || strcmp(e->paymentMethod, "bank") == 0 ||
                 strcmp(e->paymentMethod, "Bank Transfer") == 0)
            method = 1;
        else {
            printf("Payment method must be Cash or Bank.\n");
            return;
        }
        if (!calculatebalance(e->amount, method, &bankbal, &cashbal)) return;
    }
    expenseCount++;
    saveExpenses();
    checkwarning(getTotalIncome(), getTotalExpense(), savingTarget);
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
    readText("", searchCat, sizeof(searchCat));

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
/*menu print  hoga yahan pr */


/* Integrated main menu: login, income, expenses and savings */
void menu(void) {
    printf("\n========== BUDGET TRACKER MENU ==========\n");
    printf("1. Add Income\n2. View Incomes\n3. Show Total Income\n");
    printf("4. Add Expense\n5. View All Expenses\n6. Search Expenses by Category\n");
    printf("7. All Transactions\n8. Monthly Summary\n9. Category-wise Expense Report\n");
    printf("10. Set Saving Target\n11. Warning Amount\n12. Show User Information\n");
    printf("13. Update User Information\n14. Delete Account\n15. Logout\n");
}

void incomeModule(void) {
    int choice;
    do {
        menu();
        choice = readInt("Enter your choice: ");
        switch (choice) {
            case 1: addIncome(); break;
            case 2: viewIncomes(); break;
            case 3: showTotalIncome(); break;
            case 4: addExpense(); break;
            case 5: viewExpenses(); break;
            case 6: searchByCategory(); break;
            case 7: showAllTransactions(); break;
            case 8: showMonthlySummary(); break;
            case 9: showCategoryReport(); break;
            case 10:
                savingTarget = setsaving();
                saveSavingTarget();
                printf("Saving target saved: %.2f\n", savingTarget);
                break;
            case 11:
                waringamount(getTotalIncome());
                checkwarning(getTotalIncome(), getTotalExpense(), savingTarget);
                break;
            case 12: showUserInformation(); break;
            case 13: setUserInformation(); break;
            case 14:
                deleteAccount();
                if (!usernameExists(username)) choice = 15;
                break;
            case 15: printf("Logged out successfully.\n"); break;
            default: printf("Invalid choice!\n");
        }
    } while (choice != 15);
}

int main(void) {
    int choice;
    do {
        printf("\n===== BUDGET TRACKER =====\n");
        printf("1. Sign Up\n2. Login\n3. Exit\n");
        choice = readInt("Enter your choice: ");
        if (choice == 1) {
            if (signUp()) {
                incomeCount = 0;
                expenseCount = 0;
                savingTarget = 0;
                // Create an empty income file for the new account
                FILE *fp = fopen(incomeFile, "w");
                if (fp != NULL) fclose(fp);
                else printf("Warning: Could not initialize income file.\n");
                fp = fopen(expenseFile, "w");
                if (fp != NULL) fclose(fp);
                else printf("Warning: Could not initialize expense file.\n");
                saveSavingTarget();
                incomeModule();
            }
        } else if (choice == 2) {
            if (login()) incomeModule();
        } else if (choice == 3) {
            printf("Goodbye!\n");
        } else {
            printf("Invalid choice.\n");
        }
    } while (choice != 3);
    return 0;
}
