/* C regression check for the original budget core & shared removal logic.
 * The test uses temporary data files, not the user's saved accounts. */
#define BUDGETLY_NO_MAIN
#include "../src/Budgetly_Portable.c"
#include <assert.h>
int main(void){
    char tmp[]="/tmp/budgetly-c-test-XXXXXX";
    assert(mkdtemp(tmp)); assert(chdir(tmp)==0);
    strcpy(username,"testuser");makeFileNames();
    strcpy(user.name,"Ali Tester");user.age=19;
    strcpy(user.phone,"03200000000");strcpy(user.city,"Karachi");userSet=1;saveUser();
    incomeCount=2;incomes[0].amount=8000;
    strcpy(incomes[0].source,"Salary");strcpy(incomes[0].date,"15-11-2026");
    strcpy(incomes[0].paymentMethod,"Bank Transfer");
    incomes[1].amount=2000;strcpy(incomes[1].source,"Cash gift");
    strcpy(incomes[1].date,"19-11-2026");strcpy(incomes[1].paymentMethod,"Cash");
    saveIncomes();
    expenseCount=2;expenses[0].amount=1200;
    strcpy(expenses[0].category,"Transport");strcpy(expenses[0].date,"20-11-2026");
    strcpy(expenses[0].place,"Petrol");strcpy(expenses[0].paymentMethod,"Cash");
    strcpy(expenses[0].status,"Paid");
    expenses[1].amount=300;strcpy(expenses[1].category,"Bills");
    strcpy(expenses[1].date,"21-11-2026");strcpy(expenses[1].place,"Bill");
    strcpy(expenses[1].paymentMethod,"Bank");strcpy(expenses[1].status,"Unpaid");
    saveExpenses();savingTarget=2400;saveSavingTarget();
    assert(getTotalIncome()==10000);assert(getTotalExpense()==1200);
    float bank=0,cash=0;getBalances(&bank,&cash);
    assert(bank==8000 && cash==800);
    incomeCount=expenseCount=0;loadData();
    assert(incomeCount==2 && expenseCount==2);
    assert(savingTarget==2400 && !strcmp(user.name,"Ali Tester"));
    assert(validDate("29-02-2024") && !validDate("29-02-2025"));
    assert(deleteEntryFile(1,0));assert(expenseCount==1);
    assert(getTotalExpense()==0);getBalances(&bank,&cash);
    assert(bank==8000 && cash==2000);
    assert(deleteEntryFile(0,1));assert(incomeCount==1);
    incomeCount=expenseCount=0;loadData();
    assert(incomeCount==1 && expenseCount==1);
    assert(getTotalIncome()==8000 && getTotalExpense()==0);
    assert(validUsername("ali_123") && !validUsername("../bad"));
    assert(!parseAge("abc") && parseAge("20")==20);
    puts("PASS: stored data, balances, unpaid entries, dates, deletes and reload");
    return 0;
}
