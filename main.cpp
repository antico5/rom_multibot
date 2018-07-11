//      Made from Antico aka armakapo , 29th December 2009, in Argentina :)
//      For all the rompros' guys xD enjoy


#include <stdio.h>
#include <windows.h>
#include <time.h>
#include "header.h"
#define BASE_POINTER		 0x00956368
#define POWER_OFFSET         0x2D8
#define MAXPOWER_OFFSET      0x2DC
#define HP_OFFSET            0x2CC
#define MAXHP_OFFSET         0x2D4
#define CLASS_OFFSET         0x300
#define TARGET_OFFSET        0x278
#define CASTING_OFFSET       0x264
#define MISC_OFFSET          0xEC
#define TELEPOINTER_OFFSET   0x8C
#define COORD_OFFSET         0xB0


void SendKey(HWND window, WPARAM key){
    PostMessage(window,WM_KEYDOWN, key,1);
    PostMessage(window,WM_KEYUP, key,1);
}     

struct coords{
    int x;
    int y;
    int z;
};

struct Box{
    bool            commander;
    bool            active;     
    HWND            window;
    HANDLE          process;
    DWORD           pid;
    char            name[20];
    int             firstclass;
    int             hp;
    int             maxhp;
    int             power;
    int             maxpower;
    int             targethp;
    int             targetmaxhp;
    float           castingtime;
    coords          position;
};


Box boxes[7];                    // array starts at 1, to not pass a [-1] to the FindWindowEx func
Box* Commander;
char classes[][10]={"Warrior","Scout","Rogue","Mage","Priest","Knight","Warden","Druid"};

void DoBot(Box box){
    if (box.firstclass > 3 && box.power < box.maxpower*0.50){             // classes 4-8 use mana
        SendKey(box.window,0x38);    
    }
    if (Commander->targethp != Commander->targetmaxhp && Commander->targethp != 0 || Commander->castingtime > 0){
        SendKey(box.window,0x33);
    }
    else{
        SendKey(box.window,0x32); 
    }
    
    
}

void PrintData(){
    for (int i = 1; i<7 ; i++){
        if (boxes[i].active){
            printf("\nName:");SetColor(red);printf(" %s\n",boxes[i].name);SetColor(green);
            printf("Class: \t%s\n",classes[boxes[i].firstclass-1]);
            printf("HP: \t%d/%d\n",boxes[i].hp,boxes[i].maxhp);
            printf("Power: \t%d/%d\n",boxes[i].power,boxes[i].maxpower);
        }
    } 
}

void ChangeSize(HWND hwnd,int width,int height){
    RECT rect;
    GetWindowRect(hwnd,&rect);  
    SetWindowPos(hwnd,HWND_TOP,rect.left,rect.top,width,height, 0);
}

bool UpdateData(int option) {              //here all player data will be loaded to the structure
    unsigned int base;
    unsigned int player;
    unsigned int target;
    unsigned int telepointer;
    unsigned int misc;
    for ( short i = 1 ; i < 7 ; i++){
        if (boxes[i].window  > 0) {
            
                             
            
            if(ReadProcessMemory(boxes[i].process, (void *)BASE_POINTER, &base, 4, 0) == 0)
            {
                printf("Error reading base pointer (that's bad, press F8)\n");
                return false;
            }
    
            
            if(ReadProcessMemory(boxes[i].process, (void *)(base + 0x58C), &player, 4, 0) == 0)
            {
                printf("Error reading player pointer\n");
                return false;
            }
            
            if(ReadProcessMemory(boxes[i].process, (void *)(player + TARGET_OFFSET), &target, 4, 0) == 0)
            {
                printf("Error reading target pointer (happens if you are teleporting)\n");
                return false;
            }
            
            if (!ReadProcessMemory(boxes[i].process,(void*)(player+MISC_OFFSET),&misc,4,0)){
                    printf("Error reading misc pointer\n");
                    return false;
            }
                
            if (!ReadProcessMemory(boxes[i].process,(void*)(misc+TELEPOINTER_OFFSET),&telepointer,4,0)){
                printf("Error reading teleport pointer\n");
                return false;
            }
            
            ReadProcessMemory(boxes[i].process, (void *)(target + HP_OFFSET), &boxes[i].targethp, 4, 0);
            
            
            ReadProcessMemory(boxes[i].process, (void *)(target + MAXHP_OFFSET), &boxes[i].targetmaxhp, 4, 0);
            
            if (!ReadProcessMemory(boxes[i].process,(void*)0x00957b4c,&boxes[i].name,20,0)){
                printf("Error reading name\n");
                return false;
            }
            
            if (!ReadProcessMemory(boxes[i].process,(void*) (player+CLASS_OFFSET),&boxes[i].firstclass,4,0)){
                printf("Error reading 1st class\n");
                return false;
            }
            
            if (!ReadProcessMemory(boxes[i].process,(void*)(player+HP_OFFSET),&boxes[i].hp,4,0)){
                printf("Error reading hp\n");
                return false;
            }
            
            if (!ReadProcessMemory(boxes[i].process,(void*)(player+MAXHP_OFFSET),&boxes[i].maxhp,4,0)){
                printf("Error reading max hp\n");
                return false;
            }
            
            if (!ReadProcessMemory(boxes[i].process,(void*)(player+POWER_OFFSET),&boxes[i].power,4,0)){
                printf("Error reading power\n");
                return false;
            }
            
            if (!ReadProcessMemory(boxes[i].process,(void*)(player+MAXPOWER_OFFSET),&boxes[i].maxpower,4,0)){
                printf("Error reading max power\n");
                return false;
            }
            
            if (!ReadProcessMemory(boxes[i].process,(void*)(player+CASTING_OFFSET),&boxes[i].castingtime,4,0)){
                printf("Error reading casting time\n");
                return false;
            }
            if (boxes[i].commander){
                
                
                if (!ReadProcessMemory(boxes[i].process,(void*)(telepointer+COORD_OFFSET),&boxes[i].position,12,0)){
                    printf("Error reading commander coordinates\n");
                    return false;
                }
            }
            
            if (option == 1 && !boxes[i].commander) {        
                if(WriteProcessMemory(boxes[i].process, (void *)(telepointer + 0xB0), &Commander->position, 12, NULL) == 0){
                    printf("Error setting player coordinates\n");
                    return false;
                }                  
            }
        }
    }
    return true;
}
    

bool Initialize() {
    // Getting windows handles                        
    for ( short i = 1 ; i < 7 ; i++){
        boxes[i].window = FindWindowEx(NULL,boxes[i-1].window, NULL, "Runes of Magic");
        if (boxes[i].window == NULL)
            break;
        else
            boxes[i].active = true;
    }
    if (!boxes[1].window){
        printf("Didn't find any windows\n");
        return false;
    }
    
    //Getting proccess ids and open them
    for ( short i = 1 ; i < 7 ; i++){
        if (boxes[i].window  > 0) {
            GetWindowThreadProcessId(boxes[i].window, &boxes[i].pid);
            boxes[i].process = OpenProcess(PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, 0 , boxes[i].pid);
            if (boxes[i].process == NULL ){
                printf("Error while trying to open the processes %d\n",boxes[i].pid);
                return false;
            }
                                
        }   
    }
    
    //Load the data for first time
    if ( !UpdateData(0) )
        return false;
    
    //Put characters' names as window titles.
    for ( short i = 1 ; i < 7 ; i++){
        if (boxes[i].window  > 0) {
            SetWindowText(boxes[i].window, boxes[i].name);
        }
    }
    
    
    return true;
}

void Finalize(){
    for ( short i = 1 ; i < 7 ; i++ ){
        if (boxes[i].window > 0) {
            SetWindowText(boxes[i].window,"Runes of Magic");
        }
    }
}    


int main(){
    char buffer[64];
    int loop = 0;
    SetColor(yellow);
    printf("Multibot in construction, made by armakapo\n");
    memset(&boxes, sizeof(Box)*7,0);
    SetColor(red);
    if(!Initialize()) {
        printf("Couldn't initialize, exiting\n");
        Finalize();
        getchar();
        return 0;
    }
    while(loop == 0){    
        SetColor(green);
        printf("Enter the name of the commander\nPosible names are:");
        SetColor(blue);
        printf(" %s  %s  %s  %s  %s  %s\n",boxes[1].name,boxes[2].name,boxes[3].name,boxes[4].name,boxes[5].name,boxes[6].name); // lazy to make a loop xD
        gets(buffer);
        for (int i = 1; i<7; i++){
            if (strcmp(buffer,boxes[i].name) == 0){
                boxes[i].commander = true;
                Commander = &boxes[i];
                loop=1;
                break;
            }
        }
        
    }
    SetColor(green);
    printf("Now working.. press anytime F8 to exit and restore the windows' name\nDon't click the X to close!\n");
    PrintData();
    while(1){       
        for ( int i = 0; i<7 ; i++ ){
            if (boxes[i].active && boxes[i].commander != true){     
                DoBot(boxes[i]);
            }
        }
        if(!UpdateData(0) ){
            Sleep(3000);
        }
        if(GetAsyncKeyState(VK_NUMPAD1))
            UpdateData(1);    
        if(GetAsyncKeyState(VK_F8)){
            Finalize(); return 0; }
        if(GetAsyncKeyState(VK_F9)){    
            SetColor(red);
            printf("Paused\n");                                                
            while (1){
                Sleep(200);  
                if (GetAsyncKeyState(VK_F9)){
                    SetColor(green);
                    printf("Unpaused\n");                         
                    break;
                }
            }
                                                 
        }
                                                                             
        Sleep(200);
    }
    Finalize();
    getchar();
    return 0;
}
