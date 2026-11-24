#include <stdio.h>
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
int main(){

    float income,cashbal,bankbal,expense,totalexpense = 0,save;
    int options,choice =-1;
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