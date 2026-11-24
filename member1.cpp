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

struct Income incomes[MAX_INCOME];
int incomeCount = 0;
struct User user;
int userSet = 0;

char username[30];
char password[30];
char userFile[50];
char incomeFile[50];

void setUserInformation();
void loadData();

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
    userSet = 0;

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

    remove("accounts.txt");
    rename("temp.txt", "accounts.txt");

    remove(userFile);
    remove(incomeFile);

    printf("Your account and saved income data have been deleted.\n");
}

void menu()
{
    printf("\n========== MENU ==========\n");
    printf("1. Add Income\n");
    printf("2. View Incomes\n");
    printf("3. Show Total Income\n");
    printf("4. Show User Information\n");
    printf("5. Update User Information\n");
    printf("6. Delete Account\n");
    printf("7. Logout\n");
}

void incomeModule()
{
    int choice;

    do
    {
        menu();
        choice = readInt("Enter your choice: ");

        switch (choice)
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
                deleteAccount();
                choice = 7;
                break;

            case 7:
                printf("\nLogged out successfully.\n");
                break;

            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 7);
}

int main()
{
    int choice;

    do
    {
        printf("\n===== BUDGET TRACKER =====\n");
        printf("1. Sign Up\n");
        printf("2. Login\n");
        printf("3. Exit\n");

        choice = readInt("Enter your choice: ");

        if (choice == 1)
        {
            if (signUp())
            {
                incomeCount = 0;
                incomeModule();
            }
        }
        else if (choice == 2)
        {
            if (login())
            {
                incomeModule();
            }
        }
        else if (choice == 3)
        {
            printf("Goodbye!\n");
        }
        else
        {
            printf("Invalid choice.\n");
        }

    } while (choice != 3);

    return 0;
}
