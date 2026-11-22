/*
 * BUDGETLY / Shared mobile + desktop interface
 * Pure C99 + SDL2 + SDL2_ttf. No HTML, CSS, JavaScript or C++ application code.
 * Original first-semester team C functions/comments are included unchanged.
 * Touch, mouse, keyboard, inline validation and custom confirmation cards.
 * Android/iOS require platform packaging tools documented in README.
 */
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if !defined(_WIN32)
#include <strings.h>
#else
#define strcasecmp _stricmp
#endif
#include <math.h>
#include <ctype.h>
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define CHANGE_DIR _chdir
#else
#include <unistd.h>
#define CHANGE_DIR chdir
#endif

/* Original submitted backend is retained, including its existing comments. */
#define main budgetly_original_console_main
#include "BudgetTracker_Core.inc"
#undef main

#define PF_MAX 8
#define MAX_VISIBLE 7
#define COL(R,G,B) ((SDL_Color){R,G,B,255})
#define RGBA(R,G,B,A) ((SDL_Color){R,G,B,A})
#define YES(C1,C2,C3) COL(C1,C2,C3)

enum { LOGIN, SIGNUP, HOME, INCOME, EXPENSES, TRANSACTIONS, REPORTS, TOOLS, PROFILE };
enum { POP_NONE, POP_INFO, POP_ERROR, POP_DEL_INCOME, POP_DEL_EXPENSE,
       POP_DEL_ACCOUNT, POP_WARNING, POP_FORECAST };
enum { F_NONE, F_ADD_INCOME, F_ADD_EXPENSE, F_SAVING, F_LIMIT, F_FORECAST, F_PROFILE };

typedef struct { char text[104]; int maxlen; int invalid; } TextField;
static SDL_Window *window;
static SDL_Renderer *renderer;
static TTF_Font *fontSmall, *fontBody, *fontSemi, *fontTitle, *fontHero;
static int VW=1180, VH=790, mobile=0;
static int active=LOGIN, authed=0, form=F_NONE, popup=POP_NONE;
static int focus=-1, tap=0, tapX=0, tapY=0, scroll=0, navOffset=0;
static int incomePage=0, expensePage=0, categoryChoice=0, methodChoice=0, paidChoice=0;
static int deleteIndex=-1;static int selectedExpense=-1,selectedIncome=-1;
static int warningLevel=0;
static char popupTitle[128],popupBody[640],popupNote[250];
static char validation[240];
static TextField fields[PF_MAX];
static float spendingCap=0;
static char storageLocation[512];
static SDL_Color NAVY={14,34,55,255}, NAVY2={23,57,70,255};
static SDL_Color MINT={18,174,135,255}, GOLD={239,174,76,255};
static SDL_Color CORAL={223,79,94,255}, INK={27,47,64,255};
static SDL_Color MUTED={108,127,139,255}, WHITE={255,255,255,255};
static SDL_Color PALE={245,249,248,255}, LINE={224,235,232,255};

static void setColor(SDL_Color c){SDL_SetRenderDrawBlendMode(renderer,SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer,c.r,c.g,c.b,c.a);}
static void rect(float x,float y,float w,float h,SDL_Color c){
    SDL_Rect r={(int)x,(int)y,(int)w,(int)h};setColor(c);SDL_RenderFillRect(renderer,&r);
}
static void rounded(float x,float y,float w,float h,float rad,SDL_Color c){
    /* Antialiased-looking solid rounded rectangles, rendered without images. */
    int r=(int)rad,yy,xx;
    if(r<1){rect(x,y,w,h,c);return;}
    if(r>w/2)r=(int)(w/2);if(r>h/2)r=(int)(h/2);
    rect(x+r,y,w-2*r,h,c);rect(x,y+r,r,h-2*r,c);rect(x+w-r,y+r,r,h-2*r,c);
    for(yy=0;yy<r;yy++){
        float dx=sqrtf((float)r*r-(float)(r-yy-1)*(r-yy-1));
        xx=(int)ceilf((float)r-dx);
        rect(x+xx,y+yy,w-2*xx,1,c);
        rect(x+xx,y+h-yy-1,w-2*xx,1,c);
    }
}
static void card(float x,float y,float w,float h,int radius){
    rounded(x+1,y+5,w,h,radius,RGBA(20,63,66,23));rounded(x,y,w,h,radius,WHITE);
}
static int hit(float x,float y,float w,float h){return tap&&tapX>=x&&tapX<x+w&&tapY>=y&&tapY<y+h;}
static TTF_Font *pickFont(int size){
    if(size>=37)return fontHero?fontHero:fontBody;
    if(size>=27)return fontTitle?fontTitle:fontBody;
    if(size>=19)return fontSemi?fontSemi:fontBody;
    if(size<=13)return fontSmall?fontSmall:fontBody;
    return fontBody;
}
static void drawText(const char *s,int size,int x,int y,int maxWidth,SDL_Color color){
    if(!s || !*s)return;
    TTF_Font *f=pickFont(size);if(!f)return;
    SDL_Surface *surf=TTF_RenderUTF8_Blended(f,s,color);
    if(!surf)return;
    SDL_Texture *tex=SDL_CreateTextureFromSurface(renderer,surf);
    if(tex){int ww=surf->w,hh=surf->h;
        if(maxWidth>0 && ww>maxWidth){float ratio=(float)maxWidth/(float)ww;ww=maxWidth;hh=(int)(hh*ratio);}
        SDL_Rect d={x,y,ww,hh};SDL_RenderCopy(renderer,tex,NULL,&d);SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}
static void textWrap(const char *s,int size,int x,int y,int width,SDL_Color color){
    TTF_Font *f=pickFont(size);if(!f || !s)return;
    SDL_Surface *surf=TTF_RenderUTF8_Blended_Wrapped(f,s,color,(Uint32)width);
    if(!surf)return;
    SDL_Texture *tex=SDL_CreateTextureFromSurface(renderer,surf);
    if(tex){SDL_Rect d={x,y,surf->w,surf->h};SDL_RenderCopy(renderer,tex,NULL,&d);SDL_DestroyTexture(tex);}
    SDL_FreeSurface(surf);
}
static int button(const char *text,int x,int y,int w,int h,int style){
    SDL_Color bg=style==1?MINT:style==2?CORAL:style==3?NAVY:COL(230,244,239);
    SDL_Color fg=style==0?INK:WHITE;
    rounded(x,y,w,h,12,bg);
    int tw=0,th=0;TTF_Font *f=fontSemi?fontSemi:fontBody;
    if(f)TTF_SizeUTF8(f,text,&tw,&th);
    drawText(text,19,x+(w-tw)/2,y+(h-th)/2,w-14,fg);
    return popup==POP_NONE && hit(x,y,w,h);
}
static void sectionTitle(const char *title,const char *subtitle,int x,int y){
    drawText(title,29,x,y,710,INK);drawText(subtitle,14,x,y+40,740,MUTED);
}
static void setValidation(const char *s,int idx){
    snprintf(validation,sizeof validation,"%s",s);
    if(idx>=0 && idx<PF_MAX)fields[idx].invalid=1;
}
static void clearValidation(void){validation[0]=0;for(int i=0;i<PF_MAX;i++)fields[i].invalid=0;}
static void setForm(int kind){form=kind;focus=-1;scroll=0;clearValidation();
    memset(fields,0,sizeof fields);for(int i=0;i<PF_MAX;i++)fields[i].maxlen=90;
    if(kind==F_ADD_INCOME || kind==F_ADD_EXPENSE){
        time_t now=time(NULL);struct tm *tmv=localtime(&now);
        if(tmv)snprintf(fields[2].text,sizeof fields[2].text,"%02d-%02d-%04d",tmv->tm_mday,tmv->tm_mon+1,tmv->tm_year+1900);
    }
    if(kind==F_SAVING)snprintf(fields[0].text,sizeof fields[0].text,"%.2f",(double)savingTarget);
    if(kind==F_LIMIT)snprintf(fields[0].text,sizeof fields[0].text,"%.2f",(double)spendingCap);
    if(kind==F_FORECAST)for(int i=0;i<3;i++)strcpy(fields[i].text,"0");
    if(kind==F_PROFILE){
        snprintf(fields[0].text,sizeof fields[0].text,"%s",user.name);
        snprintf(fields[1].text,sizeof fields[1].text,"%d",user.age);
        snprintf(fields[2].text,sizeof fields[2].text,"%s",user.phone);
        snprintf(fields[3].text,sizeof fields[3].text,"%s",user.city);
    }
    categoryChoice=methodChoice=paidChoice=0;
}
static void toScreen(int page){active=page;form=F_NONE;focus=-1;scroll=0;
    clearValidation();memset(fields,0,sizeof fields);
    for(int i=0;i<PF_MAX;i++)fields[i].maxlen=90;
    SDL_StopTextInput();}
static void message(int type,const char *title,const char *body,const char *note){
    popup=type;snprintf(popupTitle,sizeof popupTitle,"%s",title?title:"");
    snprintf(popupBody,sizeof popupBody,"%s",body?body:"");
    snprintf(popupNote,sizeof popupNote,"%s",note?note:"");
    SDL_StopTextInput();focus=-1;
}
static void loadLimit(void){char fn[80];snprintf(fn,sizeof fn,"%s_limit.txt",username);
    spendingCap=0;FILE *f=fopen(fn,"r");if(f){float v;
    if(fscanf(f,"%f",&v)==1 && isfinite(v)&&v>=0)spendingCap=v;fclose(f);}}
static void saveLimit(void){char fn[80];snprintf(fn,sizeof fn,"%s_limit.txt",username);
    FILE *f=fopen(fn,"w");if(f){fprintf(f,"%.2f\n",(double)spendingCap);fclose(f);}}
static void resetAccountState(void){incomeCount=expenseCount=userSet=0;
    savingTarget=spendingCap=0;authed=0;username[0]=password[0]=0;
    toScreen(LOGIN);}
static int validUsername(const char *s){int n=(int)strlen(s);if(n<3||n>26)return 0;
    for(int i=0;i<n;i++)if(!isalnum((unsigned char)s[i])&&s[i]!='_')return 0;return 1;}
static int validDate(const char *s){int d,m,y,days;if(strlen(s)!=10||s[2]!='-'||s[5]!='-'||
    sscanf(s,"%2d-%2d-%4d",&d,&m,&y)!=3||m<1||m>12||y<2000||y>2100)return 0;
    days=m==2?((y%400==0||(y%4==0&&y%100!=0))?29:28):
        ((m==4||m==6||m==9||m==11)?30:31);return d>=1&&d<=days;}
static int parseMoney(const char *s,float *v,int zero){char *end;
    double val=strtod(s,&end);while(isspace((unsigned char)*end))end++;
    if(!*s||*end||!isfinite(val)||val>1000000000.0||(zero?val<0:val<=0))return 0;
    *v=(float)val;return 1;}
static int parseAge(const char *s){char *end;long v=strtol(s,&end,10);
    return *s&&!*end&&v>=1&&v<=120?(int)v:0;}
static int validPhone(const char *s){int digits=0;
    for(int i=0;s[i];i++){if(isdigit((unsigned char)s[i]))digits++;
       else if(s[i]!='+'&&s[i]!='-'&&s[i]!=' ')return 0;}
    return digits>=8&&digits<=15;}
static int fileReplace(const char *temporary,const char *target){
#ifdef _WIN32
    return MoveFileExA(temporary,target,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)!=0;
#else
    return rename(temporary,target)==0;
#endif
}
static int deleteEntryFile(int kind,int index){
    const char *target=kind==0?incomeFile:expenseFile;char temporary[160];
    snprintf(temporary,sizeof temporary,"%s.tmp",target);FILE *f=fopen(temporary,"w");if(!f)return 0;
    int fail=0;
    if(kind==0)for(int i=0;i<incomeCount;i++){if(i==index)continue;
        struct Income *v=&incomes[i];if(fprintf(f,"%.2f\n%s\n%s\n%s\n",v->amount,
            v->source,v->date,v->paymentMethod)<0)fail=1;}
    else for(int i=0;i<expenseCount;i++){if(i==index)continue;
        struct Expense *v=&expenses[i];if(fprintf(f,"%.2f\n%s\n%s\n%s\n%s\n%s\n",v->amount,
            v->category,v->date,v->place,v->paymentMethod,v->status)<0)fail=1;}
    if(fclose(f)!=0)fail=1;
    if(fail||!fileReplace(temporary,target)){remove(temporary);return 0;}
    if(kind==0){for(int i=index;i<incomeCount-1;i++)incomes[i]=incomes[i+1];incomeCount--;}
    else{for(int i=index;i<expenseCount-1;i++)expenses[i]=expenses[i+1];expenseCount--;}
    return 1;
}
static int deleteAccountFiles(void){
    FILE *src=fopen("accounts.txt","r");if(!src)return 0;
    FILE *dst=fopen("accounts.tmp","w");if(!dst){fclose(src);return 0;}
    char n[64],p[64];int failed=0;
    while(fgets(n,sizeof n,src)){if(!fgets(p,sizeof p,src)){failed=1;break;}
        removeNewline(n);removeNewline(p);
        if(strcmp(username,n)!=0 && fprintf(dst,"%s\n%s\n",n,p)<0)failed=1;
    }
    if(ferror(src))failed=1;if(fclose(dst))failed=1;fclose(src);
    if(failed||!fileReplace("accounts.tmp","accounts.txt")){remove("accounts.tmp");return 0;}
    remove(userFile);remove(incomeFile);remove(expenseFile);remove(savingFile);
    char fn[80];snprintf(fn,sizeof fn,"%s_limit.txt",username);remove(fn);
    return 1;
}
static void finishPopup(int confirm){int type=popup,idx=deleteIndex;
    popup=POP_NONE;deleteIndex=-1;
    if(!confirm)return;
    if(type==POP_DEL_INCOME||type==POP_DEL_EXPENSE){
        int kind=type==POP_DEL_INCOME?0:1;
        if(deleteEntryFile(kind,idx)){
            if(kind==0 && incomePage>0)incomePage=0;
            if(kind==1 && expensePage>0)expensePage=0;
            if(kind==0){
                float bank,cash;getBalances(&bank,&cash);
                double inc=getTotalIncome(),exp=getTotalExpense();
                if(bank<0||cash<0||(inc>0&&exp/inc>=0.80)){
                    char risk[300];snprintf(risk,sizeof risk,
                        "After deleting income, bank balance is PKR %.2f, cash is PKR %.2f and spending is %.1f%% of income.",
                        (double)bank,(double)cash,inc>0?100*exp/inc:0.0);
                    warningLevel=(bank<0||cash<0||inc>0&&exp/inc>=.90)?3:
                        inc>0&&exp/inc>=.85?2:1;
                    message(POP_WARNING,"Check your budget",risk,
                        "An income removal can make an existing expense exceed available funds.");
                    return;
                }
            }
            message(POP_INFO,"Record removed","Your entry was permanently deleted. Reports and balances have been updated.","Your saved account file was updated.");
        }else message(POP_ERROR,"Deletion failed","Could not update your saved data. No entries were removed.","Check storage permissions and try again.");
    }else if(type==POP_DEL_ACCOUNT){
        if(deleteAccountFiles()){resetAccountState();message(POP_INFO,"Account deleted","Your local account and financial history were erased.","You can create a new account from the welcome screen.");}
        else message(POP_ERROR,"Account not deleted","Could not safely update your account database.","Your user files were not removed.");
    }
}
static void validateAuth(void){clearValidation();
    if(!validUsername(fields[0].text)){setValidation("Username must have 3-26 letters, digits or underscores.",0);return;}
    if(!fields[1].text[0]){setValidation("Password is required.",1);return;}
    if(active==LOGIN){
        FILE *f=fopen("accounts.txt","r");char a[50],p[50];int correct=0,exists=0;
        if(f){while(fgets(a,sizeof a,f)){if(!fgets(p,sizeof p,f))break;removeNewline(a);removeNewline(p);
            if(!strcmp(a,fields[0].text)){exists=1;if(!strcmp(p,fields[1].text))correct=1;break;}}
            fclose(f);}
        if(!correct){if(!exists)setValidation("Account not found. Use Create account, or check the username.",0);
            else setValidation("Incorrect password. Please try again.",1);return;}
        snprintf(username,sizeof username,"%s",fields[0].text);makeFileNames();loadData();loadLimit();
        authed=1;toScreen(HOME);return;
    }
    if(strlen(fields[1].text)<4){setValidation("Create a password with at least four characters.",1);return;}
    if(strcmp(fields[1].text,fields[2].text)){setValidation("Passwords do not match.",2);return;}
    int age=parseAge(fields[4].text);
    if(!fields[3].text[0]){setValidation("Full name is required.",3);return;}
    if(!age){setValidation("Age must be between 1 and 120.",4);return;}
    if(!validPhone(fields[5].text)){setValidation("Enter a valid phone number (8-15 digits).",5);return;}
    if(!fields[6].text[0]){setValidation("City is required.",6);return;}
    if(usernameExists(fields[0].text)){setValidation("Username already exists. Please sign in or choose another.",0);return;}
    FILE *f=fopen("accounts.txt","a");if(!f){setValidation("Could not create account. Check storage access.",-1);return;}
    int writeOk=fprintf(f,"%s\n%s\n",fields[0].text,fields[1].text)>=0;
    if(fclose(f)==EOF)writeOk=0;
    if(!writeOk){
        setValidation("Could not save the account.",-1);return;}
    snprintf(username,sizeof username,"%s",fields[0].text);
    makeFileNames();incomeCount=expenseCount=0;userSet=1;savingTarget=0;
    snprintf(user.name,sizeof user.name,"%s",fields[3].text);user.age=age;
    snprintf(user.phone,sizeof user.phone,"%s",fields[5].text);
    snprintf(user.city,sizeof user.city,"%s",fields[6].text);
    saveUser();saveIncomes();saveExpenses();saveSavingTarget();loadLimit();
    authed=1;toScreen(HOME);message(POP_INFO,"Welcome to Budgetly!",
        "Your account is ready. Start by adding income, then track expenses and savings.","Your data is stored locally on this device.");
}
static void submitForm(void){clearValidation();float amount=0;
    if(form==F_ADD_INCOME){
        if(incomeCount>=100){setValidation("Income history is full.",-1);return;}
        if(!parseMoney(fields[0].text,&amount,0)){setValidation("Enter a positive income amount.",0);return;}
        if(!fields[1].text[0]){setValidation("Enter an income source.",1);return;}
        if(!validDate(fields[2].text)){setValidation("Use a valid DD-MM-YYYY date.",2);return;}
        struct Income *v=&incomes[incomeCount++];v->amount=amount;
        snprintf(v->source,sizeof v->source,"%s",fields[1].text);
        snprintf(v->date,sizeof v->date,"%s",fields[2].text);
        snprintf(v->paymentMethod,sizeof v->paymentMethod,"%s",methodChoice?"Bank Transfer":"Cash");
        saveIncomes();form=F_NONE;SDL_StopTextInput();return;
    }
    if(form==F_ADD_EXPENSE){
        if(expenseCount>=100){setValidation("Expense history is full.",-1);return;}
        if(!parseMoney(fields[0].text,&amount,0)){setValidation("Enter a positive expense amount.",0);return;}
        if(!validDate(fields[2].text)){setValidation("Use a valid DD-MM-YYYY date.",2);return;}
        if(!fields[1].text[0]){setValidation("Enter a place or merchant.",1);return;}
        if(!paidChoice){float bank,cash;getBalances(&bank,&cash);
            if((methodChoice?bank:cash)+0.001f<amount){
                setValidation(methodChoice?"Insufficient bank balance. Add income or choose Unpaid.":
                    "Insufficient cash balance. Add income or choose Unpaid.",0);return;}}
        const char *cats[]={"Food","Transport","Shopping","Bills","Other"};
        struct Expense *v=&expenses[expenseCount++];v->amount=amount;
        snprintf(v->category,sizeof v->category,"%s",cats[categoryChoice]);
        snprintf(v->date,sizeof v->date,"%s",fields[2].text);
        snprintf(v->place,sizeof v->place,"%s",fields[1].text);
        snprintf(v->paymentMethod,sizeof v->paymentMethod,"%s",methodChoice?"Bank":"Cash");
        snprintf(v->status,sizeof v->status,"%s",paidChoice?"Unpaid":"Paid");
        saveExpenses();form=F_NONE;SDL_StopTextInput();
        float inc=getTotalIncome(),spent=getTotalExpense();
        if(!paidChoice && inc>0){int tier=spent>=inc*.90?90:spent>=inc*.85?85:spent>=inc*.80?80:0;
            if(tier || (spendingCap>0&&spent>spendingCap) ||
               (savingTarget>0&&spent>inc-savingTarget)){
                char txt[320];snprintf(txt,sizeof txt,
                    "You've used %.1f%% of your income. Paid expenses: PKR %.2f, total income: PKR %.2f.",
                    100.0*spent/inc,(double)spent,(double)inc);
                warningLevel=tier>=90?3:tier>=85?2:1;
                message(POP_WARNING,tier>=90?"Critical budget warning":tier>=85?"High spending warning":
                    tier>=80?"80% budget alert":"Savings limit alert",txt,
                    "Review your spending before making another purchase.");
            }
        }
        return;
    }
    if(form==F_SAVING){if(!parseMoney(fields[0].text,&amount,1)){setValidation("Enter a valid saving goal.",0);return;}
        savingTarget=amount;saveSavingTarget();form=F_NONE;return;}
    if(form==F_LIMIT){if(!parseMoney(fields[0].text,&amount,1)){setValidation("Enter a valid spending cap.",0);return;}
        spendingCap=amount;saveLimit();form=F_NONE;return;}
    if(form==F_PROFILE){int age=parseAge(fields[1].text);
        if(!fields[0].text[0]){setValidation("Full name is required.",0);return;}
        if(!age){setValidation("Enter a valid age.",1);return;}
        if(!validPhone(fields[2].text)){setValidation("Phone number is invalid.",2);return;}
        if(!fields[3].text[0]){setValidation("City is required.",3);return;}
        snprintf(user.name,sizeof user.name,"%s",fields[0].text);user.age=age;
        snprintf(user.phone,sizeof user.phone,"%s",fields[2].text);
        snprintf(user.city,sizeof user.city,"%s",fields[3].text);saveUser();form=F_NONE;return;}
    if(form==F_FORECAST){float a,b,v;
        if(!parseMoney(fields[0].text,&a,1)||!parseMoney(fields[1].text,&b,1)||
           !parseMoney(fields[2].text,&v,1)){setValidation("Forecast amounts must be zero or more.",0);return;}
        double in=getTotalIncome()+a,spent=getTotalExpense()+b+v;
        char txt[630];snprintf(txt,sizeof txt,
           "Projected income: PKR %.2f\nProjected spending: PKR %.2f\nVehicle / transport: PKR %.2f\nRemaining: PKR %.2f\nSaving target: PKR %.2f\nBudget used: %.1f%%",
           in,spent,(double)v,in-spent,(double)savingTarget,in>0?100.0*spent/in:0.0);
        form=F_NONE;message(POP_FORECAST,"Your budget forecast",txt,
            "What-if model only; no transactions were changed.");return;
    }
}

static int contentLeft(void){return mobile?18:270;}
static int contentWidth(void){return mobile?VW-36:VW-300;}
static int navBottom(void){return mobile?VH-82:VH;}
static int fieldWidget(int idx,const char *title,int x,int y,int w,int password){
    int h=43;drawText(title,13,x,y-21,w,MUTED);
    rounded(x,y,w,h,10,fields[idx].invalid?COL(255,226,228):focus==idx?COL(223,247,239):WHITE);
    if(fields[idx].invalid)rounded(x,y,4,h,2,CORAL);
    char show[110]={0};
    if(password){int n=(int)strlen(fields[idx].text);if(n>100)n=100;
        memset(show,'*',(size_t)n);show[n]=0;}
    else snprintf(show,sizeof show,"%s",fields[idx].text);
    drawText(show[0]?show:"Tap to enter",16,x+13,y+11,w-28,
        show[0]?INK:COL(157,176,181));
    if(popup==POP_NONE && hit(x,y,w,h)){
        focus=idx;SDL_StartTextInput();SDL_Rect r={x,y,w,h};SDL_SetTextInputRect(&r);
        fields[idx].invalid=0;validation[0]=0;
    }
    return y+h;
}
static void errorRibbon(int x,int y,int w){
    if(!validation[0])return;
    rounded(x,y,w,52,11,COL(255,228,232));rounded(x+10,y+10,30,30,9,CORAL);
    drawText("!",21,x+19,y+11,25,WHITE);
    textWrap(validation,14,x+49,y+7,w-57,COL(161,45,57));
}
static void segmented(const char *label, const char **options,int total,int *chosen,
                      int x,int y,int w){
    drawText(label,13,x,y-23,w,MUTED);
    int each=(w-(total-1)*8)/total;
    for(int i=0;i<total;i++){
        int rx=x+i*(each+8);SDL_Color color=i==*chosen?MINT:COL(228,241,238);
        rounded(rx,y,each,40,10,color);
        int tw=0,th=0;if(fontSmall)TTF_SizeUTF8(fontSmall,options[i],&tw,&th);
        drawText(options[i],13,rx+(each-tw)/2,y+12,each-8,i==*chosen?WHITE:INK);
        if(popup==POP_NONE && hit(rx,y,each,40)){
            *chosen=i;validation[0]=0;
        }
    }
}
static void moneyText(float value,char *s,int n){snprintf(s,(size_t)n,"PKR %.2f",(double)value);}
static void kpi(const char *title, double amount,int x,int y,int w,int h,SDL_Color accent){
    card(x,y,w,h,16);rounded(x+17,y+20,9,9,5,accent);
    drawText(title,14,x+32,y+16,w-38,MUTED);
    char text[80];snprintf(text,sizeof text,"PKR %.2f",amount);
    drawText(text,26,x+17,y+49,w-28,INK);
}
static void bars(int x,int y,int w,int h,int withLabels){
    const char *names[]={"Food","Transport","Shopping","Bills","Other"};
    SDL_Color palette[]={MINT,GOLD,CORAL,COL(104,147,226),COL(150,174,183)};
    float values[5]={0},maxv=1;
    for(int i=0;i<expenseCount;i++)if(!strcasecmp(expenses[i].status,"Paid")){
        int j=4;for(int k=0;k<4;k++)if(!strcasecmp(expenses[i].category,names[k]))j=k;
        values[j]+=expenses[i].amount;
    }
    for(int j=0;j<5;j++)if(values[j]>maxv)maxv=values[j];
    for(int j=0;j<5;j++){
        int yy=y+j*(h/5);drawText(names[j],13,x,yy,w/4,INK);
        int barX=x+(w>500?128:92),barW=w-(barX-x)-16;
        rounded(barX,yy+4,barW,12,5,COL(228,239,237));
        if(values[j]>0)rounded(barX,yy+4,barW*values[j]/maxv,12,5,palette[j]);
        if(withLabels){char text[64];snprintf(text,sizeof text,"PKR %.0f",(double)values[j]);
            drawText(text,13,barX,yy+20,barW,MUTED);}
    }
}
static void spendingMeter(int x,int y,int w){
    double income=getTotalIncome(),spent=getTotalExpense();
    double ratio=income>0?spent/income:0;
    SDL_Color color=ratio>=0.90?CORAL:ratio>=0.85?COL(233,124,62):ratio>=0.80?GOLD:MINT;
    char s[70];snprintf(s,sizeof s,"%.1f%% of income spent",100*ratio);
    drawText(s,20,x,y,w,INK);
    rounded(x,y+40,w,18,9,COL(220,235,232));
    rounded(x,y+40,(float)(w*fmin(1.0,fmax(0.0,ratio))),18,9,color);
    drawText("80%",12,x+(int)(w*.80)-14,y+60,45,MUTED);
    drawText("85%",12,x+(int)(w*.85)-14,y+60,45,MUTED);
    drawText("90%",12,x+(int)(w*.90)-14,y+60,45,MUTED);
}
static int recordRow(int kind,int index,int x,int y,int w){
    int rowH=mobile?87:74;
    rounded(x,y,w,rowH,14,WHITE);
    char cash[65],heading[155],infoLine[180];float amount;
    if(kind==0){struct Income *v=&incomes[index];amount=v->amount;
        snprintf(heading,sizeof heading,"%s",v->source);
        snprintf(infoLine,sizeof infoLine,"%s  -  %s",v->date,v->paymentMethod);
    }else{struct Expense *v=&expenses[index];amount=v->amount;
        snprintf(heading,sizeof heading,"%s  /  %s",v->category,v->place);
        snprintf(infoLine,sizeof infoLine,"%s  -  %s  -  %s",v->date,v->paymentMethod,v->status);
    }
    moneyText(amount,cash,sizeof cash);
    drawText(heading,18,x+14,y+8,w-115,INK);
    drawText(infoLine,13,x+14,y+33,w-120,MUTED);
    if(mobile){drawText(cash,16,x+14,y+57,w-125,kind==0?MINT:CORAL);}
    else{drawText(cash,19,x+w-340,y+17,204,kind==0?MINT:CORAL);}
    rounded(x+w-94,y+17,81,40,10,COL(254,234,235));
    drawText("Delete",14,x+w-78,y+29,65,CORAL);
    if(popup==POP_NONE && hit(x+w-94,y+17,81,40)){
        deleteIndex=index;
        char body[400];snprintf(body,sizeof body,
            "Remove this %s permanently?\n\n%s\n%s\n\nAll balances and reports will be recalculated.",
            kind==0?"income":"expense",heading,cash);
        message(kind==0?POP_DEL_INCOME:POP_DEL_EXPENSE,"Delete transaction?",body,
            "This action cannot be undone. Press Keep it to cancel.");
    }
    return rowH;
}
static void drawHeader(void){
    rect(0,0,VW,84,NAVY);
    rounded(mobile?20:30,18,45,45,13,MINT);
    drawText("B",29,mobile?32:43,21,28,WHITE);
    drawText("BUDGETLY",24,mobile?76:92,24,240,WHITE);
    if(!mobile)drawText("YOUR PERSONAL FINANCE SPACE",12,VW-284,33,280,COL(173,206,206));
    if(mobile && authed){drawText("LOCAL FINANCE",12,VW-129,35,130,COL(180,225,212));}
}
static void navBar(void){
    if(!authed || form!=F_NONE)return;
    const char *names[]={"Dashboard","Income","Expenses","Transactions","Reports","Budget tools","Profile"};
    const int screens[]={HOME,INCOME,EXPENSES,TRANSACTIONS,REPORTS,TOOLS,PROFILE};
    if(mobile){
        int x=0,y=VH-81;rect(0,y,VW,81,WHITE);
        const char *shortNames[]={"Home","Income","Expense","History","Reports","Tools","Profile"};
        int cell=VW/7;
        for(int i=0;i<7;i++){
            int bx=x+cell*i;
            if(active==screens[i])rounded(bx+5,y+8,cell-10,52,12,COL(220,249,237));
            char c[2]={shortNames[i][0],0};
            drawText(c,21,bx+cell/2-7,y+11,30,active==screens[i]?MINT:MUTED);
            drawText(shortNames[i],12,bx+5,y+39,cell-7,active==screens[i]?INK:MUTED);
            if(popup==POP_NONE && hit(bx,y,cell,81))toScreen(screens[i]);
        }
    } else {
        rounded(13,105,227,VH-121,22,NAVY2);
        for(int i=0;i<7;i++){
            int yy=151+i*65;
            if(active==screens[i])rounded(27,yy,200,49,13,COL(30,104,103));
            drawText(names[i],19,46,yy+14,169,active==screens[i]?WHITE:COL(191,223,215));
            if(popup==POP_NONE && hit(27,yy,200,49))toScreen(screens[i]);
        }
        if(button("Sign out",33,VH-92,187,44,0))resetAccountState();
    }
}
static void drawLogin(void){
    int x=mobile?18:690,w=mobile?VW-36:432;
    int top=mobile?137:127;
    if(!mobile){
        rounded(34,138,615,520,22,NAVY2);
        rounded(69,172,197,39,17,COL(34,108,100));
        drawText("TRACK  |  PLAN  |  GROW",13,89,185,182,WHITE);
        drawText("Your money,",39,74,265,522,WHITE);
        drawText("beautifully",39,74,324,522,WHITE);
        drawText("organized.",39,74,383,522,COL(80,236,185));
        textWrap("Income. Expenses. Savings. Financial freedom in one focused workspace.",
            19,74,473,500,COL(210,237,231));
        card(91,547,502,82,15);
        drawText("NO SPREADSHEETS TO JUGGLE",14,113,565,470,MUTED);
        drawText("Just your money, clearly tracked.",19,113,588,470,INK);
    }
    card(x,top,w,mobile?562:544,23);
    drawText(active==LOGIN?"Welcome back":"Create your account",28,x+24,top+26,w-35,INK);
    drawText(active==LOGIN?"Sign in to continue":"Set up your private finance space",14,x+24,top+72,w-35,MUTED);
    int cy=top+139;
    fieldWidget(0,"USERNAME",x+24,cy,w-48,0);
    fieldWidget(1,"PASSWORD",x+24,cy+85,w-48,1);
    if(validation[0])errorRibbon(x+24,cy+157,w-48);
    if(button("Sign in",x+24,top+376,w-48,51,1))validateAuth();
    if(button("Create a new account",x+24,top+443,w-48,48,0)){toScreen(SIGNUP);}
}
static void drawSignup(void){
    int x=mobile?18:690,w=mobile?VW-36:432;
    int top=mobile?105:94;
    if(!mobile){
        rounded(34,145,615,498,22,NAVY2);
        drawText("A better relationship",38,68,259,570,WHITE);
        drawText("with your money.",38,68,322,570,COL(80,236,185));
        textWrap("Your daily income, expense history, smart warnings and reports stay organized in one place.",
            19,68,406,520,COL(210,237,231));
    }
    card(x,top,w,mobile?VH-130:638,23);
    drawText("Create account",27,x+23,top+17,w-40,INK);
    int fw=w-46,fx=x+23;
    const char *labels[]={"USERNAME","PASSWORD","CONFIRM PASSWORD","FULL NAME","AGE","PHONE","CITY"};
    int ys[7]={top+101,top+172,top+243,top+314,top+385,top+456,top+527};
    if(mobile){for(int i=0;i<7;i++)ys[i]-=scroll;}
    for(int i=0;i<7;i++){
        if(ys[i]<76 || ys[i]>VH-85)continue;
        fieldWidget(i,labels[i],fx,ys[i],fw,i==1||i==2);
    }
    int by=top+586-(mobile?scroll:0);
    if(validation[0])errorRibbon(fx,top+54,fw);
    if(by>85 && by<VH-45 && button("Create account",fx,by,fw,46,1))validateAuth();
    int back=by+54;
    if(back>85&&back<VH-35 && button("Back to sign in",fx,back,fw,41,0))toScreen(LOGIN);
}
static void drawDashboard(void){
    int x=contentLeft(),w=contentWidth(),y=119;
    sectionTitle("Your dashboard","Track your money at a glance.",x,y);
    float bank=0,cash=0;getBalances(&bank,&cash);
    double in=getTotalIncome(),ex=getTotalExpense(),bal=bank+cash;
    if(mobile){
        int half=(w-12)/2;
        kpi("Total income",in,x,183,half,112,MINT);
        kpi("Paid expenses",ex,x+half+12,183,half,112,CORAL);
        kpi("Bank balance",bank,x,306,half,112,COL(94,142,225));
        kpi("Cash balance",cash,x+half+12,306,half,112,GOLD);
        card(x,432,w,163,16);drawText("Spending overview",20,x+17,449,w-25,INK);
        spendingMeter(x+18,494,w-36);
        card(x,604,w,108,16);drawText("Available balance",15,x+17,615,w-35,MUTED);
        char v[80];snprintf(v,sizeof v,"PKR %.2f",bal);
        drawText(v,28,x+17,647,w-32,MINT);
        if(button("+ Add income",x+18,728,(w-44)/2,44,1)){setForm(F_ADD_INCOME);}
        if(button("+ Add expense",x+26+(w-44)/2,728,(w-44)/2,44,3))setForm(F_ADD_EXPENSE);
    }else{
        int quarter=(w-42)/4;
        kpi("Total income",in,x,183,quarter,120,MINT);
        kpi("Paid expenses",ex,x+quarter+14,183,quarter,120,CORAL);
        kpi("Bank balance",bank,x+2*(quarter+14),183,quarter,120,COL(94,142,225));
        kpi("Cash balance",cash,x+3*(quarter+14),183,quarter,120,GOLD);
        card(x,325,w*0.59,337,17);drawText("Expense breakdown",23,x+24,348,w/2,INK);
        bars(x+23,408,(int)(w*.53),205,1);
        int xx=x+(int)(w*.61), ww=w-(int)(w*.61);
        card(xx,325,ww,337,17);drawText("Budget health",23,xx+17,348,ww-24,INK);
        spendingMeter(xx+18,406,ww-36);
        char balance[80];snprintf(balance,sizeof balance,"PKR %.2f",bal);
        drawText("Remaining balance",15,xx+17,517,ww-29,MUTED);
        drawText(balance,29,xx+17,552,ww-27,MINT);
        if(button("+ Add income",x,686,202,47,1))setForm(F_ADD_INCOME);
        if(button("+ Add expense",x+215,686,202,47,3))setForm(F_ADD_EXPENSE);
    }
}
static void drawEntries(int kind){
    int x=contentLeft(),w=contentWidth(),count=kind==0?incomeCount:expenseCount;
    int *page=kind==0?&incomePage:&expensePage;
    sectionTitle(kind==0?"Income history":"Expense history",
        kind==0?"Every amount you receive, in one place.":"Review spending and remove incorrect entries.",x,112);
    if(button(kind==0?"+ Add income":"+ Add expense",x,mobile?183:183,
        mobile?w:201,45,1))setForm(kind==0?F_ADD_INCOME:F_ADD_EXPENSE);
    int per=mobile?5:7,maxpage=count?((count-1)/per):0;
    if(*page>maxpage)*page=maxpage;
    int start=count-1-(*page)*per;int rows=0,top=mobile?250:250;
    for(int i=start;i>=0 && rows<per;i--,rows++){
        int yy=top+rows*(mobile?96:82);recordRow(kind,i,x,yy,w);
    }
    if(!count){card(x,top,w,154,18);drawText("No entries yet",23,x+20,top+22,w-40,INK);
        drawText("Tap Add to create your first record.",14,x+20,top+68,w-35,MUTED);}
    int yy=mobile?VH-141:VW>1000?700:710;
    if(*page>0&&button("Previous",x,yy,123,40,0))(*page)--;
    if(*page<maxpage&&button("Next page",x+w-138,yy,138,40,3))(*page)++;
    char pages[60];snprintf(pages,sizeof pages,"Page %d of %d",*page+1,maxpage+1);
    drawText(pages,13,x+(w-100)/2,yy+12,101,MUTED);
}
static void drawTransactions(void){
    int x=contentLeft(),w=contentWidth();sectionTitle("All transactions",
        "Income and expense records across the account.",x,116);
    int total=incomeCount+expenseCount,per=mobile?5:7;
    if(total==0){card(x,198,w,146,17);drawText("No transactions yet",22,x+18,230,w-28,INK);return;}
    int shown=0,from=total-1-navOffset;
    for(int k=from;k>=0&&shown<per;k--){
        int kind,index;
        if(k<incomeCount){kind=0;index=k;}else{kind=1;index=k-incomeCount;}
        recordRow(kind,index,x,200+shown*(mobile?97:78),w);shown++;
    }
    if(navOffset>0&&button("Earlier",x,VH-(mobile?139:80),115,42,0)){
        navOffset-=per;if(navOffset<0)navOffset=0;}
    if(navOffset+per<total&&button("More",x+w-114,VH-(mobile?139:80),113,42,3))navOffset+=per;
}
static void drawReports(void){
    int x=contentLeft(),w=contentWidth();sectionTitle("Financial reports",
        "Where does your money go?",x,115);
    double spent=getTotalExpense(),inc=getTotalIncome();
    int y=mobile?181:190;
    card(x,y,w,mobile?292:319,18);
    drawText("Category-wise expenses",23,x+19,y+23,w-40,INK);
    bars(x+20,y+89,w-42,mobile?186:191,1);
    int y2=y+(mobile?309:337);card(x,y2,w,218,18);
    drawText("Monthly performance",21,x+20,y2+20,w-40,INK);
    time_t now=time(NULL);struct tm *t=localtime(&now);
    int month=t?t->tm_mon+1:11,year=t?t->tm_year+1900:2026;
    double mIn=0,mEx=0;
    for(int i=0;i<incomeCount;i++){int d,m,yr;
        if(sscanf(incomes[i].date,"%d-%d-%d",&d,&m,&yr)==3&&m==month&&yr==year)mIn+=incomes[i].amount;}
    for(int i=0;i<expenseCount;i++){int d,m,yr;
        if(sscanf(expenses[i].date,"%d-%d-%d",&d,&m,&yr)==3&&m==month&&yr==year&&
           !strcasecmp(expenses[i].status,"Paid"))mEx+=expenses[i].amount;}
    char a[120];snprintf(a,sizeof a,"%02d / %d   Income: PKR %.2f",month,year,mIn);
    drawText(a,16,x+21,y2+62,w-38,INK);
    snprintf(a,sizeof a,"Paid expenses: PKR %.2f",mEx);drawText(a,16,x+21,y2+96,w-40,CORAL);
    snprintf(a,sizeof a,"Remaining: PKR %.2f",mIn-mEx);drawText(a,20,x+21,y2+138,w-36,MINT);
    if(!mobile){char full[160];snprintf(full,sizeof full,"Lifetime income: PKR %.2f   |   Paid expenses: PKR %.2f",inc,spent);
        drawText(full,15,x+21,y2+180,w-40,MUTED);}
}
static void drawTools(void){
    int x=contentLeft(),w=contentWidth();sectionTitle("Budget tools & models",
        "Savings, spending warnings and future plans.",x,112);
    int y=mobile?183:181;
    card(x,y,w,196,18);drawText("Live spending warnings",22,x+20,y+18,w-30,INK);
    spendingMeter(x+20,y+67,w-40);
    int threshold=y+148;
    drawText("Amber at 80%  -  Orange at 85%  -  Red at 90%",13,x+20,threshold,w-33,MUTED);
    int y2=y+211;card(x,y2,w,219,18);
    char buffer[150];snprintf(buffer,sizeof buffer,"Saving target: PKR %.2f",(double)savingTarget);
    drawText(buffer,20,x+18,y2+19,w-30,INK);
    snprintf(buffer,sizeof buffer,"Custom spending cap: PKR %.2f",(double)spendingCap);
    drawText(buffer,16,x+18,y2+64,w-36,MUTED);
    if(button("Change savings goal",x+18,y2+110,mobile?w-36:223,43,1))setForm(F_SAVING);
    if(button("Set spending limit",x+18,y2+164,mobile?w-36:223,42,3))setForm(F_LIMIT);
    if(!mobile){
        drawText("Model your future budget",20,x+310,y2+41,w-334,INK);
        if(button("What-if + vehicle costs",x+310,y2+112,w-333,47,0))setForm(F_FORECAST);
    }else{
        if(button("Budget modelling + vehicle costs",x,y2+240,w,48,0))setForm(F_FORECAST);
    }
}
static void drawProfile(void){
    int x=contentLeft(),w=contentWidth();sectionTitle("My account","Keep your personal details up to date.",x,116);
    card(x,190,w,mobile?460:433,22);
    rounded(x+23,215,67,67,20,MINT);drawText("B",39,x+44,225,40,WHITE);
    drawText(user.name,25,x+105,220,w-127,INK);
    char a[170];snprintf(a,sizeof a,"@%s",username);drawText(a,15,x+105,260,w-125,MUTED);
    snprintf(a,sizeof a,"Age: %d",user.age);drawText(a,18,x+25,310,w-44,INK);
    snprintf(a,sizeof a,"Phone: %s",user.phone);drawText(a,18,x+25,353,w-44,INK);
    snprintf(a,sizeof a,"City: %s",user.city);drawText(a,18,x+25,395,w-44,INK);
    if(button("Edit profile",x+23,mobile?457:468,mobile?w-46:250,47,1))setForm(F_PROFILE);
    if(button("Delete account",x+23,mobile?519:531,mobile?w-46:250,47,2))
        message(POP_DEL_ACCOUNT,"Delete account?",
            "Remove this account, its income, expenses, saved limits and profile?",
            "This action is permanent and cannot be undone.");
    if(mobile&&button("Sign out",x+23,578,w-46,44,0))resetAccountState();
}
static void drawForm(void){
    int x=mobile?16:353,w=mobile?VW-32:545;
    int y=mobile?105:113;
    card(x,y,w,mobile?VH-133:600,21);
    const char *heading=form==F_ADD_INCOME?"Add income":form==F_ADD_EXPENSE?"Add expense":
        form==F_SAVING?"Savings goal":form==F_LIMIT?"Spending limit":
        form==F_FORECAST?"What-if + vehicle":"Update profile";
    drawText(heading,28,x+20,y+19,w-40,INK);
    if(button("Back",x+w-101,y+17,80,36,0)){form=F_NONE;SDL_StopTextInput();return;}
    int fw=w-42,fx=x+21,top=y+110;
    if(form==F_ADD_INCOME){
        fieldWidget(0,"AMOUNT (PKR)",fx,top,fw,0);
        fieldWidget(1,"INCOME SOURCE",fx,top+84,fw,0);
        fieldWidget(2,"DATE (DD-MM-YYYY)",fx,top+168,fw,0);
        const char *o[]={"Cash","Bank transfer"};
        segmented("RECEIVED THROUGH",o,2,&methodChoice,fx,top+266,fw);
        if(validation[0])errorRibbon(fx,top+337,fw);
        if(button("Save income",fx,top+418,fw,48,1))submitForm();
    }else if(form==F_ADD_EXPENSE){
        fieldWidget(0,"AMOUNT (PKR)",fx,top,fw,0);
        fieldWidget(1,"PLACE OR MERCHANT",fx,top+79,fw,0);
        fieldWidget(2,"DATE (DD-MM-YYYY)",fx,top+158,fw,0);
        const char *categories[]={"Food","Transport","Shopping","Bills","Other"};
        segmented("CATEGORY",categories,5,&categoryChoice,fx,top+252,fw);
        const char *payments[]={"Cash","Bank"};
        segmented("PAYMENT",payments,2,&methodChoice,fx,top+336,fw);
        const char *status[]={"Paid","Unpaid"};
        segmented("STATUS",status,2,&paidChoice,fx,top+421,fw);
        if(validation[0])errorRibbon(fx,top+477,fw);
        if(button("Save expense",fx,top+535,fw,47,1))submitForm();
    }else if(form==F_SAVING||form==F_LIMIT){
        fieldWidget(0,form==F_SAVING?"TARGET AMOUNT (PKR)":"MAX SPENDING (PKR)",fx,top,fw,0);
        if(validation[0])errorRibbon(fx,top+79,fw);
        drawText(form==F_SAVING?"Plan how much to save after expenses.":
            "Use zero for the standard 80% warning point.",14,fx,top+122,fw,MUTED);
        if(button("Save changes",fx,top+202,fw,48,1))submitForm();
    }else if(form==F_FORECAST){
        fieldWidget(0,"EXPECTED EXTRA INCOME (PKR)",fx,top,fw,0);
        fieldWidget(1,"PLANNED EXTRA EXPENSES (PKR)",fx,top+88,fw,0);
        fieldWidget(2,"VEHICLE / TRANSPORT COST (PKR)",fx,top+176,fw,0);
        drawText("Simulation only; saved records will not change.",13,fx,top+253,fw,MUTED);
        if(validation[0])errorRibbon(fx,top+303,fw);
        if(button("Calculate forecast",fx,top+380,fw,49,1))submitForm();
    }else if(form==F_PROFILE){
        fieldWidget(0,"FULL NAME",fx,top,fw,0);
        fieldWidget(1,"AGE",fx,top+85,fw,0);
        fieldWidget(2,"PHONE",fx,top+170,fw,0);
        fieldWidget(3,"CITY",fx,top+255,fw,0);
        if(validation[0])errorRibbon(fx,top+328,fw);
        if(button("Save profile",fx,top+404,fw,46,1))submitForm();
    }
}
static void drawPopup(void){
    if(popup==POP_NONE)return;
    rect(0,0,VW,VH,RGBA(6,27,41,198));
    int x=mobile?16:(VW-520)/2,w=mobile?VW-32:520,h=mobile?457:456;
    int y=(VH-h)/2;
    rounded(x+3,y+7,w,h,24,RGBA(14,50,57,200));
    rounded(x,y,w,h,24,WHITE);
    SDL_Color accent=(popup==POP_ERROR||popup==POP_DEL_ACCOUNT||popup==POP_DEL_INCOME||
        popup==POP_DEL_EXPENSE)?CORAL:popup==POP_WARNING?
        (warningLevel>=3?CORAL:warningLevel==2?COL(233,124,62):GOLD):MINT;
    rounded(x,y,w,7,4,accent);
    rounded(x+20,y+27,59,59,17,popup==POP_ERROR||popup>=POP_DEL_INCOME&&popup<=POP_DEL_ACCOUNT?
        COL(255,232,236):COL(222,251,240));
    drawText(popup==POP_WARNING?"!":popup==POP_INFO||popup==POP_FORECAST?"OK":"!",26,
        x+33,y+44,43,accent);
    drawText(popupTitle,25,x+94,y+31,w-110,INK);
    textWrap(popupBody,18,x+22,y+113,w-44,INK);
    rounded(x+20,y+h-146,w-40,54,13,PALE);
    textWrap(popupNote,13,x+31,y+h-136,w-64,MUTED);
    int destructive=(popup==POP_DEL_INCOME||popup==POP_DEL_EXPENSE||popup==POP_DEL_ACCOUNT);
    if(destructive){
        if(hit(x+20,y+h-76,(w-49)/2,53))finishPopup(0);
        rounded(x+20,y+h-76,(w-49)/2,53,12,COL(228,242,239));
        drawText("Keep it",19,x+49,y+h-59,155,INK);
        rounded(x+29+(w-49)/2,y+h-76,(w-49)/2,53,12,CORAL);
        drawText("Yes, delete",19,x+42+(w-49)/2,y+h-59,155,WHITE);
        if(hit(x+29+(w-49)/2,y+h-76,(w-49)/2,53))finishPopup(1);
    }else{
        rounded(x+24,y+h-76,w-48,53,12,MINT);
        drawText(popup==POP_WARNING?"Review and continue":"Got it",19,
            x+50,y+h-59,w-80,WHITE);
        if(hit(x+24,y+h-76,w-48,53)){
            int warning=popup==POP_WARNING;popup=POP_NONE;
            if(warning)toScreen(TOOLS);
        }
    }
}
static void render(void){
    setColor(PALE);SDL_RenderClear(renderer);
    drawHeader();
    if(form!=F_NONE){drawForm();}
    else if(active==LOGIN)drawLogin();
    else if(active==SIGNUP)drawSignup();
    else{
        navBar();
        /* A screen tap that changes pages is applied on the next frame. */
        if(form==F_NONE){
            if(active==HOME)drawDashboard();
            else if(active==INCOME)drawEntries(0);
            else if(active==EXPENSES)drawEntries(1);
            else if(active==TRANSACTIONS)drawTransactions();
            else if(active==REPORTS)drawReports();
            else if(active==TOOLS)drawTools();
            else if(active==PROFILE)drawProfile();
        }
    }
    drawPopup();SDL_RenderPresent(renderer);
}
static void openFonts(void){
    const char *paths[]={
#ifdef _WIN32
        "C:/Windows/Fonts/segoeui.ttf","C:/Windows/Fonts/arial.ttf",
#elif defined(__ANDROID__)
        "/system/fonts/Roboto-Regular.ttf","/system/fonts/NotoSans-Regular.ttf",
#elif defined(__APPLE__)
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/Core/Helvetica.ttc",
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Arial.ttf",
#endif
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
        "assets/UIFont.ttf",NULL
    };
    for(int i=0;paths[i];i++){
        fontBody=TTF_OpenFont(paths[i],17);
        if(fontBody){
            fontSmall=TTF_OpenFont(paths[i],13);
            fontSemi=TTF_OpenFont(paths[i],20);
            fontTitle=TTF_OpenFont(paths[i],29);
            fontHero=TTF_OpenFont(paths[i],43);
            break;
        }
    }
}
static void setLogicalSize(void){
    int ww=0,hh=0;SDL_GetWindowSize(window,&ww,&hh);
#if defined(__ANDROID__) || defined(__IPHONEOS__)
    mobile=1;
#else
    mobile=(ww>0&&hh>0&&ww<hh);
#endif
    VW=mobile?420:1180;
    VH=mobile?((hh>0&&ww>0)?(int)fmax(900.0,420.0*hh/ww):900):790;
    SDL_RenderSetLogicalSize(renderer,VW,VH);
}
static void processEvent(SDL_Event *event,int *running){
    if(event->type==SDL_QUIT){*running=0;return;}
    if(event->type==SDL_WINDOWEVENT &&
       (event->window.event==SDL_WINDOWEVENT_SIZE_CHANGED||
        event->window.event==SDL_WINDOWEVENT_RESIZED)){setLogicalSize();return;}
    if(event->type==SDL_MOUSEBUTTONUP && event->button.button==SDL_BUTTON_LEFT){
        tap=1;tapX=event->button.x;tapY=event->button.y;return;
    }
    if(event->type==SDL_FINGERMOTION && mobile && active==SIGNUP && popup==POP_NONE){
        scroll-=(int)(event->tfinger.dy*VH);if(scroll<0)scroll=0;if(scroll>190)scroll=190;
        return;
    }
    if(event->type==SDL_MOUSEWHEEL && mobile && active==SIGNUP){
        scroll-=event->wheel.y*35;if(scroll<0)scroll=0;if(scroll>190)scroll=190;return;
    }
    if(event->type==SDL_TEXTINPUT && focus>=0 && popup==POP_NONE){
        TextField *f=&fields[focus];int n=(int)strlen(f->text);
        int count=(int)strlen(event->text.text);
        int limit=f->maxlen?f->maxlen:90;
        if(n+count<limit && n+count<(int)sizeof f->text){
            strcat(f->text,event->text.text);f->invalid=0;validation[0]=0;}
        return;
    }
    if(event->type!=SDL_KEYDOWN)return;
    SDL_Keycode code=event->key.keysym.sym;
    if(popup!=POP_NONE){
        if(code==SDLK_ESCAPE)finishPopup(0);
        if(code==SDLK_RETURN||code==SDLK_KP_ENTER){
            /* Destructive actions require an explicit button click. */
            if(popup!=POP_DEL_INCOME&&popup!=POP_DEL_EXPENSE&&popup!=POP_DEL_ACCOUNT)
                finishPopup(0);
        }
        return;
    }
    if(code==SDLK_BACKSPACE && focus>=0){
        int n=(int)strlen(fields[focus].text);if(n)fields[focus].text[n-1]=0;
        validation[0]=0;return;
    }
    if(code==SDLK_TAB && focus>=0){focus=(focus+1)%PF_MAX;return;}
    if(code==SDLK_RETURN||code==SDLK_KP_ENTER){
        if(active==LOGIN||active==SIGNUP)validateAuth();
        else if(form!=F_NONE)submitForm();
        return;
    }
    if(code==SDLK_ESCAPE){
        if(form!=F_NONE){form=F_NONE;focus=-1;SDL_StopTextInput();}
        else if(active==SIGNUP)toScreen(LOGIN);
        else if(authed)toScreen(HOME);
        return;
    }
    if(code==SDLK_F11){
        Uint32 flags=SDL_GetWindowFlags(window);
        SDL_SetWindowFullscreen(window,flags&SDL_WINDOW_FULLSCREEN_DESKTOP?0:SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
}
static void setupStorage(void){
    char *pref=SDL_GetPrefPath("Budgetly","BudgetTracker");
    if(pref){snprintf(storageLocation,sizeof storageLocation,"%s",pref);SDL_free(pref);
        if(CHANGE_DIR(storageLocation)!=0)fprintf(stderr,"Could not access account data folder.\n");}
}
static int budgetlyStart(int argc,char **argv){
    (void)argc;(void)argv;
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)!=0){fprintf(stderr,"SDL: %s\n",SDL_GetError());return 1;}
    if(TTF_Init()!=0){fprintf(stderr,"TTF: %s\n",TTF_GetError());SDL_Quit();return 1;}
    setupStorage();
    window=SDL_CreateWindow("Budgetly - C Finance Manager",SDL_WINDOWPOS_CENTERED,
       SDL_WINDOWPOS_CENTERED,1230,840,SDL_WINDOW_SHOWN|SDL_WINDOW_RESIZABLE);
    if(!window){fprintf(stderr,"Window: %s\n",SDL_GetError());TTF_Quit();SDL_Quit();return 1;}
    renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC);
    if(!renderer)renderer=SDL_CreateRenderer(window,-1,SDL_RENDERER_SOFTWARE);
    if(!renderer){fprintf(stderr,"Renderer: %s\n",SDL_GetError());SDL_DestroyWindow(window);TTF_Quit();SDL_Quit();return 1;}
    setLogicalSize();openFonts();
    if(!fontBody){fprintf(stderr,"No system TTF font is available. Supply a font via assets/UIFont.ttf.\n");
        SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);TTF_Quit();SDL_Quit();return 1;}
    toScreen(LOGIN);
    FILE *f=fopen("accounts.txt","r");if(!f)toScreen(SIGNUP);else fclose(f);
    int running=1;
    while(running){
        SDL_Event event;
        while(SDL_PollEvent(&event))processEvent(&event,&running);
        if(!running)break;
        render();tap=0;
        SDL_Delay(14);
    }
    if(fontSmall)TTF_CloseFont(fontSmall);if(fontBody)TTF_CloseFont(fontBody);
    if(fontSemi)TTF_CloseFont(fontSemi);if(fontTitle)TTF_CloseFont(fontTitle);
    if(fontHero)TTF_CloseFont(fontHero);
    SDL_DestroyRenderer(renderer);SDL_DestroyWindow(window);TTF_Quit();SDL_Quit();return 0;
}
#ifndef BUDGETLY_NO_MAIN
#if defined(__ANDROID__)
int SDL_main(int argc,char **argv){return budgetlyStart(argc,argv);}
#elif defined(__IPHONEOS__)
int SDL_main(int argc,char **argv){return budgetlyStart(argc,argv);}
int main(int argc,char **argv){return SDL_UIKitRunApp(argc,argv,SDL_main);}
#else
int main(int argc,char **argv){SDL_SetMainReady();return budgetlyStart(argc,argv);}
#endif
#endif /* BUDGETLY_NO_MAIN */
