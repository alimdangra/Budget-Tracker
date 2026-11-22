#include <stdio.h>
#include <string.h>

struct Expense {
    float amount;
    char category[30];
    char date[20];
    char place[50];
    char paymentMethod[20];
    char status[20]; // Paid / Unpaid
};
struct Expense expenses[100];
int expenseCount = 0;
/*------ANAYA--------*/
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
/*------oneeba-------*/
// Global Array and Counter for Expenses


// Function Prototypes for Expense Management
void addExpense();
void viewExpenses();
void searchByCategory();
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
    printf("Enter the amount you want to you want to save \n");
    scanf("%f",&save);
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
        scanf("%d",&opt);
        if(opt == 1){
            printf("Enter the saving in bank \n");
            scanf("%f",&amount);
            /*amount savings se ziyada to nahi?*/
            if(amount <= savings){
            *Bankbal = *Bankbal + amount;
            savings = savings - amount;
            }
            else{
                printf("Amount is greater than savings \n");
            }
        }
        else if(opt ==2 ){
            printf("Enter the savings in cash \n");
            scanf("%f",&amount);
            /*amount savings se ziyada to nahi?*/
            if(amount <= savings){
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
void menu(){
    int choice;
    /*Enter 1. Add Income2. Add Expense 3. View All Transactions 4. Monthly Summary
    5. Category-wise Expense Report 6. Set Spending Limit
    7. Check Budget Warning 8. Exit*/
    printf("----Menu---- \n");
    printf("1-Add income \n");
    printf("2-Expense \n");
    printf("3-All transaction \n");
    printf("4-Monthly summary \n");
    printf("5-Category wised expense report \n");
    printf("6-Set saving \n");
    printf("7-Warning amount \n");
    printf("8-Exit \n");
    
}

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
#define MAX_INCOME 100
   /* we can store a maximum of 100 incomes */

/*Structures*/

struct Income
{
    float amount;
    char source[30];
    char date[20];
    char paymentMethod[20];
};

/* Structure to store the user's information */
struct User
{
    char name[50];
    int age;
    char phone[20];
    char city[30];
};

/*Global variables*/

struct Income incomes[MAX_INCOME]; 
/* all incomes are saved in this array */

int incomeCount = 0;               
 /* how many incomes have been added so far */

struct User user;                 
 /* we have only one user */

int userSet = 0;                    
/* 0 = info not entered yet, 1 = info entered */

/*Helper functions*/


void removeNewline(char text[])
{
    int len = strlen(text);
    if (len > 0 && text[len - 1] == '\n')
    {
        text[len - 1] = '\0';
    }
}

void clearBuffer()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
    {
}
}

/*File handling functions*/

/* Without files, all data is lost when the program closes.
   So we save the data in text files and load it again at the start. */

#define USER_FILE   "user.txt"
#define INCOME_FILE "income.txt"

/* Saves the user's information in user.txt (one item per line) */
void saveUser(){
    FILE *fp = fopen(USER_FILE, "w");  
    if (fp == NULL)
    {
        printf("Error: could not save user information!\n");
        return;
    }

    fprintf(fp, "%s\n%d\n%s\n%s\n", user.name, user.age, user.phone, user.city);
    fclose(fp);
}

/* Saves all incomes in income.txt (one income per line, parts separated by |) */
void saveIncomes()
{
    int n;
    FILE *fp = fopen(INCOME_FILE, "w");
    if (fp == NULL)
    {
        printf("Error: could not save incomes!\n");
        return;
    }

    for (n = 0; n < incomeCount; n++)
    {
        fprintf(fp, "%.2f|%s|%s|%s\n",
                incomes[n].amount,
                incomes[n].source,
                incomes[n].date,
                incomes[n].paymentMethod);
    }
    fclose(fp);
}

/* Loads the saved data when the program starts */
void loadData()
{
    FILE *fp;

    /* ----- load user ----- */
    fp = fopen(USER_FILE, "r");      
    if (fp != NULL)                     /* NULL means the file does not exist yet */
    {
        if (fgets(user.name, sizeof(user.name), fp) != NULL &&
            fscanf(fp, "%d\n", &user.age) == 1 &&
            fgets(user.phone, sizeof(user.phone), fp) != NULL &&
            fgets(user.city, sizeof(user.city), fp) != NULL)
        {
            removeNewline(user.name);
            removeNewline(user.phone);
            removeNewline(user.city);
            userSet = 1;
        }
        fclose(fp);
    }

    /*load incomes*/
    fp = fopen(INCOME_FILE, "r");
    if (fp != NULL)
    {
        while (incomeCount < MAX_INCOME &&
                fscanf(fp, "%f|%29[^|]|%19[^|]|%19[^\n]\n",
                &incomes[incomeCount].amount,
                incomes[incomeCount].source,
                incomes[incomeCount].date,
                incomes[incomeCount].paymentMethod) == 4)
        {
            incomeCount++;
        }
        fclose(fp);
    }
}

/*User Information functions*/

void setUserInformation()
{
    printf("\n--- Enter User Information ---\n");

    printf("Enter name: ");
    fgets(user.name, sizeof(user.name), stdin);
    removeNewline(user.name);


    do
    {
        printf("Enter age: ");
        if (scanf("%d", &user.age) != 1)
        {
            user.age = 0;            /* wrong input (like letters) */
        }
        clearBuffer();

        if (user.age <= 0 || user.age > 120)
        {
            printf("Invalid age! Please enter again.\n");
        }
    } while (user.age <= 0 || user.age > 120);

    printf("Enter phone number: ");
    fgets(user.phone, sizeof(user.phone), stdin);
    removeNewline(user.phone);

    printf("Enter city: ");
    fgets(user.city, sizeof(user.city), stdin);
    removeNewline(user.city);

    userSet = 1;
    saveUser();                      /* save to file */
    printf("User information saved successfully!\n");
}

/* Shows the user's information on the screen */
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

/* Income functions*/

/* Adds a new income */
void addIncome()
{
    /* first check that the array is not full */
    if (incomeCount >= MAX_INCOME)
    {
        printf("\nIncome list is full! Cannot add more income.\n");
        return;
    }

    /* pointer to the current empty slot, so we don't have to write
       incomes[incomeCount] again and again */
    struct Income *i = &incomes[incomeCount];

    printf("\n--- Add Income ---\n");

    /* amount must be positive, otherwise ask again */
    do
    {
        printf("Enter amount: ");
        if (scanf("%f", &i->amount) != 1)
        {
            i->amount = 0;           /* wrong input */
        }
        clearBuffer();

        if (i->amount <= 0)
        {
            printf("Amount must be greater than 0!\n");
        }
    } while (i->amount <= 0);

    printf("Enter source (e.g. Salary, Business, Pocket Money): ");
    fgets(i->source, sizeof(i->source), stdin);
    removeNewline(i->source);

    printf("Enter date (DD-MM-YYYY): ");
    fgets(i->date, sizeof(i->date), stdin);
    removeNewline(i->date);

    /* payment method: only Cash or Bank Transfer is allowed */
    int choice;
    do
    {
        printf("Payment method (1 = Cash, 2 = Bank Transfer): ");
        if (scanf("%d", &choice) != 1)
        {
            choice = 0;
        }
        clearBuffer();

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

    incomeCount++;   /* now we have one more income */
    saveIncomes();   /* save to file */
    printf("Income added successfully!\n");
}

/* Shows all incomes in a table */
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

/* Shows the total income on the screen */
void showTotalIncome()
{
    printf("\nTotal Income = %.2f\n", getTotalIncome());
}
/*main function*/
int main(){
    float income,cashbal,bankbal,expense,totalexpense = 0,save;
    int options,choice =-1;
    int choices;
    

    printf("===== BUDGET TRACKER (Member 1 Test) =====\n");

    loadData();

    if (userSet == 0)
    {
        setUserInformation();
    }
    else
    {
        printf("Welcome back, %s!\n", user.name);
    }

    do
    {
        printf("\n========== MENU ==========\n");
        printf("1. Add Income\n");
        printf("2. View Incomes\n");
        printf("3. Show Total Income\n");
        printf("4. Show User Information\n");
        printf("5. Update User Information\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");

        if (scanf("%d", &choices) != 1)
        {
            choices = 0;
        }
        clearBuffer();

        switch (choices)
        {
            case 1:
                addIncome();
                break;
            case 2:
                viewIncomes();
                break;
            case 3:
                showTotalIncome();
                break;
            case 4:
                showUserInformation();
                break;
            case 5:
                setUserInformation();
                break;
            case 6:
                printf("\nThank you, %s! Goodbye.\n", user.name);
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choices != 6);
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
    save = setsaving();
    printf("Enter your income \n");/*idhr income input hogi */
    scanf("%f",&income);
    /*cash balanace input le raha hai users se*/
    printf("Enter your cash balance \n");
    scanf("%f",&cashbal);
    /*bank balanace input le raha hai users se*/
    printf("enter your bank balance \n");
    scanf("%f",&bankbal);
    
    /*expense input ho raha hai*/
    printf("enter your expenses \n");
    scanf("%f",&expense);
    /*user se pooch raha hai ke kisse usne pyment ki hai*/
    printf("press 1 if you spent from bank and \n 2 if you spent cash \n");
    scanf("%d",&options);
    /*calculate balnce ke function ko call kar raha hai*/
    /*yahan pr condition check ho rahi hai if vlue true hogi 
    to yeh condition apply hogi*/
    if(calculatebalance(expense,options,&bankbal,&cashbal)){
        /*saare expenses ko add kar raha hai */
        totalexpense = totalexpense + expense;
        /*ispe check warning hogi aur 3 parameters apply honge */
        
        checkwarning(income,totalexpense,save);
    }


while (choice!=8)
{
    menu();
    printf("Enter choice \n");
    scanf("%d",&choice);
    
}
}



