/*
    BUDGET TRACKER | Windows Desktop Application | Pure C / Win32 API + GDI
    Visual interface: signup, signin, dashboard, transactions, reports, profile.
    The team's original C code and original comments remain unchanged in
    BudgetTracker_Core.inc, which this file includes directly.
    Build: gcc -std=c11 -O2 BudgetTracker_Win32.c -o BudgetTracker.exe -mwindows -lgdi32 -luser32 -lm
*/
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

/* Existing program: its console main is renamed, while its original comments,
   structs, calculations and file-handling functions are preserved. */
#define main original_console_main
#include "BudgetTracker_Core.inc"
#undef main

#define APP_W 1240
#define APP_H 780
#define C(r,g,b) RGB((r),(g),(b))
#define IDC_AUTH_PRIMARY 101
#define IDC_AUTH_SWITCH 102
#define IDC_AUTH_EXIT 103
#define IDC_AUTH_BACK 104
#define IDC_FORM_SAVE 201
#define IDC_FORM_CANCEL 202
#define MAX_CHILDREN 24

/* Application pages */
enum { AUTH_LOGIN, AUTH_SIGNUP, PAGE_HOME, PAGE_INCOME, PAGE_EXPENSE,
       PAGE_TRANSACTIONS, PAGE_REPORTS, PAGE_TOOLS, PAGE_PROFILE };
enum { FORM_INCOME = 1, FORM_EXPENSE, FORM_SAVING, FORM_PROFILE,
       FORM_LIMIT, FORM_FORECAST };

static HWND g_main = NULL, g_modal = NULL;
static HWND g_controls[MAX_CHILDREN];
static RECT g_designRects[MAX_CHILDREN];
static int g_controlCount = 0, g_page = AUTH_LOGIN, g_authenticated = 0;
/* All dashboard coordinates are logical design units, scaled to fit each
   Windows display/window while preserving the original proportions. */
static float g_scale = 1.0f;
static int g_viewX = 0, g_viewY = 0, g_viewW = APP_W, g_viewH = APP_H;
static HFONT g_fieldFont = NULL, g_buttonFont = NULL;
static int g_fontPixelSize = 0;
static int g_fullscreen = 0;
static WINDOWPLACEMENT g_previousPlacement = { sizeof(WINDOWPLACEMENT) };
static LONG_PTR g_previousStyle = 0;
static int g_incomeOffset = 0, g_expenseOffset = 0, g_transactionOffset = 0;
static int g_transactionFilter = 0, g_reportMonthOffset = 0, g_navHover = -1;
/* Rows are selectable. Delete actions update the account's existing text files. */
static int g_selectedIncome = -1, g_selectedExpense = -1;
/* Graphical finance warnings are painted by GDI, without web libraries. */
static int g_alertVisible = 0, g_alertLevel = 0;
static double g_alertRatio = 0;
static char g_alertTitle[96], g_alertBody[240], g_alertHint[180];

static float g_spendingLimit = 0; /* 0 means the original 80% warning threshold. */
static HWND g_search = NULL;
static HFONT g_fontRegular, g_fontMedium, g_fontBold, g_fontTitle, g_fontHero, g_fontSmall;
static HBRUSH g_whiteBrush, g_backgroundBrush, g_authErrorBrush;
/* Error text and field outlines are drawn directly on the signup/login card. */
static char g_authErrors[7][40];
static char g_authMessage[192];
static HWND g_authUser, g_authPass, g_authConfirm, g_authName, g_authAge,
            g_authPhone, g_authCity;
/* Custom full-interface dialog: no standard Windows Yes/No text boxes.
   Type 1 success, 2 validation/error, 3 delete income, 4 delete expense,
   5 delete account, 6 what-if budget model. */
static int g_popupType=0, g_popupIndex=-1;
static char g_popupTitle[120],g_popupBody[660],g_popupDetail[220];
static void executeDeleteTransaction(int kind,int index);
static void executeDeleteAccount(void);
static HWND g_formFields[8];
static int g_formType = 0;
static char g_formError[224]; /* Red inline validation, drawn on income/expense forms. */
/* Budgetly financial theme: ink blue + emerald + warm gold + coral accents. */
static COLORREF NAVY = C(13, 33, 53), MINT = C(13, 170, 138),
                MUTED = C(100, 115, 135), INK = C(25, 46, 65),
                SURFACE = C(244, 249, 249), LINE = C(220, 235, 232),
                GOLD = C(240, 173, 68), CORAL = C(229, 104, 110),
                ERROR_RED = C(190, 52, 65);

static LRESULT CALLBACK mainProcedure(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK formProcedure(HWND, UINT, WPARAM, LPARAM);
static void setPage(int page);
static void openForm(int type);
static void repaintMain(void);
static void updateViewport(void);
static void layoutControls(void);
static void toggleFullscreen(void);
static void authSubmit(void);
static void submitForm(void);
static void loadBudgetLimit(void);
static void saveBudgetLimit(void);
static void showBudgetAlert(int level, const char *title,
                            const char *body, const char *hint, double ratio);
static void paintBudgetAlert(HDC dc);
static void deleteTransaction(int kind, int index);
static void paintPopup(HDC dc);
static void openPopup(int type, const char *title, const char *message, const char *detail);
static void popupDecision(int accepted);


static HFONT font(int size, int bold) {
    return CreateFontA(-size, 0, 0, 0, bold ? FW_SEMIBOLD : FW_NORMAL,
        FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
        CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, "Segoe UI");
}
static void info(const char *msg) {
    openPopup(1,"All done",msg,"Your changes are saved in your local account.");
}
static void errorBox(const char *msg) {
    if(g_modal){
        snprintf(g_formError,sizeof g_formError,"%s",msg);
        InvalidateRect(g_modal,NULL,TRUE);
        MessageBeep(MB_ICONWARNING);
    } else openPopup(2,"Please check",msg,"Correct the information and try again.");
}
static void getText(HWND h, char *out, int cap) {
    if (!h) { out[0] = 0; return; }
    GetWindowTextA(h, out, cap);
}
static void trim(char *s) {
    size_t n = strlen(s), a = 0;
    while (a < n && isspace((unsigned char)s[a])) a++;
    if (a) memmove(s, s + a, n - a + 1);
    n = strlen(s);
    while (n && isspace((unsigned char)s[n - 1])) s[--n] = 0;
}
static void copySafe(char *out, size_t size, const char *src) {
    if (!size) return;
    snprintf(out, size, "%s", src);
}
static int amountFrom(HWND h, float *out, int canBeZero) {
    char text[64], *end;
    double value;
    getText(h, text, sizeof text);
    value = strtod(text, &end);
    while (isspace((unsigned char)*end)) end++;
    if (!*text || *end || !isfinite(value) || value > 1000000000.0 ||
       (canBeZero ? value < 0 : value <= 0)) return 0;
    *out = (float)value;
    return 1;
}
/* Budget configuration is stored per user, beside the existing income files. */
static void budgetLimitFilename(char out[80]) {
    snprintf(out,80,"%s_limit.txt",username);
}
static void loadBudgetLimit(void) {
    char filename[80]; budgetLimitFilename(filename);
    g_spendingLimit=0;
    FILE *fp=fopen(filename,"r");
    if (fp) {
        float value=0;
        if (fscanf(fp,"%f",&value)==1 && isfinite(value) && value>=0 && value<1e9f)
            g_spendingLimit=value;
        fclose(fp);
    }
}
static void saveBudgetLimit(void) {
    char filename[80]; budgetLimitFilename(filename);
    FILE *fp=fopen(filename,"w");
    if (fp) {fprintf(fp,"%.2f\n",g_spendingLimit);fclose(fp);}
}
static int isPaid(int i);
static void reportMonth(int *month, int *year) {
    SYSTEMTIME st;GetLocalTime(&st);
    int m=(int)st.wMonth - g_reportMonthOffset;
    int y=(int)st.wYear;
    while(m<=0){m+=12;y--;}
    while(m>12){m-=12;y++;}
    *month=m;*year=y;
}
static int inMonth(const char *date,int month,int year) {
    int d,m,y;
    return sscanf(date,"%2d-%2d-%4d",&d,&m,&y)==3 && m==month && y==year;
}
static const char *monthTitle(int month) {
    static const char *names[]={"January","February","March","April","May","June",
       "July","August","September","October","November","December"};
    return month>=1 && month<=12 ? names[month-1] : "Unknown";
}
static void monthTotals(double *income,double *spent,double categories[5]) {
    int month,year;reportMonth(&month,&year);
    *income=*spent=0;
    for(int j=0;j<5;j++)categories[j]=0;
    for(int i=0;i<incomeCount;i++)
        if(inMonth(incomes[i].date,month,year))*income+=incomes[i].amount;
    for(int i=0;i<expenseCount;i++) {
        if(!isPaid(i) || !inMonth(expenses[i].date,month,year))continue;
        *spent+=expenses[i].amount;
        /* 0-3: Food/Transport/Shopping/Bills, 4: Other */
        int idx=4;
        if(!_stricmp(expenses[i].category,"Food"))idx=0;
        else if(!_stricmp(expenses[i].category,"Transport"))idx=1;
        else if(!_stricmp(expenses[i].category,"Shopping"))idx=2;
        else if(!_stricmp(expenses[i].category,"Bills"))idx=3;
        categories[idx]+=expenses[i].amount;
    }
}
static void ensureDate(char out[20]) {
    SYSTEMTIME t;
    GetLocalTime(&t);
    snprintf(out, 20, "%02d-%02d-%04d", (int)t.wDay,
         (int)t.wMonth, (int)t.wYear);
}
static int validDate(const char *date) {
    int d,m,y,days;
    if (strlen(date)!=10 || date[2]!='-' || date[5]!='-' ||
        sscanf(date,"%2d-%2d-%4d",&d,&m,&y)!=3 || m<1 || m>12 ||
        y<2000 || y>2100) return 0;
    days=(m==4 || m==6 || m==9 || m==11) ? 30 : 31;
    if (m==2) days=(y%400==0 || (y%4==0 && y%100!=0)) ? 29 : 28;
    return d>=1 && d<=days;
}
static void money(char *buf, size_t n, double v) {
    snprintf(buf, n, "PKR %s%.2f", v < 0 ? "-" : "", fabs(v));
}
static int isPaid(int i) {
    return _stricmp(expenses[i].status, "Paid") == 0;
}
static int isCash(const char *value) {
    return _stricmp(value, "Cash") == 0;
}
static void rounded(HDC dc, int x, int y, int w, int h, int radius,
                    COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ oldBrush = SelectObject(dc, brush), oldPen = SelectObject(dc, pen);
    RoundRect(dc, x, y, x+w, y+h, radius, radius);
    SelectObject(dc, oldBrush); SelectObject(dc, oldPen);
    DeleteObject(brush); DeleteObject(pen);
}
static void filled(HDC dc, int x, int y, int w, int h, COLORREF color) {
    RECT r = {x,y,x+w,y+h}; HBRUSH b = CreateSolidBrush(color);
    FillRect(dc, &r, b); DeleteObject(b);
}
static void line(HDC dc, int x1, int y1, int x2, int y2, COLORREF color) {
    HPEN pen = CreatePen(PS_SOLID, 1, color);
    HGDIOBJ old = SelectObject(dc, pen);
    MoveToEx(dc, x1, y1, NULL); LineTo(dc, x2, y2);
    SelectObject(dc, old); DeleteObject(pen);
}
static void label(HDC dc, const char *s, int x, int y, int w, int h,
                  HFONT fontUsed, COLORREF color, UINT flags) {
    RECT r={x,y,x+w,y+h}; HGDIOBJ old=SelectObject(dc, fontUsed);
    SetBkMode(dc, TRANSPARENT); SetTextColor(dc,color);
    DrawTextA(dc,s,-1,&r,flags|DT_END_ELLIPSIS);
    SelectObject(dc,old);
}
static void shadowCard(HDC dc, int x, int y, int w, int h, int radius) {
    rounded(dc,x+2,y+5,w,h,radius,C(230,234,241));
    rounded(dc,x,y,w,h,radius,C(255,255,255));
}
static void openPopup(int type,const char *title,const char *message,const char *detail){
    g_popupType=type;
    snprintf(g_popupTitle,sizeof g_popupTitle,"%s",title?title:"");
    snprintf(g_popupBody,sizeof g_popupBody,"%s",message?message:"");
    snprintf(g_popupDetail,sizeof g_popupDetail,"%s",detail?detail:"");
    for(int k=0;k<g_controlCount;k++)if(g_controls[k])ShowWindow(g_controls[k],SW_HIDE);
    if(g_main) {SetForegroundWindow(g_main);InvalidateRect(g_main,NULL,FALSE);}
}
static void paintPopup(HDC dc) {
    if(!g_popupType)return;
    /* Full-window dimming and a custom rounded finance-themed confirmation. */
    /* One draw operation keeps the GDI confirmation smooth on older laptops. */
    filled(dc,0,0,APP_W,APP_H,C(23,48,61));
    int left=362,top=144,w=526,h=480;
    rounded(dc,left+7,top+11,w,h,28,C(26,68,74));
    rounded(dc,left,top,w,h,28,C(255,255,255));
    COLORREF accent=(g_popupType==2 || g_popupType>=3 && g_popupType<=5)?CORAL:MINT;
    rounded(dc,left,top,w,8,8,accent);
    rounded(dc,left+34,top+30,63,63,19,g_popupType==2 || g_popupType>=3 && g_popupType<=5?C(255,232,234):C(219,249,240));
    label(dc,(g_popupType==2 || (g_popupType>=3 && g_popupType<=5))?"!":"OK",
          left+36,top+34,59,54,g_fontTitle,accent,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,g_popupTitle,left+114,top+32,w-149,54,g_fontTitle,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,g_popupBody,left+39,top+118,w-78,202,g_fontRegular,INK,DT_LEFT|DT_TOP|DT_WORDBREAK);
    rounded(dc,left+37,top+336,w-74,53,13,C(244,249,248));
    label(dc,g_popupDetail,left+52,top+339,w-105,46,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_WORDBREAK);
    if(g_popupType>=3 && g_popupType<=5){
        rounded(dc,left+37,top+405,208,49,13,C(235,245,242));
        rounded(dc,left+261,top+405,228,49,13,C(203,54,71));
        label(dc,"Keep it",left+39,top+407,204,44,g_fontMedium,INK,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        label(dc,"Yes, delete",left+263,top+407,224,44,g_fontMedium,C(255,255,255),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    } else {
        rounded(dc,left+264,top+405,225,49,13,MINT);
        label(dc,"Got it",left+265,top+407,223,44,g_fontMedium,C(255,255,255),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }
}
static void popupDecision(int accepted){
    int previous=g_popupType,index=g_popupIndex;
    g_popupType=0;g_popupIndex=-1;
    for(int k=0;k<g_controlCount;k++)if(g_controls[k])ShowWindow(g_controls[k],SW_SHOW);
    repaintMain();
    if(!accepted)return;
    if(previous==3 || previous==4)executeDeleteTransaction(previous-3,index);
    else if(previous==5)executeDeleteAccount();
}
static void buttonPaint(DRAWITEMSTRUCT *d) {
    int kind = (int)GetWindowLongPtrA(d->hwndItem,GWLP_USERDATA);
    char text[128]; COLORREF bg,fg;
    getText(d->hwndItem,text,sizeof text);
    bg = kind == 1 ? MINT : (kind == 2 ? NAVY : C(230,243,240));
    fg = kind == 1 || kind == 2 ? C(255,255,255) : INK;
    if (d->itemState & ODS_SELECTED)
        bg = kind == 1 ? C(8,119,99) : (kind == 2 ? C(21,53,74) : C(207,230,225));
    rounded(d->hDC, d->rcItem.left+1,d->rcItem.top+1,
      d->rcItem.right-d->rcItem.left-2,d->rcItem.bottom-d->rcItem.top-2,16,bg);
    label(d->hDC,text,d->rcItem.left+4,d->rcItem.top+1,
          d->rcItem.right-d->rcItem.left-8,
          d->rcItem.bottom-d->rcItem.top-2,g_buttonFont ? g_buttonFont : g_fontBold,fg,
          DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    if (d->itemState & ODS_FOCUS) {
        RECT r=d->rcItem; InflateRect(&r,-5,-5); DrawFocusRect(d->hDC,&r);
    }
}
static HWND createBtn(HWND p, const char *text, int x,int y,int w,int h,
                      int id,int kind) {
    HWND control=CreateWindowExA(0,"BUTTON",text, WS_CHILD|WS_VISIBLE|BS_OWNERDRAW|
        WS_TABSTOP,x,y,w,h,p,(HMENU)(INT_PTR)id,GetModuleHandleA(NULL),NULL);
    SetWindowLongPtrA(control,GWLP_USERDATA,kind);
    SendMessageA(control,WM_SETFONT,(WPARAM)g_fontBold,TRUE);
    return control;
}
static HWND createEdit(HWND p,int x,int y,int w,int h,const char *initial,
                       int password) {
    HWND control=CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT",initial ? initial : "",
          WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL|
          (password ? ES_PASSWORD : 0), x,y,w,h,p,NULL,GetModuleHandleA(NULL),NULL);
    SendMessageA(control,WM_SETFONT,(WPARAM)g_fontMedium,TRUE);
    SendMessageA(control,EM_SETLIMITTEXT,190,0);
    return control;
}
static HWND createCombo(HWND p,int x,int y,int w,int h,
                        const char *const values[],int count) {
    HWND control=CreateWindowExA(0,"COMBOBOX","",
          WS_CHILD|WS_VISIBLE|WS_TABSTOP|CBS_DROPDOWNLIST|WS_VSCROLL,
          x,y,w,h,p,NULL,GetModuleHandleA(NULL),NULL);
    for (int i=0;i<count;i++) SendMessageA(control,CB_ADDSTRING,0,(LPARAM)values[i]);
    SendMessageA(control,CB_SETCURSEL,0,0);
    SendMessageA(control,WM_SETFONT,(WPARAM)g_fontMedium,TRUE);
    return control;
}
static void comboGet(HWND h,char *out,int cap) {
    int i=(int)SendMessageA(h,CB_GETCURSEL,0,0);
    if (i < 0) { out[0]=0; return; }
    SendMessageA(h,CB_GETLBTEXT,i,(LPARAM)out);
    out[cap-1]=0;
}
static void updateViewport(void) {
    if (!g_main) return;
    RECT r; GetClientRect(g_main, &r);
    int cw = r.right - r.left, ch = r.bottom - r.top;
    if (cw < 1 || ch < 1) return;
    float xs = (float)cw / (float)APP_W;
    float ys = (float)ch / (float)APP_H;
    g_scale = xs < ys ? xs : ys;
    /* Scale uniformly: never stretch circles, typography, or financial charts. */
    g_viewW = (int)(APP_W * g_scale + 0.5f);
    g_viewH = (int)(APP_H * g_scale + 0.5f);
    g_viewX = (cw - g_viewW) / 2;
    g_viewY = (ch - g_viewH) / 2;
}
static int px(float coord) {
    return (int)(coord * g_scale + 0.5f);
}
static void layoutControls(void) {
    if (!g_main) return;
    int newSize = px(16.0f);
    if (newSize < 10) newSize = 10;
    if (newSize != g_fontPixelSize) {
        HFONT prevField = g_fieldFont, prevButton = g_buttonFont;
        g_fieldFont = font(newSize, 1);
        g_buttonFont = font(newSize + 2, 1);
        g_fontPixelSize = newSize;
        for (int i = 0; i < g_controlCount; i++)
            if (g_controls[i]) SendMessageA(g_controls[i], WM_SETFONT,
                                            (WPARAM)g_fieldFont, FALSE);
        if (prevField) DeleteObject(prevField);
        if (prevButton) DeleteObject(prevButton);
    }
    for (int i = 0; i < g_controlCount; i++) {
        if (!g_controls[i] || !IsWindow(g_controls[i])) continue;
        RECT r = g_designRects[i];
        SetWindowPos(g_controls[i], NULL,
            g_viewX + px((float)r.left), g_viewY + px((float)r.top),
            px((float)(r.right-r.left)), px((float)(r.bottom-r.top)),
            SWP_NOZORDER | SWP_NOACTIVATE);
        if (g_fieldFont) SendMessageA(g_controls[i],WM_SETFONT,(WPARAM)g_fieldFont,FALSE);
        InvalidateRect(g_controls[i],NULL,TRUE);
    }
}
static void trackControl(HWND h) {
    if (!h || g_controlCount >= MAX_CHILDREN) return;
    RECT r;
    GetWindowRect(h,&r);
    MapWindowPoints(NULL,g_main,(POINT*)&r,2);
    g_designRects[g_controlCount] = r;
    g_controls[g_controlCount++] = h;
}
static void toggleFullscreen(void) {
    if (!g_main || g_modal) return;
    if (!g_fullscreen) {
        MONITORINFO mi = { sizeof(mi) };
        HMONITOR monitor = MonitorFromWindow(g_main,MONITOR_DEFAULTTONEAREST);
        if (!GetMonitorInfoA(monitor,&mi)) return;
        g_previousPlacement.length = sizeof(g_previousPlacement);
        GetWindowPlacement(g_main,&g_previousPlacement);
        g_previousStyle = GetWindowLongPtrA(g_main,GWL_STYLE);
        SetWindowLongPtrA(g_main,GWL_STYLE,g_previousStyle & ~WS_OVERLAPPEDWINDOW);
        SetWindowPos(g_main,HWND_TOP,mi.rcMonitor.left,mi.rcMonitor.top,
          mi.rcMonitor.right-mi.rcMonitor.left,
          mi.rcMonitor.bottom-mi.rcMonitor.top,
          SWP_FRAMECHANGED | SWP_NOOWNERZORDER);
        g_fullscreen=1;
    } else {
        SetWindowLongPtrA(g_main,GWL_STYLE,g_previousStyle);
        SetWindowPlacement(g_main,&g_previousPlacement);
        SetWindowPos(g_main,NULL,0,0,0,0,
           SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
           SWP_NOOWNERZORDER);
        g_fullscreen=0;
    }
    updateViewport();layoutControls();repaintMain();
}
static HWND authField(int index) {
    switch (index) {
        case 0: return g_authUser;
        case 1: return g_authPass;
        case 2: return g_authConfirm;
        case 3: return g_authName;
        case 4: return g_authAge;
        case 5: return g_authPhone;
        case 6: return g_authCity;
        default: return NULL;
    }
}
static void clearAuthValidation(void) {
    memset(g_authErrors, 0, sizeof g_authErrors);
    g_authMessage[0] = 0;
}
static void authFieldError(int index, const char *message) {
    if (index >= 0 && index < 7)
        copySafe(g_authErrors[index], sizeof g_authErrors[index], message);
}
static int anyAuthErrors(void) {
    for (int i=0; i<7; i++) if (g_authErrors[i][0]) return 1;
    return 0;
}
static void showAuthValidation(const char *message) {
    copySafe(g_authMessage, sizeof g_authMessage, message);
    /* Move the typing cursor to the first problem, not to a separate popup. */
    int focused=0;
    for (int i=0; i<7; i++) {
        if (g_authErrors[i][0] && authField(i)) {
            InvalidateRect(authField(i),NULL,TRUE);
            if (!focused) {
                SetFocus(authField(i));
                focused=1;
            }
        }
    }
    repaintMain();
}
static void authTyping(HWND changed) {
    if (g_page != AUTH_LOGIN && g_page != AUTH_SIGNUP) return;
    for (int i=0; i<7; i++) {
        if (changed != authField(i)) continue;
        if (g_authErrors[i][0]) {
            g_authErrors[i][0]=0;
            if (!anyAuthErrors()) g_authMessage[0]=0;
            InvalidateRect(changed,NULL,TRUE);
            repaintMain();
        }
        return;
    }
}
static int parseAge(const char *text, int *age) {
    char *end; long value;
    if (!text[0]) return 0;
    value=strtol(text,&end,10);
    while (isspace((unsigned char)*end)) end++;
    if (*end || value<1 || value>120) return 0;
    *age=(int)value;
    return 1;
}
static int validPhone(const char *text) {
    int digits=0;
    for (int i=0;text[i];i++) {
        unsigned char ch=(unsigned char)text[i];
        if (isdigit(ch)) digits++;
        else if (ch!='+' && ch!='-' && ch!=' ' && ch!='(' && ch!=')') return 0;
    }
    return digits>=7 && digits<=15;
}
static void clearControls(void) {
    for (int i=0;i<g_controlCount;i++) if (g_controls[i]) DestroyWindow(g_controls[i]);
    g_controlCount=0; g_search=NULL;
    g_authUser=g_authPass=g_authConfirm=g_authName=g_authAge=g_authPhone=g_authCity=NULL;
}
static void showAuthenticationInputs(void) {
    int signup = g_page == AUTH_SIGNUP;
    int x=780,y=signup?182:264,w=365,h=34;
    g_authUser=createEdit(g_main,x,y,w,h,"",0);trackControl(g_authUser);
    SendMessageA(g_authUser,EM_SETLIMITTEXT,25,0);
    g_authPass=createEdit(g_main,x,y+67,w,h,"",1);trackControl(g_authPass);
    SendMessageA(g_authPass,EM_SETLIMITTEXT,29,0);
    if (signup) {
        g_authConfirm=createEdit(g_main,x,y+134,w,h,"",1);trackControl(g_authConfirm);
        SendMessageA(g_authConfirm,EM_SETLIMITTEXT,29,0);
        g_authName=createEdit(g_main,x,y+201,w,h,"",0); trackControl(g_authName);
        g_authAge=createEdit(g_main,x,y+268,100,h,"",0); trackControl(g_authAge);
        g_authPhone=createEdit(g_main,x+115,y+268,250,h,"",0);trackControl(g_authPhone);
        g_authCity=createEdit(g_main,x,y+335,w,h,"",0);trackControl(g_authCity);
    }
    trackControl(createBtn(g_main,signup?"Create my account":"Sign in to dashboard",
                  x,signup?661:460,w,46,IDC_AUTH_PRIMARY,1));
    if (signup) {
        /* Explicit back option prevents getting stuck on Create Account. */
        trackControl(createBtn(g_main,"< Back",x,712,121,38,IDC_AUTH_BACK,0));
        trackControl(createBtn(g_main,"I already have an account",x+137,712,w-137,38,
                               IDC_AUTH_SWITCH,0));
    } else {
        trackControl(createBtn(g_main,"New here? Create account",x,523,w,38,
                               IDC_AUTH_SWITCH,0));
    }
}
static void setPage(int page) {
    if (page>=PAGE_HOME && !g_authenticated) page=AUTH_LOGIN;
    clearControls(); g_page=page;
    g_alertVisible=0;
    clearAuthValidation();
    if (page==AUTH_LOGIN || page==AUTH_SIGNUP) showAuthenticationInputs();
    if (page==PAGE_EXPENSE) {
        g_search=createEdit(g_main,840,159,327,37,"",0);
        trackControl(g_search);
    }
    updateViewport();layoutControls();repaintMain();
    if ((page==AUTH_LOGIN || page==AUTH_SIGNUP) && g_authUser)
        SetFocus(g_authUser);
}
static void repaintMain(void) { if(g_main) InvalidateRect(g_main,NULL,TRUE); }

/* Valid username characters also prevent path traversal in per-user filenames. */
static int validUsername(const char *name) {
    size_t len=strlen(name);
    if (len<3 || len>25) return 0;
    for (size_t i=0;i<len;i++) {
        char c=name[i];
        if (!isalnum((unsigned char)c) && c!='_' && c!='-') return 0;
    }
    return 1;
}
static void authSubmit(void) {
    char u[30],p[30],confirm[30],name[50],ageText[20],phone[20],city[30];
    int age=0;
    if (g_authenticated || (g_page!=AUTH_LOGIN && g_page!=AUTH_SIGNUP)) return;
    clearAuthValidation();
    getText(g_authUser,u,sizeof u); trim(u);
    getText(g_authPass,p,sizeof p);

    if (!u[0]) authFieldError(0,"Required");
    else if (!validUsername(u)) authFieldError(0,"3-25 letters / digits");
    if (!p[0]) authFieldError(1,"Required");

    if (g_page==AUTH_LOGIN) {
        if (anyAuthErrors()) {
            showAuthValidation("Please fill in the red fields before signing in.");
            return;
        }
        FILE *fp=fopen("accounts.txt","r");
        char savedName[30],savedPass[30];
        int matched=0, accountsAvailable=(fp!=NULL);
        if (fp) {
            while (fgets(savedName,sizeof savedName,fp)) {
                if (!fgets(savedPass,sizeof savedPass,fp)) break;
                removeNewline(savedName);removeNewline(savedPass);
                if (!strcmp(u,savedName) && !strcmp(p,savedPass)) {
                    matched=1; break;
                }
            }
            fclose(fp);
        }
        if (!matched) {
            if (!accountsAvailable) {
                authFieldError(0,"No account found");
                showAuthValidation("No account exists yet. Choose Create account below.");
            } else if (!usernameExists(u)) {
                authFieldError(0,"Not registered");
                showAuthValidation("This username has no account. Create one or go Back.");
            } else {
                authFieldError(1,"Incorrect password");
                showAuthValidation("Password is incorrect. Try again, or switch account.");
            }
            return;
        }
        copySafe(username,sizeof username,u); makeFileNames(); loadData();loadBudgetLimit();
        if (!userSet) {
            copySafe(user.name,sizeof user.name,username);
            user.age=18;user.phone[0]=0;user.city[0]=0;
            userSet=1;saveUser();
        }
        g_authenticated=1;g_incomeOffset=g_expenseOffset=g_transactionOffset=0;
        g_selectedIncome=g_selectedExpense=-1;g_alertVisible=0;
        g_transactionFilter=g_reportMonthOffset=0;
        setPage(PAGE_HOME);
        return;
    }

    getText(g_authConfirm,confirm,sizeof confirm);
    getText(g_authName,name,sizeof name); trim(name);
    getText(g_authAge,ageText,sizeof ageText); trim(ageText);
    getText(g_authPhone,phone,sizeof phone); trim(phone);
    getText(g_authCity,city,sizeof city); trim(city);

    if (!g_authErrors[1][0] && strlen(p)<6)
        authFieldError(1,"Minimum 6 characters");
    if (!confirm[0]) authFieldError(2,"Required");
    else if (strcmp(p,confirm)!=0) authFieldError(2,"Passwords differ");
    if (!name[0]) authFieldError(3,"Required");
    if (!ageText[0]) authFieldError(4,"Required");
    else if (!parseAge(ageText,&age)) authFieldError(4,"1-120 only");
    if (!phone[0]) authFieldError(5,"Required");
    else if (!validPhone(phone)) authFieldError(5,"Invalid phone");
    if (!city[0]) authFieldError(6,"Required");
    if (!g_authErrors[0][0] && usernameExists(u))
        authFieldError(0,"Already exists");

    if (anyAuthErrors()) {
        showAuthValidation("Please correct the red fields to create your account.");
        return;
    }
    FILE *fp=fopen("accounts.txt","a");
    if (!fp) {
        showAuthValidation("Could not save the account. Check folder permissions.");
        return;
    }
    int wrote=fprintf(fp,"%s\n%s\n",u,p);
    int closed=fclose(fp);
    if (wrote<0 || closed==EOF) {
        showAuthValidation("Account file could not be written. Try again.");
        return;
    }
    copySafe(username,sizeof username,u);makeFileNames();
    copySafe(user.name,sizeof user.name,name);user.age=age;
    copySafe(user.phone,sizeof user.phone,phone);
    copySafe(user.city,sizeof user.city,city);userSet=1;
    incomeCount=0;expenseCount=0;savingTarget=0;g_spendingLimit=0;
    saveUser();
    /* Fourth point integrated: fresh accounts start with empty data files. */
    fp=fopen(incomeFile,"w");if(fp)fclose(fp);
    fp=fopen(expenseFile,"w");if(fp)fclose(fp);
    saveSavingTarget();saveBudgetLimit();
    g_authenticated=1;g_incomeOffset=g_expenseOffset=g_transactionOffset=0;
        g_selectedIncome=g_selectedExpense=-1;g_alertVisible=0;
        g_transactionFilter=g_reportMonthOffset=0;
    setPage(PAGE_HOME);
}

/* Alert levels: 0 guidance, 1 amber 80%, 2 orange 85%, 3 red 90% / cap. */
static COLORREF budgetAlertColor(int level) {
    return level>=3 ? ERROR_RED : level==2 ? CORAL : level==1 ? GOLD : MINT;
}
static void showBudgetAlert(int level, const char *title,
                            const char *body, const char *hint, double ratio) {
    g_alertLevel=level;
    g_alertRatio=ratio;
    copySafe(g_alertTitle,sizeof g_alertTitle,title);
    copySafe(g_alertBody,sizeof g_alertBody,body);
    copySafe(g_alertHint,sizeof g_alertHint,hint);
    g_alertVisible=1;
    if(g_search)ShowWindow(g_search,SW_HIDE);
    repaintMain();
}
static void dismissBudgetAlert(void) {
    g_alertVisible=0;
    if(g_search)ShowWindow(g_search,SW_SHOW);
    repaintMain();
}
static void paintBudgetAlert(HDC dc) {
    if (!g_alertVisible || !g_authenticated) return;
    COLORREF accent=budgetAlertColor(g_alertLevel);
    /* Dedicated graphic dialog, with high-contrast urgency and a progress meter. */
    filled(dc,254,0,APP_W-254,APP_H,C(27,42,60));
    shadowCard(dc,398,189,701,405,24);
    rounded(dc,399,189,699,9,6,accent);
    rounded(dc,439,228,72,72,22,
        g_alertLevel>=3?C(255,230,234):g_alertLevel>=1?C(255,245,223):C(228,249,240));
    label(dc,g_alertLevel?"!":"OK",440,227,70,74,g_fontHero,accent,
          DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,"BUDGETLY | FINANCIAL ALERT",534,229,510,22,g_fontSmall,MUTED,
          DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,g_alertTitle,535,252,520,43,g_fontTitle,INK,
          DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,g_alertBody,444,322,607,55,g_fontMedium,INK,
          DT_LEFT|DT_WORDBREAK);
    label(dc,"SPENDING LEVEL",442,393,380,23,g_fontSmall,MUTED,
          DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    char pct[40];snprintf(pct,sizeof pct,"%.0f%% of income",g_alertRatio*100.0);
    label(dc,pct,836,390,214,25,g_fontBold,accent,
          DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,441,426,613,16,10,C(232,239,243));
    double ratio=g_alertRatio;
    if(ratio<0)ratio=0;
    if(ratio>1)ratio=1;
    if(ratio>0)rounded(dc,441,426,(int)(613*ratio),16,10,accent);
    for(int at=80;at<=90;at+=5) {
        int xx=441+613*at/100;
        line(dc,xx,447,xx,457,at==90?ERROR_RED:GOLD);
        char text[16];snprintf(text,sizeof text,"%d%%",at);
        label(dc,text,xx-24,459,50,18,g_fontSmall,MUTED,DT_CENTER|DT_SINGLELINE);
    }
    label(dc,g_alertHint,444,488,611,30,g_fontRegular,MUTED,
          DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,620,531,207,43,12,C(233,241,244));
    label(dc,"Review budget",620,531,207,43,g_fontMedium,INK,
          DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,842,531,211,43,12,accent);
    label(dc,"Got it",842,531,211,43,g_fontMedium,C(255,255,255),
          DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}

static int catIndex(const char *cat) {
    if (!_stricmp(cat,"Food"))return 0;
    if (!_stricmp(cat,"Transport"))return 1;
    if (!_stricmp(cat,"Shopping"))return 2;
    if (!_stricmp(cat,"Bills"))return 3;
    return 4;
}
static void categoryTotals(double v[5]) {
    for(int i=0;i<5;i++)v[i]=0;
    for(int i=0;i<expenseCount;i++)
        if (isPaid(i)) v[catIndex(expenses[i].category)]+=expenses[i].amount;
}
static void drawIcon(HDC dc,int x,int y,const char *s) {
    label(dc,s,x,y,25,24,g_fontBold,C(155,170,188),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
}
static void navItem(HDC dc,int idx,int y,const char *symbol,const char *title) {
    int selected=g_page==PAGE_HOME+idx;
    if(selected)rounded(dc,15,y,222,43,12,C(34,63,70));
    else if(g_navHover==idx)rounded(dc,15,y,222,43,12,C(31,42,62));
    if(selected)rounded(dc,15,y+6,4,30,4,MINT);
    drawIcon(dc,36,y+10,symbol);
    label(dc,title,78,y+3,140,38,g_fontMedium,
          selected?C(240,255,251):C(171,186,202),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
}
static void drawSidebar(HDC dc) {
    filled(dc,0,0,254,APP_H,NAVY);
    rounded(dc,28,28,48,48,13,MINT);
    label(dc,"B",28,27,48,49,g_fontTitle,C(255,255,255),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,"BUDGETLY",87,33,155,35,g_fontBold,C(255,255,255),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"PERSONAL FINANCE",32,111,196,26,g_fontSmall,C(116,136,155),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    navItem(dc,0,164,"01","Overview");
    navItem(dc,1,216,"02","Income");
    navItem(dc,2,268,"03","Expenses");
    navItem(dc,3,320,"04","All transactions");
    navItem(dc,4,372,"05","Reports");
    navItem(dc,5,424,"06","Budget tools");
    navItem(dc,6,476,"07","My profile");
    line(dc,26,640,225,640,C(46,59,77));
    rounded(dc,27,666,36,36,12,C(52,77,90));
    label(dc,user.name[0]?user.name:username,75,662,154,26,g_fontMedium,C(252,255,255),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"Signed in",75,691,140,18,g_fontSmall,C(135,157,175),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"A",27,665,36,36,g_fontBold,C(236,255,246),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,28,724,199,36,10,C(40,54,72));
    label(dc,"Log out",28,724,199,36,g_fontMedium,C(203,216,230),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}
static void paintTitle(HDC dc,const char *title,const char *subtitle) {
    label(dc,title,292,37,750,45,g_fontTitle,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,subtitle,294,83,840,25,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,1053,42,152,39,12,C(230,248,241));
    label(dc,"LOCALLY STORED",1055,42,148,39,g_fontSmall,C(32,133,110),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}
static void cardNumber(HDC dc,int x,int y,int w,int h,const char *title,
                       double value,COLORREF accent,const char *tag) {
    char text[64];shadowCard(dc,x,y,w,h,16);
    rounded(dc,x+20,y+21,9,9,5,accent);
    label(dc,title,x+38,y+15,w-56,27,g_fontMedium,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    money(text,sizeof text,value);
    label(dc,text,x+20,y+53,w-35,40,g_fontTitle,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,tag,x+21,y+h-33,w-40,22,g_fontSmall,C(126,140,155),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
}
static void screenBtn(HDC dc,int x,int y,int w,int h,const char *title,int dark) {
    rounded(dc,x,y,w,h,12,dark?NAVY:MINT);
    label(dc,title,x+8,y,w-16,h,g_fontMedium,C(255,255,255),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}
static void categoryBars(HDC dc,int x,int y,int w,int h,int horizontal) {
    const char *cats[5]={"Food","Transport","Shopping","Bills","Other"};
    COLORREF colors[5]={C(64,153,190),MINT,GOLD,C(139,131,217),CORAL};
    double v[5],maximum=1;
    categoryTotals(v);
    for(int i=0;i<5;i++)if(v[i]>maximum)maximum=v[i];
    if(!horizontal) {
        for(int i=0;i<5;i++) {
            int bw=45, bx=x+45+i*(w-85)/5+10;
            int bh=(int)((h-54)*v[i]/maximum);
            filled(dc,bx,y+8,bw,h-44,C(244,247,251));
            if(bh>0)rounded(dc,bx,y+8+(h-54-bh),bw,bh+1,8,colors[i]);
            label(dc,cats[i],bx-6,y+h-32,bw+12,24,g_fontSmall,MUTED,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        }
        line(dc,x+35,y+h-39,x+w-25,y+h-39,LINE);
    } else {
        for(int i=0;i<5;i++) {
            int yy=y+i*56, bw=(int)((w-195)*v[i]/maximum);
            label(dc,cats[i],x,yy,105,30,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
            rounded(dc,x+109,yy+9,w-215,14,8,C(239,243,248));
            if (bw>0)rounded(dc,x+109,yy+9,bw,14,8,colors[i]);
            char amt[64]; snprintf(amt,sizeof amt,"%.0f",v[i]);
            label(dc,amt,x+w-92,yy,88,30,g_fontMedium,INK,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
        }
    }
}
static void drawRecent(HDC dc,int x,int y,int w,int maxRows) {
    int n=0;
    for(int i=expenseCount-1;i>=0&&n<maxRows;i--,n++) {
        int yy=y+n*55;
        if(n)line(dc,x+16,yy-4,x+w-16,yy-4,LINE);
        rounded(dc,x+17,yy+7,31,31,10,C(255,242,229));
        label(dc,"-",x+17,yy+7,31,31,g_fontBold,C(232,139,64),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        label(dc,expenses[i].category,x+62,yy+3,w-170,25,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,expenses[i].date,x+62,yy+27,160,20,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        char amt[55];snprintf(amt,sizeof amt,"-%.2f",expenses[i].amount);
        label(dc,amt,x+w-140,yy+10,120,29,g_fontBold,C(222,86,105),DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
    }
    if(!n)label(dc,"Your recent expenses will appear here.",x+18,y+60,w-35,45,
                g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
}
static void paintDashboard(HDC dc) {
    float bank,cash;double income=getTotalIncome(),spent=getTotalExpense();
    double remaining;char msg[200];
    getBalances(&bank,&cash);remaining=bank+cash;
    paintTitle(dc,"Financial overview","A clear picture of your money, all in one place.");
    cardNumber(dc,293,135,214,138,"Total income",income,MINT,"All recorded income");
    cardNumber(dc,524,135,214,138,"Paid expenses",spent,GOLD,"Completed payments");
    cardNumber(dc,755,135,214,138,"Available",remaining,C(61,148,183),"Bank + cash balance");
    cardNumber(dc,986,135,214,138,"Savings goal",savingTarget,C(139,131,217),"Your personal target");
    shadowCard(dc,293,294,581,336,18);
    label(dc,"Spending by category",314,311,370,36,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"Paid expense distribution",315,346,330,20,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    categoryBars(dc,307,372,550,222,0);
    shadowCard(dc,893,294,307,336,18);
    label(dc,"Recent activity",915,311,265,36,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    drawRecent(dc,901,366,286,4);
    shadowCard(dc,293,649,907,102,16);
    int level=income>0 && spent>=income*.90?3:
        income>0 && spent>=income*.85?2:
        income>0 && spent>=income*.80?1:0;
    COLORREF state=budgetAlertColor(level);
    rounded(dc,312,666,41,41,13,
        level>=3?C(255,231,235):level>=1?C(255,246,224):C(224,247,238));
    label(dc,level?"!":"OK",312,665,41,42,g_fontBold,state,
          DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,level==3?"Critical budget alert":level==2?"High spending alert":
          level==1?"Budget needs attention":"Your budget pulse",
          366,662,508,26,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    if(income<=0)copySafe(msg,sizeof msg,"Add income to activate your budget monitor.");
    else if(level==3)copySafe(msg,sizeof msg,"90%+ spent. Review your budget before adding expenses.");
    else if(level==2)copySafe(msg,sizeof msg,"85% spent. Protect the money left for your savings.");
    else if(level==1)copySafe(msg,sizeof msg,"80% spent. Check upcoming purchases carefully.");
    else if(remaining<savingTarget)copySafe(msg,sizeof msg,"Your available balance is below your savings goal.");
    else copySafe(msg,sizeof msg,"Spending is currently under the 80% warning point.");
    label(dc,msg,366,688,657,24,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,366,724,599,9,6,C(233,239,243));
    double pulse=income>0?spent/income:0;
    if(pulse<0)pulse=0;if(pulse>1)pulse=1;
    if(pulse>0)rounded(dc,366,724,(int)(599*pulse),9,6,state);
    char pulseText[30];snprintf(pulseText,sizeof pulseText,"%.0f%%",income>0?100*spent/income:0);
    label(dc,pulseText,976,715,65,27,g_fontMedium,state,DT_CENTER|DT_SINGLELINE);
    screenBtn(dc,1070,690,111,40,"+ Income",0);
}
static int expenseMatches(int i,const char *filter) {
    if(!filter[0])return 1;
    char a[60],b[60];copySafe(a,sizeof a,expenses[i].category);
    copySafe(b,sizeof b,filter);
    for(int k=0;a[k];k++)a[k]=(char)tolower((unsigned char)a[k]);
    for(int k=0;b[k];k++)b[k]=(char)tolower((unsigned char)b[k]);
    return strstr(a,b)!=NULL;
}
static int filteredCount(const char *filter) {
    int n=0;for(int i=0;i<expenseCount;i++)if(expenseMatches(i,filter))n++;
    return n;
}
static void tableHeader(HDC dc,int x,int y,int w,const char *a,const char *b,
                        const char *c,const char *d) {
    rounded(dc,x,y,w,50,10,C(239,244,247));
    label(dc,a,x+19,y,178,50,g_fontBold,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,b,x+240,y,198,50,g_fontBold,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,c,x+470,y,198,50,g_fontBold,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,d,x+w-183,y,160,50,g_fontBold,MUTED,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
}
static void paintIncome(HDC dc) {
    paintTitle(dc,"Income management","Record every payment and track where your money comes from.");
    screenBtn(dc,1024,142,173,48,"+ Add income",0);
    rounded(dc,837,142,174,48,12,g_selectedIncome>=0?CORAL:C(197,207,211));
    label(dc,"Delete selected",840,142,168,48,g_fontMedium,C(255,255,255),
          DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    shadowCard(dc,293,215,907,527,16);
    label(dc,"Income history",313,234,420,38,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    tableHeader(dc,311,283,870,"DATE","SOURCE","PAYMENT METHOD","AMOUNT / ACTION");
    for(int j=0;j<7;j++) {
        int i=incomeCount-1-g_incomeOffset-j;if(i<0)break;
        int yy=341+j*51;char amt[70];money(amt,sizeof amt,incomes[i].amount);
        if(i==g_selectedIncome)filled(dc,311,yy,870,51,C(224,246,240));
        else if(j%2==1)filled(dc,311,yy,870,51,C(250,251,253));
        label(dc,incomes[i].date,329,yy,174,51,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,incomes[i].source,551,yy,205,51,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,incomes[i].paymentMethod,781,yy,189,51,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,amt,967,yy,143,51,g_fontBold,C(24,149,113),DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
        rounded(dc,1120,yy+10,56,29,8,C(255,233,236));
        label(dc,"Delete",1120,yy+10,56,29,g_fontSmall,ERROR_RED,
              DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }
    if(!incomeCount)label(dc,"No incomes yet. Click '+ Add income' to get started.",
           335,430,820,50,g_fontRegular,MUTED,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    screenBtn(dc,1033,697,63,30,"Prev",1);
    screenBtn(dc,1108,697,65,30,"Next",1);
    char txt[80];snprintf(txt,sizeof txt,"%d recorded income%s",incomeCount,incomeCount==1?"":"s");
    label(dc,txt,328,693,300,40,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
}
static void paintExpenses(HDC dc) {
    char filter[55];getText(g_search,filter,sizeof filter);trim(filter);
    paintTitle(dc,"Expense management","Track spending, categories and outstanding payments.");
    screenBtn(dc,314,154,163,42,"+ Add expense",0);
    rounded(dc,489,154,158,42,12,g_selectedExpense>=0?CORAL:C(197,207,211));
    label(dc,"Delete selected",491,154,154,42,g_fontMedium,C(255,255,255),
          DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    shadowCard(dc,293,215,907,527,16);
    label(dc,"Expense history",313,234,420,38,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"Filter by category:",831,237,310,29,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    tableHeader(dc,311,283,870,"DATE","CATEGORY / PLACE","PAYMENT / STATUS","AMOUNT / ACTION");
    int count=0,skip=0;
    for(int i=expenseCount-1;i>=0 && count<7;i--) {
        if(!expenseMatches(i,filter))continue;
        if(skip++<g_expenseOffset)continue;
        int yy=341+count*51;char amt[70],detail[96];
        money(amt,sizeof amt,expenses[i].amount);
        snprintf(detail,sizeof detail,"%s  /  %s",expenses[i].paymentMethod,expenses[i].status);
        if(i==g_selectedExpense)filled(dc,311,yy,870,51,C(255,239,235));
        else if(count%2)filled(dc,311,yy,870,51,C(250,251,253));
        label(dc,expenses[i].date,329,yy,174,51,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,expenses[i].category,551,yy,190,29,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,expenses[i].place,551,yy+26,200,24,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,detail,781,yy,189,51,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,amt,967,yy,143,51,g_fontBold,C(222,86,105),DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
        rounded(dc,1120,yy+10,56,29,8,C(255,233,236));
        label(dc,"Delete",1120,yy+10,56,29,g_fontSmall,ERROR_RED,
              DT_CENTER|DT_VCENTER|DT_SINGLELINE);
        count++;
    }
    if(!count)label(dc,expenseCount?"No matching expenses found.":"No expenses yet. Click '+ Add expense' to get started.",
           333,420,823,50,g_fontRegular,MUTED,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    screenBtn(dc,1033,697,63,30,"Prev",1);
    screenBtn(dc,1108,697,65,30,"Next",1);
    char txt[80];snprintf(txt,sizeof txt,"%d matching expense%s",filteredCount(filter),filteredCount(filter)==1?"":"s");
    label(dc,txt,328,693,300,40,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
}
/* Original All Transactions feature, now presented as one interactive table.
   Income and expense records are joined in a simple fixed-size C array. */
typedef struct {int kind,index,dateKey;} TransactionRow;
static int dateKey(const char *date) {
    int d=0,m=0,y=0;sscanf(date,"%d-%d-%d",&d,&m,&y);
    return y*10000 + m*100 + d;
}
static int allTransactions(TransactionRow rows[200]) {
    int n=0;
    for(int i=0;i<incomeCount;i++)
        if(g_transactionFilter==0 || g_transactionFilter==1) {
            rows[n].kind=0;rows[n].index=i;
            rows[n++].dateKey=dateKey(incomes[i].date);
        }
    for(int i=0;i<expenseCount;i++)
        if(g_transactionFilter==0 || g_transactionFilter==2 ||
           (g_transactionFilter==3 && !isPaid(i))) {
            rows[n].kind=1;rows[n].index=i;
            rows[n++].dateKey=dateKey(expenses[i].date);
        }
    for(int i=1;i<n;i++) {
        TransactionRow temp=rows[i];int j=i;
        while(j>0 && rows[j-1].dateKey<temp.dateKey) {
            rows[j]=rows[j-1];j--;
        }
        rows[j]=temp;
    }
    return n;
}
static void paintTransactions(HDC dc) {
    paintTitle(dc,"All transactions","Income, paid expenses and unpaid bills in one place.");
    const char *filters[4]={"All","Income","Expenses","Unpaid"};
    for(int k=0;k<4;k++)screenBtn(dc,300+k*174,144,155,41,filters[k],
        g_transactionFilter==k);
    shadowCard(dc,293,215,907,527,16);
    label(dc,"Transaction history",313,234,570,38,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    tableHeader(dc,311,283,870,"DATE","DETAILS","METHOD / STATUS","AMOUNT / ACTION");
    TransactionRow rows[200];int n=allTransactions(rows);
    if(g_transactionOffset>=n)g_transactionOffset=n>0?((n-1)/7)*7:0;
    for(int j=0;j<7 && j+g_transactionOffset<n;j++) {
        TransactionRow row=rows[j+g_transactionOffset];
        int yy=341+j*51;char amount[75],detail[100],status[78];
        const char *date;COLORREF amountColor;
        if(j%2)filled(dc,311,yy,870,51,C(250,252,251));
        if(row.kind==0) {
            struct Income *in=&incomes[row.index];
            date=in->date;snprintf(detail,sizeof detail,"Income: %s",in->source);
            snprintf(status,sizeof status,"%s",in->paymentMethod);
            money(amount,sizeof amount,in->amount);amountColor=MINT;
        } else {
            struct Expense *ex=&expenses[row.index];
            date=ex->date;snprintf(detail,sizeof detail,"%s / %s",ex->category,ex->place);
            snprintf(status,sizeof status,"%s / %s",ex->paymentMethod,ex->status);
            money(amount,sizeof amount,ex->amount);
            amountColor=isPaid(row.index)?CORAL:MUTED;
        }
        label(dc,date,329,yy,176,51,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,detail,551,yy,216,51,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,status,781,yy,188,51,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,amount,967,yy,143,51,g_fontBold,amountColor,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
        rounded(dc,1120,yy+10,56,29,8,C(255,233,236));
        label(dc,"Delete",1120,yy+10,56,29,g_fontSmall,ERROR_RED,
              DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    }
    if(!n)label(dc,"No transactions match this filter.",339,420,808,45,
        g_fontRegular,MUTED,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    char caption[80];snprintf(caption,sizeof caption,"%d record%s",n,n==1?"":"s");
    label(dc,caption,327,694,300,37,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    screenBtn(dc,1033,697,63,30,"Prev",1);
    screenBtn(dc,1108,697,65,30,"Next",1);
}

/* Functional budget model based on Ali's warning, savings and balance logic.
   The user's actual records remain untouched by what-if projections. */
static void paintBudgetTools(HDC dc) {
    float bank,cash;getBalances(&bank,&cash);
    double income=getTotalIncome(),spent=getTotalExpense();
    double remaining=income-spent;
    double warning80=income*0.80,limit=g_spendingLimit>0?g_spendingLimit:warning80;
    double vehicle=0,unpaid=0;
    for(int i=0;i<expenseCount;i++) {
        if(!_stricmp(expenses[i].category,"Transport") && isPaid(i))vehicle+=expenses[i].amount;
        if(!isPaid(i))unpaid+=expenses[i].amount;
    }
    paintTitle(dc,"Budget tools & models","Original C calculations, warnings and a what-if planner.");
    cardNumber(dc,294,140,214,127,"Bank balance",bank,MINT,"Available in bank");
    cardNumber(dc,524,140,214,127,"Cash balance",cash,GOLD,"Available in cash");
    cardNumber(dc,754,140,214,127,"Saving target",savingTarget,C(130,113,213),"Your reserved goal");
    cardNumber(dc,984,140,214,127,"Unpaid bills",unpaid,CORAL,"Not counted as spent");
    shadowCard(dc,294,287,587,307,17);
    label(dc,"Budget warning monitor",315,302,520,36,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    char t[130];snprintf(t,sizeof t,"Paid spending:  PKR %.2f of income PKR %.2f",spent,income);
    label(dc,t,316,345,542,30,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    double ratio=income>0?spent/income:0;
    if(ratio<0)ratio=0;if(ratio>1)ratio=1;
    rounded(dc,318,394,536,19,10,C(235,243,241));
    if(ratio>0)rounded(dc,318,394,(int)(536*ratio),19,10,
        spent>=income*.90?ERROR_RED:spent>=income*.80?GOLD:MINT);
    const char *status="Add income to enable spending alerts.";
    if(income>0) {
        if(spent>=income*.90) status="90% WARNING: critical spending level.";
        else if(spent>=income*.85) status="85% WARNING: spending is high.";
        else if(spent>=income*.80) status="80% WARNING: review upcoming purchases.";
        else status="Spending is under 80% of your income.";
    }
    int risk=income>0 && spent>=income*.90?3:
             income>0 && spent>=income*.85?2:
             income>0 && spent>=income*.80?1:0;
    COLORREF riskColor=budgetAlertColor(risk);
    rounded(dc,316,421,538,47,12,risk>=3?C(255,234,237):
        risk>=1?C(255,246,223):C(226,247,240));
    rounded(dc,325,427,32,33,10,riskColor);
    label(dc,risk?"!":"OK",325,427,32,33,g_fontBold,C(255,255,255),
        DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,status,367,425,480,39,g_fontMedium,risk>=1?riskColor:INK,
        DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    for(int threshold=80;threshold<=90;threshold+=5) {
        int xx=318+536*threshold/100;
        line(dc,xx,416,xx,422,threshold==90?ERROR_RED:GOLD);
    }
    char low[130];snprintf(low,sizeof low,"80%% warning point: PKR %.2f",warning80);
    label(dc,low,318,462,520,28,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    snprintf(low,sizeof low,"Spending cap: PKR %.2f %s",limit,
       g_spendingLimit>0?"(custom)":"(default 80%)");
    label(dc,low,318,493,535,29,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    if(income>0 && spent>limit)
        label(dc,"Limit exceeded - check your spending plan.",318,530,535,31,
            g_fontBold,ERROR_RED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    else label(dc,"You are within your configured spending cap.",318,530,535,31,
            g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    shadowCard(dc,901,287,297,307,17);
    label(dc,"Savings & spending",921,305,252,33,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    snprintf(t,sizeof t,"Remaining: PKR %.2f",remaining);
    label(dc,t,920,366,265,35,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    snprintf(t,sizeof t,"Target: PKR %.2f",savingTarget);
    label(dc,t,920,406,263,34,g_fontMedium,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,remaining>=savingTarget?"Target achievable":"Saving zone reached",
        920,459,265,35,g_fontBold,remaining>=savingTarget?MINT:CORAL,
        DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    screenBtn(dc,916,527,267,45,"Set saving goal",0);
    shadowCard(dc,294,611,904,140,17);
    label(dc,"Practical budget actions",314,624,546,28,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    screenBtn(dc,314,668,252,47,"Set spending limit",1);
    screenBtn(dc,585,668,252,47,"What-if budget model",0);
    char transport[120];snprintf(transport,sizeof transport,"Transport: PKR %.2f",vehicle);
    label(dc,transport,855,675,324,34,g_fontMedium,MUTED,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
}

static void drawDonut(HDC dc,int x,int y,int size,double ratio) {
    if(ratio<0)ratio=0;if(ratio>1)ratio=1;
    HBRUSH br=CreateSolidBrush(C(228,234,244));
    HPEN p=CreatePen(PS_SOLID,1,C(228,234,244));
    HGDIOBJ ob=SelectObject(dc,br),op=SelectObject(dc,p);
    Ellipse(dc,x,y,x+size,y+size);
    SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(br);DeleteObject(p);
    if(ratio>=1) {
        HBRUSH full=CreateSolidBrush(MINT);HPEN edge=CreatePen(PS_SOLID,1,MINT);
        HGDIOBJ oldB=SelectObject(dc,full),oldP=SelectObject(dc,edge);
        Ellipse(dc,x,y,x+size,y+size);
        SelectObject(dc,oldB);SelectObject(dc,oldP);
        DeleteObject(full);DeleteObject(edge);
    } else if(ratio>0) {
        br=CreateSolidBrush(MINT);p=CreatePen(PS_SOLID,1,MINT);
        ob=SelectObject(dc,br);op=SelectObject(dc,p);
        double a=ratio*6.2831853;
        int cx=x+size/2,cy=y+size/2;
        int sx=cx,sy=y;
        int ex=cx+(int)(sin(a)*size/2),ey=cy-(int)(cos(a)*size/2);
        Pie(dc,x,y,x+size,y+size,sx,sy,ex,ey);
        SelectObject(dc,ob);SelectObject(dc,op);DeleteObject(br);DeleteObject(p);
    }
    rounded(dc,x+33,y+33,size-66,size-66,size,C(255,255,255));
    char pct[50];snprintf(pct,sizeof pct,"%.0f%%",ratio*100);
    label(dc,pct,x+30,y+55,size-60,56,g_fontTitle,INK,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,"SAVED",x+32,y+107,size-64,25,g_fontSmall,MUTED,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
}
static void paintReports(HDC dc) {
    float bank,cash;double income=getTotalIncome(),spent=getTotalExpense();
    getBalances(&bank,&cash);
    int month,year;reportMonth(&month,&year);
    double monthIncome,monthSpent,categories[5],max=1;
    monthTotals(&monthIncome,&monthSpent,categories);
    for(int i=0;i<5;i++)if(categories[i]>max)max=categories[i];
    paintTitle(dc,"Insights & reports","Real monthly totals, category breakdown and savings progress.");
    char period[80];snprintf(period,sizeof period,"%s %d",monthTitle(month),year);
    rounded(dc,291,127,907,49,13,C(231,245,238));
    label(dc,period,308,130,480,39,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    screenBtn(dc,934,134,109,36,"Previous",1);
    screenBtn(dc,1051,134,129,36,"Next month",1);
    shadowCard(dc,293,196,585,357,18);
    label(dc,"Category spending this month",313,213,530,37,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    const char *cats[5]={"Food","Transport","Shopping","Bills","Other"};
    COLORREF colors[5]={C(64,153,190),MINT,GOLD,C(139,131,217),CORAL};
    for(int i=0;i<5;i++) {
        int yy=270+i*53;double v=categories[i];
        label(dc,cats[i],312,yy,123,34,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        rounded(dc,435,yy+10,301,14,8,C(237,244,241));
        int width=(int)(301*v/max);
        if(width>0)rounded(dc,435,yy+10,width,14,8,colors[i]);
        char vText[70];snprintf(vText,sizeof vText,"%.0f",v);
        label(dc,vText,753,yy,98,34,g_fontMedium,INK,DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
    }
    shadowCard(dc,895,196,305,357,18);
    label(dc,"Savings progress",914,214,268,33,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    double remaining=income-spent;
    drawDonut(dc,966,269,162,savingTarget>0?remaining/savingTarget:0);
    char totalText[80];money(totalText,sizeof totalText,remaining);
    label(dc,totalText,914,457,267,36,g_fontBold,INK,DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    screenBtn(dc,938,502,217,38,"Set savings goal",0);
    shadowCard(dc,293,574,907,166,18);
    label(dc,"Monthly financial summary",315,587,615,35,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    char summary[210];snprintf(summary,sizeof summary,
        "Income  PKR %.2f          Paid expenses  PKR %.2f          Net  PKR %.2f",
        monthIncome,monthSpent,monthIncome-monthSpent);
    label(dc,summary,315,629,849,40,g_fontMedium,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    snprintf(summary,sizeof summary,"All-time balances   Bank PKR %.2f  |  Cash PKR %.2f",
        (double)bank,(double)cash);
    label(dc,summary,315,685,849,31,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
}
static void paintProfile(HDC dc) {
    paintTitle(dc,"My account","Manage your personal details and financial preferences.");
    shadowCard(dc,294,144,907,500,18);
    rounded(dc,330,185,76,76,22,C(225,245,238));
    label(dc,"A",330,185,76,76,g_fontHero,C(30,156,125),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,user.name,432,189,550,42,g_fontTitle,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,username,434,232,460,27,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    line(dc,331,286,1155,286,LINE);
    char a[80];snprintf(a,sizeof a,"%d years",user.age);
    label(dc,"AGE",332,305,180,23,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,a,332,332,360,39,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"PHONE",709,305,180,23,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,user.phone[0]?user.phone:"Not provided",709,332,375,39,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"CITY",332,405,180,23,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,user.city[0]?user.city:"Not provided",332,435,360,39,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"SAVINGS TARGET",709,405,350,23,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    char target[80];money(target,sizeof target,savingTarget);
    label(dc,target,709,435,380,39,g_fontBold,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    line(dc,331,508,1155,508,LINE);
    screenBtn(dc,335,546,202,44,"Edit my profile",0);
    screenBtn(dc,554,546,202,44,"Savings goal",1);
    screenBtn(dc,971,546,186,44,"Delete account",1);
    label(dc,"All application data is saved locally for the signed-in user.",
          315,670,829,40,g_fontRegular,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
}
static void authFieldOutline(HDC dc, HWND h) {
    if (!h) return;
    RECT r;GetWindowRect(h,&r);
    MapWindowPoints(NULL,g_main,(POINT*)&r,2);
    int l=(int)((r.left-g_viewX)/g_scale),
        t=(int)((r.top-g_viewY)/g_scale),
        right=(int)((r.right-g_viewX)/g_scale),
        bottom=(int)((r.bottom-g_viewY)/g_scale);
    HPEN pen=CreatePen(PS_SOLID,3,ERROR_RED);
    HGDIOBJ oldPen=SelectObject(dc,pen);
    HGDIOBJ oldBrush=SelectObject(dc,GetStockObject(HOLLOW_BRUSH));
    RoundRect(dc,l-3,t-3,right+3,bottom+3,9,9);
    SelectObject(dc,oldBrush);SelectObject(dc,oldPen);DeleteObject(pen);
}
static void drawAuthValidation(HDC dc) {
    int signup=g_page==AUTH_SIGNUP;
    int y=signup?148:231;
    if (g_authMessage[0]) {
        COLORREF errBack=C(255,235,238);
        rounded(dc,770,signup?99:144,404,signup?40:47,10,errBack);
        label(dc,g_authMessage,780,signup?100:148,381,signup?38:42,
              g_fontSmall,ERROR_RED,DT_LEFT|DT_VCENTER|DT_WORDBREAK);
    }
    int indices=signup?7:2;
    for (int i=0;i<indices;i++) {
        if (!g_authErrors[i][0]) continue;
        int ey=y+(i>4?i-1:i)*67;
        int ex=1008,ew=157;
        if (i==2 && signup) { ex=1050;ew=115; }
        if (i==4 && signup) { ex=835;ew=55; }
        if (i==5 && signup) { ex=1054;ew=111; }
        if (i==6 && signup) { ey=y+335; }
        if (i==5 && signup) { ey=y+268; }
        /* Short, readable red hints stay beside each form label. */
        label(dc,g_authErrors[i],ex,ey,ew,25,g_fontSmall,ERROR_RED,
              DT_RIGHT|DT_VCENTER|DT_SINGLELINE);
        authFieldOutline(dc,authField(i));
    }
}
static void authBackground(HDC dc) {
    filled(dc,0,0,APP_W,APP_H,SURFACE);
    /* Premium financial palette: midnight blue, emerald and a gold glow. */
    for (int x=0; x<652; x+=6) {
        float t=(float)x/652.0f;
        filled(dc,x,0,6,APP_H,C(10+(int)(15*t),29+(int)(23*t),
                                  51+(int)(18*t)));
    }
    rounded(dc,464,42,159,159,158,C(29,71,84));
    rounded(dc,493,71,101,101,106,C(31,86,94));
    rounded(dc,27,725,210,180,120,C(18,61,73));
    rounded(dc,553,327,7,7,7,GOLD);
    rounded(dc,571,327,7,7,7,C(106,220,188));
    rounded(dc,49,37,47,47,14,MINT);
    label(dc,"B",49,37,47,47,g_fontTitle,C(255,255,255),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,"BUDGETLY",107,43,230,39,g_fontBold,C(255,255,255),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,51,123,222,35,17,C(30,77,83));
    label(dc,"TRACK  |  PLAN  |  GROW",64,124,200,32,g_fontSmall,C(207,244,225),
          DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,"Your money,",51,190,560,59,g_fontHero,C(251,253,255),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"beautifully",51,250,570,59,g_fontHero,C(251,253,255),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"organized.",51,310,560,59,g_fontHero,C(87,233,185),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"One intelligent space for income, expenses and savings.",
          53,391,525,42,g_fontRegular,C(201,215,229),DT_LEFT|DT_VCENTER|DT_WORDBREAK);
    /* Chart panel: text, legend and bars occupy separate bands. */
    rounded(dc,52,465,542,234,20,C(29,62,78));
    rounded(dc,52,465,542,5,5,C(68,127,120));
    label(dc,"YOUR MONEY AT A GLANCE",78,482,338,23,g_fontSmall,C(189,214,215),
          DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,472,483,95,26,13,C(47,102,88));
    label(dc,"GROWTH",476,483,88,26,g_fontSmall,C(162,250,193),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,79,529,9,9,9,C(58,221,169));
    label(dc,"Income",95,516,94,32,g_fontMedium,C(236,246,253),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,194,529,9,9,9,GOLD);
    label(dc,"Expenses",210,516,105,32,g_fontMedium,C(236,246,253),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    rounded(dc,336,529,9,9,9,C(153,186,255));
    label(dc,"Savings",352,516,115,32,g_fontMedium,C(236,246,253),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    line(dc,78,558,564,558,C(56,93,105));
    int heights[7]={35,67,46,88,63,101,117};
    COLORREF colors[7]={C(65,137,150),C(77,151,166),C(64,135,154),
        C(97,169,168),C(101,177,159),GOLD,C(51,211,154)};
    for(int i=0;i<7;i++) {
        int bx=98+i*66, by=670-heights[i];
        rounded(dc,bx,by,29,heights[i],9,colors[i]);
    }
    line(dc,78,671,566,671,C(79,112,119));
    label(dc,"BETTER SPENDING  |  BETTER SAVING  |  BETTER TOMORROW",55,717,540,27,
          g_fontSmall,C(178,203,204),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    /* A well-spaced sign-in card, clearly distinguished from the hero. */
    shadowCard(dc,738,26,458,727,24);
    rounded(dc,738,26,458,5,5,MINT);
    int signup=g_page==AUTH_SIGNUP;
    int markX=signup?1127:764,markY=signup?48:52;
    rounded(dc,markX,markY,39,39,12,C(228,251,245));
    label(dc,"B",markX,markY,39,39,g_fontBold,C(26,162,140),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    label(dc,signup?"Create your account":"Welcome back",
          773,signup?44:96,397,signup?48:56,
          g_fontTitle,INK,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    if (!g_authMessage[0])
        label(dc,signup?"A few details and you're ready to begin.":
              "Sign in to view your personal finance dashboard.",
              775,signup?100:143,390,signup?39:49,
              g_fontRegular,MUTED,DT_LEFT|DT_WORDBREAK);
    int y=signup?148:231;
    label(dc,"USERNAME",782,y,260,24,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    label(dc,"PASSWORD",782,y+67,260,24,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    if(signup) {
        label(dc,"CONFIRM PASSWORD",782,y+134,300,24,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,"FULL NAME",782,y+201,300,24,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,"AGE",782,y+268,80,24,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,"PHONE",895,y+268,200,24,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
        label(dc,"CITY",782,y+335,220,24,g_fontSmall,MUTED,DT_LEFT|DT_VCENTER|DT_SINGLELINE);
    }
    if(!signup) label(dc,"PRIVATE LOCAL WORKSPACE",788,707,367,24,g_fontSmall,
          C(138,148,169),DT_CENTER|DT_VCENTER|DT_SINGLELINE);
    drawAuthValidation(dc);
}
static void paintMain(HWND wnd) {
    PAINTSTRUCT ps; HDC dc=BeginPaint(wnd,&ps);
    RECT client;GetClientRect(wnd,&client);
    /* Fill unused window area then render at responsive logical coordinates. */
    HBRUSH backdrop=CreateSolidBrush(g_page==AUTH_LOGIN || g_page==AUTH_SIGNUP
                                     ? NAVY : SURFACE);
    FillRect(dc,&client,backdrop);DeleteObject(backdrop);
    SetMapMode(dc,MM_ANISOTROPIC);
    SetWindowExtEx(dc,APP_W,APP_H,NULL);
    SetViewportExtEx(dc,g_viewW,g_viewH,NULL);
    SetViewportOrgEx(dc,g_viewX,g_viewY,NULL);
    SetBkMode(dc,TRANSPARENT);
    if(g_page==AUTH_LOGIN || g_page==AUTH_SIGNUP)authBackground(dc);
    else {
        filled(dc,0,0,APP_W,APP_H,SURFACE);
        drawSidebar(dc);
        if(g_page==PAGE_HOME)paintDashboard(dc);
        else if(g_page==PAGE_INCOME)paintIncome(dc);
        else if(g_page==PAGE_EXPENSE)paintExpenses(dc);
        else if(g_page==PAGE_TRANSACTIONS)paintTransactions(dc);
        else if(g_page==PAGE_REPORTS)paintReports(dc);
        else if(g_page==PAGE_TOOLS)paintBudgetTools(dc);
        else if(g_page==PAGE_PROFILE)paintProfile(dc);
        paintBudgetAlert(dc);
    }
    paintPopup(dc);
    EndPaint(wnd,&ps);
}

/* ---------- GUI forms: all forms are real Windows child windows in pure C. ---------- */
static void formLabel(HWND parent,const char *text,int x,int y,int w) {
    HWND h=CreateWindowExA(0,"STATIC",text,WS_CHILD|WS_VISIBLE,
         x,y,w,22,parent,NULL,GetModuleHandleA(NULL),NULL);
    SendMessageA(h,WM_SETFONT,(WPARAM)g_fontMedium,TRUE);
}
static void formCreateControls(HWND wnd) {
    int x=35,w=440;
    memset(g_formFields,0,sizeof g_formFields);
    if(g_formType==FORM_INCOME) {
        char date[20];ensureDate(date);
        formLabel(wnd,"AMOUNT (PKR)",x,82,w);
        g_formFields[0]=createEdit(wnd,x,107,w,34,"",0);
        formLabel(wnd,"SOURCE",x,154,w);
        g_formFields[1]=createEdit(wnd,x,179,w,34,"",0);
        formLabel(wnd,"DATE  (DD-MM-YYYY)",x,226,w);
        g_formFields[2]=createEdit(wnd,x,251,w,34,date,0);
        formLabel(wnd,"RECEIVED THROUGH",x,298,w);
        const char *methods[]={"Cash","Bank Transfer"};
        g_formFields[3]=createCombo(wnd,x,323,w,250,methods,2);
        createBtn(wnd,"Save income",x,405,209,43,IDC_FORM_SAVE,1);
        createBtn(wnd,"Cancel",x+230,405,210,43,IDC_FORM_CANCEL,0);
    } else if(g_formType==FORM_EXPENSE) {
        char date[20];ensureDate(date);
        formLabel(wnd,"AMOUNT (PKR)",x,72,w);
        g_formFields[0]=createEdit(wnd,x,97,w,32,"",0);
        formLabel(wnd,"CATEGORY",x,138,w);
        const char *cats[]={"Food","Transport","Shopping","Bills","Other"};
        g_formFields[1]=createCombo(wnd,x,163,w,220,cats,5);
        formLabel(wnd,"DATE  (DD-MM-YYYY)",x,198,w);
        g_formFields[2]=createEdit(wnd,x,223,w,32,date,0);
        formLabel(wnd,"PLACE / MERCHANT",x,264,w);
        g_formFields[3]=createEdit(wnd,x,289,w,32,"",0);
        formLabel(wnd,"PAID WITH",x,330,200);
        formLabel(wnd,"PAYMENT STATUS",x+230,330,200);
        const char *methods[]={"Cash","Bank"},*statuses[]={"Paid","Unpaid"};
        g_formFields[4]=createCombo(wnd,x,355,210,220,methods,2);
        g_formFields[5]=createCombo(wnd,x+230,355,210,220,statuses,2);
        createBtn(wnd,"Save expense",x,421,209,43,IDC_FORM_SAVE,1);
        createBtn(wnd,"Cancel",x+230,421,210,43,IDC_FORM_CANCEL,0);
    } else if(g_formType==FORM_SAVING) {
        char txt[60];snprintf(txt,sizeof txt,"%.2f",savingTarget);
        formLabel(wnd,"MY SAVINGS GOAL (PKR)",x,97,w);
        g_formFields[0]=createEdit(wnd,x,126,w,38,txt,0);
        formLabel(wnd,"This target will appear on your dashboard and reports.",
                  x,190,w);
        createBtn(wnd,"Save target",x,250,209,43,IDC_FORM_SAVE,1);
        createBtn(wnd,"Cancel",x+230,250,210,43,IDC_FORM_CANCEL,0);
    } else if(g_formType==FORM_PROFILE) {
        char age[30];snprintf(age,sizeof age,"%d",user.age);
        formLabel(wnd,"FULL NAME",x,73,w);
        g_formFields[0]=createEdit(wnd,x,99,w,34,user.name,0);
        formLabel(wnd,"AGE",x,149,w);
        g_formFields[1]=createEdit(wnd,x,175,w,34,age,0);
        formLabel(wnd,"PHONE NUMBER",x,224,w);
        g_formFields[2]=createEdit(wnd,x,250,w,34,user.phone,0);
        formLabel(wnd,"CITY",x,298,w);
        g_formFields[3]=createEdit(wnd,x,324,w,34,user.city,0);
        createBtn(wnd,"Save changes",x,402,209,43,IDC_FORM_SAVE,1);
        createBtn(wnd,"Cancel",x+230,402,210,43,IDC_FORM_CANCEL,0);
    } else if(g_formType==FORM_LIMIT) {
        char value[60];snprintf(value,sizeof value,"%.2f",g_spendingLimit);
        formLabel(wnd,"CUSTOM SPENDING CAP (PKR)",x,90,w);
        g_formFields[0]=createEdit(wnd,x,121,w,38,value,0);
        formLabel(wnd,"Set 0 to use the original 80% of income warning cap.",x,179,w);
        createBtn(wnd,"Save spending cap",x,242,209,43,IDC_FORM_SAVE,1);
        createBtn(wnd,"Cancel",x+230,242,210,43,IDC_FORM_CANCEL,0);
    } else if(g_formType==FORM_FORECAST) {
        formLabel(wnd,"EXPECTED EXTRA INCOME (PKR)",x,85,w);
        g_formFields[0]=createEdit(wnd,x,111,w,38,"0",0);
        formLabel(wnd,"PLANNED EXTRA EXPENSE (PKR)",x,170,w);
        g_formFields[1]=createEdit(wnd,x,196,w,38,"0",0);
        formLabel(wnd,"PLANNED VEHICLE / TRANSPORT COSTS (PKR)",x,258,w);
        g_formFields[2]=createEdit(wnd,x,284,w,38,"0",0);
        formLabel(wnd,"Simulation only: actual balances are not changed.",x,348,w);
        createBtn(wnd,"Calculate forecast",x,407,209,43,IDC_FORM_SAVE,1);
        createBtn(wnd,"Cancel",x+230,407,210,43,IDC_FORM_CANCEL,0);
    }
}
static void closeForm(void) {
    if(g_modal) DestroyWindow(g_modal);
}
static void showFormError(const char *message,HWND focus) {
    if(!g_modal) {errorBox(message);return;}
    copySafe(g_formError,sizeof g_formError,message);
    if(focus)SetFocus(focus);
    InvalidateRect(g_modal,NULL,TRUE);
    MessageBeep(MB_ICONWARNING);
}

static void submitForm(void) {
    if(!g_authenticated)return;
    if(g_formType==FORM_INCOME) {
        float amount;char source[30],date[20],method[40];
        if(incomeCount>=MAX_INCOME) {showFormError("Income history is full (100 entries).",NULL);return;}
        if(!amountFrom(g_formFields[0],&amount,0)) {showFormError("Enter a positive income amount (numbers only).",g_formFields[0]);return;}
        getText(g_formFields[1],source,sizeof source);trim(source);
        getText(g_formFields[2],date,sizeof date);trim(date);
        comboGet(g_formFields[3],method,sizeof method);
        if(!source[0] || !validDate(date)) {showFormError("A source and correct date (DD-MM-YYYY) are required.",g_formFields[1]);return;}
        incomes[incomeCount].amount=amount;
        copySafe(incomes[incomeCount].source,sizeof incomes[incomeCount].source,source);
        copySafe(incomes[incomeCount].date,sizeof incomes[incomeCount].date,date);
        copySafe(incomes[incomeCount].paymentMethod,sizeof incomes[incomeCount].paymentMethod,method);
        incomeCount++;saveIncomes();g_incomeOffset=0;
        g_selectedIncome=incomeCount-1;
        closeForm();repaintMain();return;
    }
    if(g_formType==FORM_EXPENSE) {
        float amount;char cat[30],date[20],place[50],method[30],status[20];
        if(expenseCount>=100) {showFormError("Expense history is full (100 entries).",NULL);return;}
        if(!amountFrom(g_formFields[0],&amount,0)) {showFormError("Enter a positive expense amount (numbers only).",g_formFields[0]);return;}
        comboGet(g_formFields[1],cat,sizeof cat);
        getText(g_formFields[2],date,sizeof date);trim(date);
        getText(g_formFields[3],place,sizeof place);trim(place);
        comboGet(g_formFields[4],method,sizeof method);
        comboGet(g_formFields[5],status,sizeof status);
        if(!place[0] || !validDate(date)) {showFormError("A place and correct date (DD-MM-YYYY) are required.",g_formFields[3]);return;}
        if(!_stricmp(status,"Paid")) {
            float bank,cash;getBalances(&bank,&cash);
            /* Original Ali calculatebalance() checks bank or cash availability. */
            if(!calculatebalance(amount,isCash(method)?2:1,&bank,&cash)) {
                showFormError(isCash(method)?"Not enough cash. Add income or mark this expense Unpaid.":
                            "Not enough bank balance. Add income or mark this expense Unpaid.",g_formFields[0]);
                return;
            }
        }
        expenses[expenseCount].amount=amount;
        copySafe(expenses[expenseCount].category,sizeof expenses[expenseCount].category,cat);
        copySafe(expenses[expenseCount].date,sizeof expenses[expenseCount].date,date);
        copySafe(expenses[expenseCount].place,sizeof expenses[expenseCount].place,place);
        copySafe(expenses[expenseCount].paymentMethod,sizeof expenses[expenseCount].paymentMethod,method);
        copySafe(expenses[expenseCount].status,sizeof expenses[expenseCount].status,status);
        expenseCount++;saveExpenses();g_expenseOffset=0;
        g_selectedExpense=expenseCount-1;
        closeForm();repaintMain();
        /* Original checkwarning thresholds: 80%, 85%, 90% of income.
           Present them in the custom graphical GDI warning instead of plain MessageBox. */
        float totalIncome=getTotalIncome(),totalExpense=getTotalExpense();
        if(!_stricmp(status,"Paid") && totalIncome>0) {
            int tier=totalExpense>=totalIncome*.90f?90:
                     totalExpense>=totalIncome*.85f?85:
                     totalExpense>=totalIncome*.80f?80:0;
            char title[96],description[240],hint[180];
            double percent=100.0*(double)totalExpense/(double)totalIncome;
            if(tier || (g_spendingLimit>0 && totalExpense>g_spendingLimit) ||
               (savingTarget>0 && totalExpense>totalIncome-savingTarget)) {
                if(tier>=90)copySafe(title,sizeof title,"Critical: 90% budget used");
                else if(tier>=85)copySafe(title,sizeof title,"High spending: 85% reached");
                else if(tier>=80)copySafe(title,sizeof title,"Heads up: 80% reached");
                else if(g_spendingLimit>0 && totalExpense>g_spendingLimit)
                    copySafe(title,sizeof title,"Custom spending cap exceeded");
                else copySafe(title,sizeof title,"Your savings zone is at risk");
                snprintf(description,sizeof description,
                    "Paid expenses: PKR %.2f   |   Total income: PKR %.2f   |   %.1f%% used",
                    (double)totalExpense,(double)totalIncome,percent);
                if(g_spendingLimit>0 && totalExpense>g_spendingLimit)
                    snprintf(hint,sizeof hint,"Your custom cap is PKR %.2f. Review Budget Tools.",
                        (double)g_spendingLimit);
                else if(savingTarget>0 && totalExpense>totalIncome-savingTarget)
                    snprintf(hint,sizeof hint,"Savings target PKR %.2f is now at risk.",
                        (double)savingTarget);
                else copySafe(hint,sizeof hint,
                    "Review spending categories and plan your next purchases.");
                showBudgetAlert(tier>=90?3:tier>=85?2:tier>=80?1:2,
                                title,description,hint,(double)totalExpense/(double)totalIncome);
            }
        }
        return;
    }
    if(g_formType==FORM_SAVING) {
        float value;
        if(!amountFrom(g_formFields[0],&value,1)) {errorBox("Enter a valid savings goal of zero or more.");return;}
        savingTarget=value;saveSavingTarget();closeForm();repaintMain();return;
    }
    if(g_formType==FORM_LIMIT) {
        float limit;
        if(!amountFrom(g_formFields[0],&limit,1)) {
            errorBox("Enter a valid spending cap (zero uses the standard 80% warning).");return;
        }
        g_spendingLimit=limit;saveBudgetLimit();closeForm();repaintMain();return;
    }
    if(g_formType==FORM_FORECAST) {
        float extraIncome,extraExpense,vehicleExpense;
        if(!amountFrom(g_formFields[0],&extraIncome,1) ||
           !amountFrom(g_formFields[1],&extraExpense,1) ||
           !amountFrom(g_formFields[2],&vehicleExpense,1)) {
            errorBox("Forecast amounts must be valid and zero or more.");return;
        }
        double projectedIncome=getTotalIncome()+extraIncome;
        double projectedSpent=getTotalExpense()+extraExpense+vehicleExpense;
        double projectedRemaining=projectedIncome-projectedSpent;
        const char *warning=projectedIncome<=0?"Add income to activate warnings":
            projectedSpent>=projectedIncome*.90?"90% critical warning":
            projectedSpent>=projectedIncome*.85?"85% spending warning":
            projectedSpent>=projectedIncome*.80?"80% spending warning":"Within the 80% warning point";
        char result[560];snprintf(result,sizeof result,
            "WHAT-IF BUDGET MODEL\n\nProjected income: PKR %.2f\nPlanned extra expenses: PKR %.2f\nVehicle / transport plan: PKR %.2f\nProjected total spending: PKR %.2f\nProjected remaining: PKR %.2f\nSavings goal: PKR %.2f\nStatus: %s\n\nThis forecast has NOT changed your saved transactions.",
            projectedIncome,(double)extraExpense,(double)vehicleExpense,
            projectedSpent,projectedRemaining,(double)savingTarget,warning);
        closeForm();openPopup(6,"Your budget forecast",result,
            "Simulation only: this does not change your saved transactions.");
        repaintMain();return;
    }
    if(g_formType==FORM_PROFILE) {
        char name[50],ageText[24],phone[20],city[30];
        getText(g_formFields[0],name,sizeof name);trim(name);
        getText(g_formFields[1],ageText,sizeof ageText);
        getText(g_formFields[2],phone,sizeof phone);trim(phone);
        getText(g_formFields[3],city,sizeof city);trim(city);
        int age=0;
        if(!name[0] || !validPhone(phone) || !city[0] || !parseAge(ageText,&age)) {
            errorBox("Enter your name, valid age, phone and city.");return;
        }
        copySafe(user.name,sizeof user.name,name);user.age=age;
        copySafe(user.phone,sizeof user.phone,phone);
        copySafe(user.city,sizeof user.city,city);
        saveUser();closeForm();repaintMain();return;
    }
}
static LRESULT CALLBACK formProcedure(HWND wnd,UINT msg,WPARAM wp,LPARAM lp) {
    switch(msg) {
        case WM_CREATE:
            formCreateControls(wnd);
            return 0;
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;HDC dc=BeginPaint(wnd,&ps);
            RECT r;GetClientRect(wnd,&r);
            filled(dc,0,0,r.right,r.bottom,C(255,255,255));
            filled(dc,0,0,r.right,60,NAVY);
            const char *title=g_formType==FORM_INCOME?"New income":
               g_formType==FORM_EXPENSE?"New expense":
               g_formType==FORM_SAVING?"Update savings target":
               g_formType==FORM_LIMIT?"Spending limit":
               g_formType==FORM_FORECAST?"What-if budget model":"Edit profile";
            label(dc,title,29,5,r.right-55,52,g_fontTitle,C(255,255,255),DT_LEFT|DT_VCENTER|DT_SINGLELINE);
            if(g_formError[0]) {
                int top=g_formType==FORM_INCOME?470:g_formType==FORM_EXPENSE?486:
                         g_formType==FORM_SAVING||g_formType==FORM_LIMIT?297:
                         g_formType==FORM_FORECAST?467:457;
                rounded(dc,34,top,442,50,12,C(255,236,239));
                rounded(dc,44,top+9,31,31,9,ERROR_RED);
                label(dc,"!",44,top+8,31,31,g_fontBold,C(255,255,255),
                      DT_CENTER|DT_VCENTER|DT_SINGLELINE);
                label(dc,g_formError,87,top+5,379,41,g_fontMedium,ERROR_RED,
                      DT_LEFT|DT_VCENTER|DT_WORDBREAK);
            }
            EndPaint(wnd,&ps);return 0;
        }
        case WM_CTLCOLORSTATIC: {
            HDC dc=(HDC)wp;SetTextColor(dc,MUTED);
            SetBkColor(dc,C(255,255,255));return (LRESULT)g_whiteBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC dc=(HDC)wp;SetTextColor(dc,INK);
            SetBkColor(dc,C(255,255,255));return (LRESULT)g_whiteBrush;
        }
        case WM_DRAWITEM:
            buttonPaint((DRAWITEMSTRUCT*)lp);return TRUE;
        case WM_COMMAND:
            if(g_formError[0] &&
               (HIWORD(wp)==EN_CHANGE || HIWORD(wp)==CBN_SELCHANGE)) {
                g_formError[0]=0;
                InvalidateRect(wnd,NULL,TRUE);
            }
            if(LOWORD(wp)==IDC_FORM_SAVE) {submitForm();return 0;}
            if(LOWORD(wp)==IDC_FORM_CANCEL) {closeForm();return 0;}
            break;
        case WM_CLOSE:
            DestroyWindow(wnd);return 0;
        case WM_DESTROY:
            g_modal=NULL;
            EnableWindow(g_main,TRUE);
            SetForegroundWindow(g_main);
            repaintMain();return 0;
    }
    return DefWindowProcA(wnd,msg,wp,lp);
}
static void openForm(int type) {
    if(!g_authenticated || g_modal)return;
    g_formType=type;
    g_formError[0]=0;
    int h=(type==FORM_INCOME?566:type==FORM_EXPENSE?589:
           type==FORM_SAVING||type==FORM_LIMIT?352:
           type==FORM_FORECAST?525:501);
    RECT r;GetWindowRect(g_main,&r);
    int x=r.left+(r.right-r.left-520)/2,y=r.top+(r.bottom-r.top-h)/2;
    g_modal=CreateWindowExA(WS_EX_DLGMODALFRAME,"BudgetlyForm","Budgetly | Transaction",
        WS_POPUP|WS_CAPTION|WS_SYSMENU,x,y,520,h,g_main,NULL,GetModuleHandleA(NULL),NULL);
    if(!g_modal){errorBox("Unable to open the form.");return;}
    EnableWindow(g_main,FALSE);
    ShowWindow(g_modal,SW_SHOW);UpdateWindow(g_modal);
    SetForegroundWindow(g_modal);
    if(g_formFields[0])SetFocus(g_formFields[0]);
}

/* Visible rows map to fixed-array indices. Sorting/filtering never changes
   the original records until a confirmed deletion takes place. */
static int visibleIncomeIndex(int row) {
    int index=incomeCount-1-g_incomeOffset-row;
    return row>=0 && row<7 && index>=0 && index<incomeCount ? index : -1;
}
static int visibleExpenseIndex(int row) {
    if(row<0 || row>=7)return -1;
    char filter[55];getText(g_search,filter,sizeof filter);trim(filter);
    int n=0;
    for(int i=expenseCount-1;i>=0;i--) {
        if(!expenseMatches(i,filter))continue;
        if(n++==g_expenseOffset+row)return i;
    }
    return -1;
}
/* Write to a temporary file first. Replace the per-user data file only after
   the full updated contents were successfully written. No record is removed
   from memory if saving fails. Existing 4-line/6-line formats stay unchanged. */
static int persistDeletion(int kind,int deleteIndex) {
    const char *original=kind==0?incomeFile:expenseFile;
    char temporary[108];
    snprintf(temporary,sizeof temporary,"%s.remove.tmp",original);
    FILE *fp=fopen(temporary,"w");
    if(!fp)return 0;
    int failure=0;
    if(kind==0) {
        for(int i=0;i<incomeCount;i++) {
            if(i==deleteIndex)continue;
            struct Income *v=&incomes[i];
            if(fprintf(fp,"%.2f\n%s\n%s\n%s\n",v->amount,v->source,
                       v->date,v->paymentMethod)<0) {failure=1;break;}
        }
    } else {
        for(int i=0;i<expenseCount;i++) {
            if(i==deleteIndex)continue;
            struct Expense *v=&expenses[i];
            if(fprintf(fp,"%.2f\n%s\n%s\n%s\n%s\n%s\n",v->amount,
                    v->category,v->date,v->place,v->paymentMethod,v->status)<0) {
                failure=1;break;
            }
        }
    }
    if(fclose(fp)==EOF)failure=1;
    if(failure || !MoveFileExA(temporary,original,
                 MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(temporary);
        return 0;
    }
    return 1;
}
static void deleteTransaction(int kind,int index) {
    if(!g_authenticated || g_modal || g_alertVisible)return;
    if((kind==0 && (index<0 || index>=incomeCount)) ||
       (kind==1 && (index<0 || index>=expenseCount)))return;
    char message[420],name[120];double amount;
    if(kind==0) {
        copySafe(name,sizeof name,incomes[index].source);
        amount=incomes[index].amount;
    } else {
        snprintf(name,sizeof name,"%s / %s",expenses[index].category,
                 expenses[index].place);
        amount=expenses[index].amount;
    }
    snprintf(message,sizeof message,
        "%s\n\nAmount: PKR %.2f\n\nRemoving this entry updates bank/cash balances, charts and monthly reports.",
        name,amount);
    g_popupIndex=index;
    openPopup(kind==0?3:4,kind==0?"Delete income?":"Delete expense?",
              message,"This action cannot be undone. Your saved file will be updated.");
}
static void executeDeleteTransaction(int kind,int index){
    if((kind==0 && (index<0 || index>=incomeCount)) ||
       (kind==1 && (index<0 || index>=expenseCount)))return;
    if(!persistDeletion(kind,index)) {
        errorBox("Could not update the saved data file. Nothing was deleted. \nCheck folder permissions and retry.");
        return;
    }
    if(kind==0) {
        for(int i=index;i<incomeCount-1;i++) incomes[i]=incomes[i+1];
        incomeCount--;
        g_selectedIncome=-1;
        if(g_incomeOffset>0 && g_incomeOffset>=incomeCount)
            g_incomeOffset=incomeCount>0?((incomeCount-1)/7)*7:0;
    } else {
        for(int i=index;i<expenseCount-1;i++) expenses[i]=expenses[i+1];
        expenseCount--;
        g_selectedExpense=-1;
        /* Search results may shrink after a deletion. Keep the current page valid. */
        char filter[55];getText(g_search,filter,sizeof filter);trim(filter);
        int found=filteredCount(filter);
        if(g_expenseOffset>0 && g_expenseOffset>=found)
            g_expenseOffset=found>0?((found-1)/7)*7:0;
    }
    g_transactionOffset=0;
    repaintMain();
    /* Deleting an income may change spending percentage or available balance.
       Recalculate before showing a friendly graphical budget warning. */
    if(kind==0) {
        float bank=0,cash=0;getBalances(&bank,&cash);
        double totalIncome=getTotalIncome(),spent=getTotalExpense();
        char title[96],body[240],hint[180];
        int risk=totalIncome>0 && spent>=totalIncome*.90?3:
                 totalIncome>0 && spent>=totalIncome*.85?2:
                 totalIncome>0 && spent>=totalIncome*.80?1:0;
        if(bank<0 || cash<0 || (totalIncome<=0 && spent>0)) {
            copySafe(title,sizeof title,"Review your balance");
            snprintf(body,sizeof body,
                "Removing income left Bank PKR %.2f and Cash PKR %.2f.",
                (double)bank,(double)cash);
            copySafe(hint,sizeof hint,"Review your history and fix incorrect entries.");
            showBudgetAlert(3,title,body,hint,totalIncome>0?spent/totalIncome:1);
        } else if(risk) {
            snprintf(title,sizeof title,"Spending is now above %d%%",risk==3?90:risk==2?85:80);
            snprintf(body,sizeof body,
                "After deleting income, PKR %.2f of PKR %.2f remains counted as spent.",
                spent,totalIncome);
            copySafe(hint,sizeof hint,"Review Budget Tools or adjust your spending plan.");
            showBudgetAlert(risk,title,body,hint,spent/totalIncome);
        }
    }
}

/* Account deletion is non-interactive in the GUI; the original console function
   remains in BudgetTracker_Core.inc and is not called from Win32 events. */
static void deleteAccountGUI(void) {
    openPopup(5,"Delete your account?",
        "This will permanently remove your login, personal profile, all income and expense entries, savings goals, and reports.",
        "Your financial history will be erased. This cannot be undone.");
}
static void executeDeleteAccount(void) {
    FILE *in=fopen("accounts.txt","r");
    if(!in){errorBox("Account database is not accessible.");return;}
    char tempPath[MAX_PATH],accountPath[MAX_PATH],name[30],pass[30];
    GetCurrentDirectoryA(MAX_PATH,accountPath);
    snprintf(tempPath,sizeof tempPath,"%s\\accounts_new.tmp",accountPath);
    snprintf(accountPath,sizeof accountPath,"%s\\accounts.txt",accountPath);
    FILE *out=fopen(tempPath,"w");
    if(!out){fclose(in);errorBox("Cannot prepare account deletion.");return;}
    int failure=0;
    while(fgets(name,sizeof name,in)) {
        if(!fgets(pass,sizeof pass,in)) {failure=1;break;}
        removeNewline(name);removeNewline(pass);
        if(strcmp(name,username)!=0 && fprintf(out,"%s\n%s\n",name,pass)<0) failure=1;
    }
    if(ferror(in))failure=1;
    if(fclose(out)==EOF)failure=1;
    fclose(in);
    if(failure || !MoveFileExA(tempPath,accountPath,MOVEFILE_REPLACE_EXISTING|
        MOVEFILE_WRITE_THROUGH)) {
        DeleteFileA(tempPath);errorBox("Could not complete account deletion. No profile files were removed.");return;
    }
    remove(userFile);remove(incomeFile);remove(expenseFile);remove(savingFile);
    char limitName[80];budgetLimitFilename(limitName);remove(limitName);
    incomeCount=expenseCount=0;savingTarget=0;g_spendingLimit=0;userSet=0;
    g_authenticated=0;username[0]=0;password[0]=0;
    setPage(AUTH_LOGIN);
    info("The account was deleted.");
}
static void logout(void) {
    if(g_modal)return;
    g_authenticated=0;
    incomeCount=expenseCount=0;
    savingTarget=g_spendingLimit=0;userSet=0;
    g_selectedIncome=g_selectedExpense=-1;g_alertVisible=0;
    username[0]=password[0]=0;
    setPage(AUTH_LOGIN);
}
static int inRect(int x,int y,int l,int t,int r,int b) {
    return x>=l && x<=r && y>=t && y<=b;
}
static void handleClick(int x,int y) {
    if(g_popupType){
        if(x>=362 && x<=888 && y>=549 && y<=606){
            if(g_popupType>=3 && g_popupType<=5){
                if(x>=399 && x<=607)popupDecision(0);
                else if(x>=623 && x<=851)popupDecision(1);
            } else popupDecision(0);
        }
        return;
    }
    if(!g_authenticated || g_modal)return;
    if(g_alertVisible) {
        if(inRect(x,y,842,531,1053,574)) {dismissBudgetAlert();return;}
        if(inRect(x,y,620,531,827,574)) {
            dismissBudgetAlert();setPage(PAGE_TOOLS);return;
        }
        return; /* Prevent clicks behind an active graphical budget warning. */
    }
    for(int i=0;i<7;i++) {
        int top=164+i*52;
        if(inRect(x,y,15,top,237,top+43)) {setPage(PAGE_HOME+i);return;}
    }
    if(inRect(x,y,28,724,227,760)) {logout();return;}
    if(g_page==PAGE_HOME && inRect(x,y,1070,690,1181,730)) {openForm(FORM_INCOME);return;}
    if(g_page==PAGE_INCOME) {
        if(inRect(x,y,1024,142,1197,190)) {openForm(FORM_INCOME);return;}
        if(inRect(x,y,837,142,1011,190)) {
            deleteTransaction(0,g_selectedIncome);return;
        }
        if(inRect(x,y,311,341,1181,698)) {
            int index=visibleIncomeIndex((y-341)/51);
            if(index>=0) {
                if(inRect(x,y,1120,341+((y-341)/51)*51+10,
                             1176,341+((y-341)/51)*51+39))
                    deleteTransaction(0,index);
                else {g_selectedIncome=index;repaintMain();}
                return;
            }
        }
        if(inRect(x,y,1033,697,1096,727)) {if(g_incomeOffset>=7)g_incomeOffset-=7;repaintMain();return;}
        if(inRect(x,y,1108,697,1173,727)) {
            if(g_incomeOffset+7<incomeCount)g_incomeOffset+=7;repaintMain();return;
        }
    }
    if(g_page==PAGE_EXPENSE) {
        if(inRect(x,y,314,154,477,196)) {openForm(FORM_EXPENSE);return;}
        if(inRect(x,y,489,154,647,196)) {
            deleteTransaction(1,g_selectedExpense);return;
        }
        if(inRect(x,y,311,341,1181,698)) {
            int row=(y-341)/51;
            int index=visibleExpenseIndex(row);
            if(index>=0) {
                if(inRect(x,y,1120,341+row*51+10,1176,341+row*51+39))
                    deleteTransaction(1,index);
                else {g_selectedExpense=index;repaintMain();}
                return;
            }
        }
        char filter[55];getText(g_search,filter,sizeof filter);trim(filter);
        int found=filteredCount(filter);
        if(inRect(x,y,1033,697,1096,727)) {if(g_expenseOffset>=7)g_expenseOffset-=7;repaintMain();return;}
        if(inRect(x,y,1108,697,1173,727)) {
            if(g_expenseOffset+7<found)g_expenseOffset+=7;repaintMain();return;
        }
    }
    if(g_page==PAGE_TRANSACTIONS) {
        for(int k=0;k<4;k++)if(inRect(x,y,300+k*174,144,455+k*174,185)) {
            g_transactionFilter=k;g_transactionOffset=0;repaintMain();return;
        }
        TransactionRow records[200];int n=allTransactions(records);
        if(inRect(x,y,1120,341,1176,698)) {
            int row=(y-341)/51;
            if(row>=0 && row<7 && row+g_transactionOffset<n &&
               inRect(x,y,1120,341+row*51+10,1176,341+row*51+39)) {
                TransactionRow tr=records[row+g_transactionOffset];
                deleteTransaction(tr.kind,tr.index);return;
            }
        }
        if(inRect(x,y,1033,697,1096,727)) {
            if(g_transactionOffset>=7)g_transactionOffset-=7;
            repaintMain();return;
        }
        if(inRect(x,y,1108,697,1173,727)) {
            if(g_transactionOffset+7<n)g_transactionOffset+=7;
            repaintMain();return;
        }
    }
    if(g_page==PAGE_REPORTS) {
        if(inRect(x,y,938,502,1155,540)) {openForm(FORM_SAVING);return;}
        if(inRect(x,y,934,134,1043,170)) {if(g_reportMonthOffset<120)g_reportMonthOffset++;repaintMain();return;}
        if(inRect(x,y,1051,134,1180,170)) {if(g_reportMonthOffset>0)g_reportMonthOffset--;repaintMain();return;}
    }
    if(g_page==PAGE_TOOLS) {
        if(inRect(x,y,916,527,1183,572)) {openForm(FORM_SAVING);return;}
        if(inRect(x,y,314,668,566,715)) {openForm(FORM_LIMIT);return;}
        if(inRect(x,y,585,668,837,715)) {openForm(FORM_FORECAST);return;}
    }
    if(g_page==PAGE_PROFILE) {
        if(inRect(x,y,335,546,537,590)) {openForm(FORM_PROFILE);return;}
        if(inRect(x,y,554,546,756,590)) {openForm(FORM_SAVING);return;}
        if(inRect(x,y,971,546,1157,590)) {deleteAccountGUI();return;}
    }
}
static LRESULT CALLBACK mainProcedure(HWND wnd,UINT msg,WPARAM wp,LPARAM lp) {
    switch(msg) {
        case WM_CREATE:
            g_main=wnd;
            /* First launch displays Sign up; returning users see Sign in. */
            FILE *existing=fopen("accounts.txt","r");
            if(existing) {fclose(existing);setPage(AUTH_LOGIN);}
            else setPage(AUTH_SIGNUP);
            return 0;
        case WM_ERASEBKGND:return 1;
        case WM_SIZE:
            updateViewport();layoutControls();repaintMain();return 0;
        case WM_DPICHANGED: {
            RECT *suggested=(RECT*)lp;
            SetWindowPos(wnd,NULL,suggested->left,suggested->top,
               suggested->right-suggested->left,suggested->bottom-suggested->top,
               SWP_NOZORDER|SWP_NOACTIVATE);
            updateViewport();layoutControls();return 0;
        }
        case WM_PAINT:paintMain(wnd);return 0;
        case WM_CTLCOLORSTATIC: {
            HDC dc=(HDC)wp;SetTextColor(dc,MUTED);
            SetBkColor(dc,C(255,255,255));return (LRESULT)g_whiteBrush;
        }
        case WM_CTLCOLOREDIT: {
            HDC dc=(HDC)wp;
            for (int i=0;i<7;i++) {
                if ((HWND)lp==authField(i) && g_authErrors[i][0]) {
                    SetTextColor(dc,ERROR_RED);
                    SetBkColor(dc,C(255,240,242));
                    return (LRESULT)g_authErrorBrush;
                }
            }
            SetTextColor(dc,INK);
            SetBkColor(dc,C(255,255,255));return (LRESULT)g_whiteBrush;
        }
        case WM_DRAWITEM:buttonPaint((DRAWITEMSTRUCT*)lp);return TRUE;
        case WM_COMMAND:
            if(LOWORD(wp)==IDC_AUTH_PRIMARY) {authSubmit();return 0;}
            if(LOWORD(wp)==IDC_AUTH_SWITCH) {
                setPage(g_page==AUTH_LOGIN?AUTH_SIGNUP:AUTH_LOGIN);return 0;
            }
            if(LOWORD(wp)==IDC_AUTH_BACK) {
                setPage(AUTH_LOGIN);return 0;
            }
            if(HIWORD(wp)==EN_CHANGE && (g_page==AUTH_LOGIN || g_page==AUTH_SIGNUP)) {
                authTyping((HWND)lp);return 0;
            }
            if((HWND)lp==g_search && HIWORD(wp)==EN_CHANGE) {
                g_expenseOffset=0;repaintMain();return 0;
            }
            break;
        case WM_MOUSEMOVE: {
            int x=(int)((GET_X_LPARAM(lp)-g_viewX)/g_scale);
            int y=(int)((GET_Y_LPARAM(lp)-g_viewY)/g_scale),index=-1;
            if(g_authenticated && x>=15 && x<=237)
               for(int i=0;i<7;i++)if(y>=164+i*52 && y<=207+i*52)index=i;
            if(index!=g_navHover){g_navHover=index;repaintMain();}
            return 0;
        }
        case WM_LBUTTONUP: {
            int x=(int)((GET_X_LPARAM(lp)-g_viewX)/g_scale);
            int y=(int)((GET_Y_LPARAM(lp)-g_viewY)/g_scale);
            handleClick(x,y);return 0;
        }
        case WM_KEYDOWN:
            if(wp==VK_DELETE && g_authenticated && !g_alertVisible && !g_popupType) {
                if(g_page==PAGE_INCOME && g_selectedIncome>=0)
                    deleteTransaction(0,g_selectedIncome);
                else if(g_page==PAGE_EXPENSE && g_selectedExpense>=0)
                    deleteTransaction(1,g_selectedExpense);
                return 0;
            }
            if(g_alertVisible && (wp==VK_ESCAPE || wp==VK_RETURN)) {
                dismissBudgetAlert();return 0;
            }
            if(wp==VK_F11) {toggleFullscreen();return 0;}
            if(wp==VK_ESCAPE && g_page==AUTH_SIGNUP) {setPage(AUTH_LOGIN);return 0;}
            if(wp==VK_ESCAPE && g_authenticated) {logout();return 0;}
            break;
        case WM_CLOSE:
            if(g_modal || g_popupType)return 0;
            DestroyWindow(wnd);return 0;
        case WM_DESTROY:
            DeleteObject(g_fontRegular);DeleteObject(g_fontMedium);
            DeleteObject(g_fontBold);DeleteObject(g_fontTitle);
            DeleteObject(g_fontHero);DeleteObject(g_fontSmall);
            DeleteObject(g_whiteBrush);DeleteObject(g_backgroundBrush);
            DeleteObject(g_authErrorBrush);
            if(g_fieldFont)DeleteObject(g_fieldFont);
            if(g_buttonFont)DeleteObject(g_buttonFont);
            PostQuitMessage(0);return 0;
    }
    return DefWindowProcA(wnd,msg,wp,lp);
}

int WINAPI WinMain(HINSTANCE instance,HINSTANCE previous,LPSTR cmd,int show) {
    (void)previous;(void)cmd;
    SetProcessDPIAware();
    /* Always save the text files beside the executable, rather than a random
       working directory chosen by a shortcut or the terminal. */
    char path[MAX_PATH];
    if(GetModuleFileNameA(NULL,path,MAX_PATH)) {
        char *slash=strrchr(path,'\\');if(slash){*slash=0;SetCurrentDirectoryA(path);}
    }
    g_fontSmall=font(13,0);g_fontRegular=font(16,0);
    g_fontMedium=font(16,1);g_fontBold=font(19,1);
    g_fontTitle=font(27,1);g_fontHero=font(43,1);
    g_whiteBrush=CreateSolidBrush(C(255,255,255));
    g_backgroundBrush=CreateSolidBrush(SURFACE);
    g_authErrorBrush=CreateSolidBrush(C(255,240,242));
    WNDCLASSA mainClass={0},formClass={0};
    mainClass.lpfnWndProc=mainProcedure;
    mainClass.hInstance=instance;
    mainClass.hCursor=LoadCursor(NULL,IDC_ARROW);
    mainClass.hbrBackground=g_backgroundBrush;
    mainClass.lpszClassName="BudgetlyMain";
    mainClass.hIcon=LoadIcon(NULL,IDI_APPLICATION);
    if(!RegisterClassA(&mainClass))return 1;
    formClass.lpfnWndProc=formProcedure;
    formClass.hInstance=instance;
    formClass.hCursor=LoadCursor(NULL,IDC_ARROW);
    formClass.hbrBackground=g_whiteBrush;
    formClass.lpszClassName="BudgetlyForm";
    if(!RegisterClassA(&formClass))return 1;
    RECT r={0,0,APP_W,APP_H};
    AdjustWindowRectEx(&r,WS_OVERLAPPEDWINDOW,FALSE,0);
    int w=r.right-r.left,h=r.bottom-r.top;
    int x=(GetSystemMetrics(SM_CXSCREEN)-w)/2;
    int y=(GetSystemMetrics(SM_CYSCREEN)-h)/2;
    HWND win=CreateWindowExA(0,"BudgetlyMain","Budgetly | Personal Finance Manager",
        WS_OVERLAPPEDWINDOW,
        x<0?0:x,y<0?0:y,w,h,NULL,NULL,instance,NULL);
    if(!win)return 1;
    /* Start maximized on desktops; the user can restore or press F11. */
    ShowWindow(win,SW_SHOWMAXIMIZED);UpdateWindow(win);
    MSG msg;
    while(GetMessageA(&msg,NULL,0,0)>0){
        if(msg.message==WM_KEYDOWN && g_popupType &&
           (msg.wParam==VK_RETURN || msg.wParam==VK_ESCAPE)){
            /* Return/Escape can dismiss info; deletion requires a deliberate button click. */
            popupDecision(0);
            continue;
        }
        /* Dismiss graphical warning using Enter/Escape, even with controls focused. */
        if(msg.message==WM_KEYDOWN && g_alertVisible &&
           (msg.wParam==VK_RETURN || msg.wParam==VK_ESCAPE)) {
            dismissBudgetAlert();continue;
        }
        /* Keyboard shortcut must work even while an edit field has focus. */
        if(msg.message==WM_KEYDOWN && msg.wParam==VK_F11) {
            toggleFullscreen();continue;
        }
        /* Enter confirms the focused sign-in/sign-up form, without a mouse. */
        if(msg.message==WM_KEYDOWN && msg.wParam==VK_RETURN && !g_authenticated &&
           (g_page==AUTH_LOGIN || g_page==AUTH_SIGNUP) && !g_modal) {
            authSubmit();continue;
        }
        /* Also allow pressing Enter in income, expense, profile or savings edits. */
        if(msg.message==WM_KEYDOWN && msg.wParam==VK_RETURN && g_modal &&
           (msg.hwnd==g_modal || IsChild(g_modal,msg.hwnd))) {
            char cls[24]={0};GetClassNameA(msg.hwnd,cls,sizeof cls);
            if (!_stricmp(cls,"Edit")) {submitForm();continue;}
        }
        /* Escape from accidental signup returns to sign-in. */
        if(msg.message==WM_KEYDOWN && msg.wParam==VK_ESCAPE &&
           !g_authenticated && g_page==AUTH_SIGNUP && !g_modal) {
            setPage(AUTH_LOGIN);continue;
        }
        TranslateMessage(&msg);DispatchMessageA(&msg);
    }
    return (int)msg.wParam;
}
